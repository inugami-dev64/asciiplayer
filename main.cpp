#include <cstdio>
#include <iostream>
#include <thread>

#include "src/console_info.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
}

using namespace std;

static int decode_packet(AVPacket *pPacket, AVCodecContext *pCodecContext, AVFrame *pFrame, AVFrame *pConvertedFrame, SwsContext *pSwsContext);
static int draw_frame(AVFrame *pFrame);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("You need to specify a media file.\n");
        return -1;
    }

    cout << "Initializing all the containers, codecs and protocols" << endl;

    // Holds header information from the format
    AVFormatContext *pFormatContext = avformat_alloc_context();
    if (!pFormatContext) {
        cerr << "ERROR: Could not allocate memory for Format Context" << endl;
        return -1;
    }

    cout << "Opening the input file " << argv[1] << " and loading format (container) header" << endl;

    // Open the file and read its header
    if (avformat_open_input(&pFormatContext, argv[1], nullptr, nullptr) != 0) {
        cerr << "ERROR: Could not open input file " << argv[1] << endl;
        return -1;
    }

    // now we have access to some information about the file
    cout << "format " << pFormatContext->iformat->name <<
        ", duration " << pFormatContext->duration <<
        ", bit_rate " << pFormatContext->bit_rate << endl;

    cout << "finding stream info from format" << endl;
    // read packets from the Format to get stream information
    if (avformat_find_stream_info(pFormatContext, nullptr) < 0) {
        cerr << "ERROR: Could not find stream information" << endl;
        return -1;
    }

    // in this step we will find out the codec for video stream
    const AVCodec *pCodec = nullptr;
    const AVCodecParameters *pCodecParameters = nullptr;
    int video_stream_index = -1;

    for (int i = 0; i < pFormatContext->nb_streams; i++) {
        AVCodecParameters *pLocalCodecParameters = pFormatContext->streams[i]->codecpar;
        cout << "AVStream->time_base before open coded " << pFormatContext->streams[i]->time_base.num << '/' << pFormatContext->streams[i]->time_base.den << endl;
        cout << "AVStream->r_frame_rate before open coded " << pFormatContext->streams[i]->r_frame_rate.num << '/' << pFormatContext->streams[i]->r_frame_rate.den << endl;
        printf("AVStream->start_time %" PRId64 "\n", pFormatContext->streams[i]->start_time);
        printf("AVStream->duration %" PRId64 "\n", pFormatContext->streams[i]->duration);

        cout << "finding the proper decoder (CODEC)" << endl;


        // finds the registered decoder for codec ID
        const AVCodec *pLocalCodec = avcodec_find_decoder(pLocalCodecParameters->codec_id);
        if (pLocalCodec == nullptr) {
            cerr << "ERROR: unsupported codec!" << endl;
            return -1;
        }

        // we will save the video stream's index, codec parameters and codec
        if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            pCodec = pLocalCodec;
            pCodecParameters = pLocalCodecParameters;
        } else if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_AUDIO) {
            cout << "Audio Codec: " << pLocalCodecParameters->ch_layout.nb_channels << " channels, sample rate " << pLocalCodecParameters->sample_rate << endl;
        }

        // print codec name, id and bitrate
        cout << "\tCodec " << pLocalCodec->name << " ID " << pLocalCodec->id << " bit_rate " << pLocalCodecParameters->bit_rate << endl;
    }

    if (video_stream_index == -1) {
        cerr << "ERROR: Could not find a video stream" << endl;
        return -1;
    }

    AVCodecContext *pCodecContext = avcodec_alloc_context3(pCodec);
    if (!pCodecContext) {
        cerr << "ERROR: Could not allocate video codec context" << endl;
        return -1;
    }

    // Fill the codec context based on the values provided by the codec parameters
    if (avcodec_parameters_to_context(pCodecContext, pCodecParameters) < 0) {
        cerr << "ERROR: Failed to copy codec parameters to context" << endl;
        return -1;
    }

    // Initialize the AVCodecContext to use given AVCodec
    if (avcodec_open2(pCodecContext, pCodec, nullptr) < 0) {
        cerr << "ERROR: Could not open codec" << endl;
        return -1;
    }

    AVFrame *pFrame = av_frame_alloc();
    if (!pFrame) {
        cerr << "ERROR: Could not allocate memory for AVFrame" << endl;
        return -1;
    }

    AVPacket *pPacket = av_packet_alloc();
    if (!pPacket) {
        cerr << "ERROR: Could not allocate memory for AVPacket" << endl;
        return -1;
    }

    // Create SwsContext
    ap::Rectangle<int> console_dimensions = ap::Console::get_console_dimensions();
    SwsContext *pSwsContext = sws_getContext(
        pCodecContext->width,
        pCodecContext->height,
        pCodecContext->pix_fmt,
        console_dimensions.width,
        console_dimensions.height,
        AV_PIX_FMT_GRAY8,
        SWS_BICUBIC,
        nullptr, nullptr, nullptr);


    AVFrame *pConvertedFrame = av_frame_alloc();
    if (!pConvertedFrame) {
        cerr << "ERROR: Could not allocate memory for AVFrame" << endl;
        return -1;
    }
    pConvertedFrame->width = console_dimensions.width;
    pConvertedFrame->height = console_dimensions.height;

    int num_bytes = av_image_get_buffer_size(AV_PIX_FMT_GRAY8, console_dimensions.width, console_dimensions.height, 1);
    auto *pConvertedFrameBuffer = static_cast<uint8_t*>(av_malloc(num_bytes * sizeof(uint8_t)));
    int response = av_image_fill_arrays(pConvertedFrame->data, pConvertedFrame->linesize, pConvertedFrameBuffer, AV_PIX_FMT_GRAY8, console_dimensions.width, console_dimensions.height, 1);
    if (response < 0) {
        cerr << "ERROR: Could not fill buffer" << endl;
        av_free(pConvertedFrameBuffer);
        return response;
    }

    // fill the packet with data from stream
    while (av_read_frame(pFormatContext, pPacket) >= 0) {
        auto begin_ts = std::chrono::high_resolution_clock::now();
        if (pPacket->stream_index == video_stream_index) {
            int response = decode_packet(pPacket, pCodecContext, pFrame, pConvertedFrame, pSwsContext);
            if (response < 0)
                break;
        }

        av_packet_unref(pPacket);
        auto end_ts = std::chrono::high_resolution_clock::now();
        double diff = std::chrono::duration<double, std::micro>(end_ts-begin_ts).count();
        double frame_time = (double)pFormatContext->streams[video_stream_index]->avg_frame_rate.den / (double)pFormatContext->streams[video_stream_index]->avg_frame_rate.num * 1e6;
        const double slippage = frame_time / 1.6;
        if (diff < (frame_time - slippage))
            std::this_thread::sleep_for(std::chrono::microseconds(static_cast<long>((frame_time - slippage) - diff)));
    }

    avformat_close_input(&pFormatContext);
    av_packet_free(&pPacket);
    av_frame_free(&pFrame);
    av_frame_free(&pConvertedFrame);
    av_freep(&pConvertedFrameBuffer);
    avformat_free_context(pFormatContext);
    
    return 0;
}


static int decode_packet(AVPacket *pPacket, AVCodecContext *pCodecContext, AVFrame *pFrame, AVFrame *pConvertedFrame, SwsContext *pSwsContext) {
    // supply raw packet as input to a decoder
    int response = avcodec_send_packet(pCodecContext, pPacket);
    if (response < 0) {
        cerr << "ERROR: Could not send packet to codec: " << av_err2str(response) << endl;
        return response;
    }

    while (response >= 0) {
        response = avcodec_receive_frame(pCodecContext, pFrame);
        if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
            break;
        }

        if (response < 0) {
            cerr << "ERROR: Could not receive frame from the decoder: " << av_err2str(response) << endl;
            return response;
        }

        // here we will now perform sws scaling
        sws_scale(pSwsContext, pFrame->data, pFrame->linesize, 0, pCodecContext->height, pConvertedFrame->data, pConvertedFrame->linesize);
        if (draw_frame(pConvertedFrame) < 0) {
            cerr << "ERROR: Could not draw frame" << endl;
            return -1;
        }
    }

    return 0;
}

static int draw_frame(AVFrame *pFrame) {
    // from darkest to brightest
    static const char symbols[] = " .-:*+=%@#";

    ap::Console::clear_console();
    // allocate memory for output string
    char* data = static_cast<char*>(calloc(((pFrame->width + 1) * pFrame->height - 1), sizeof(char)));
    if (!data) {
        cerr << "ERROR: Could not allocate memory for ASCII frame" << endl;
        return -1;
    }

    for (int i = 0; i < pFrame->height; i++) {
        for (int j = 0; j < pFrame->width; j++) {
            int index = static_cast<int>(static_cast<double>(*(pFrame->data[0] + i * pFrame->linesize[0] + j)) / 25.6);
            data[i * (pFrame->width + 1) + j] = symbols[index];
        }

        if (i != pFrame->height - 1) {
            data[i * (pFrame->width + 1) + pFrame->width] = '\n';
        }
    }

    ap::Console::output(data);
    fflush(stdout);
    free(data);

    return 0;
}
