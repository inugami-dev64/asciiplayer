//
// Created by user on 25/04/20.
//

#include "Player.h"
#include <stdexcept>
#include <sstream>
#include <string>
#include <cstdio>

using namespace std;

// Workaround to make av_err2str macro work
#undef av_err2str
#define av_err2str(errnum) \
    av_make_error_string((char*)__builtin_alloca(AV_ERROR_MAX_STRING_SIZE), AV_ERROR_MAX_STRING_SIZE, errnum);

namespace ap {
    Player::Player(const char *filename, Logger logger, VideoPresenter* pVideoPresenter) :
        logger(logger)
    {
        this->pVideoPresenter = pVideoPresenter;
        logger.log(DEBUG, "Initializing all the containers, codecs and protocols");

        pFormatContext = avformat_alloc_context();
        if (!pFormatContext)
            throw runtime_error("Could not allocate memory for format context");

        logger.log(DEBUG, std::string("Attempting to open an input file %s" + std::string(filename)).c_str());

        // Open the file and read its header
        if (avformat_open_input(&pFormatContext, filename, nullptr, nullptr) != 0)
            throw std::runtime_error("Could not open input file " + std::string(filename) + ".");

        stringstream ss;
        // now we have access to some information about the file
        ss << "format " << pFormatContext->iformat->name <<
            ", duration " << pFormatContext->duration <<
            ", bit_rate " << pFormatContext->bit_rate << std::endl;
        logger.log(DEBUG, ss.str().c_str());

        logger.log(DEBUG, "Finding stream info from format");

        // read packets stream information
        if (avformat_find_stream_info(pFormatContext, nullptr) < 0)
            throw std::runtime_error("Could not find stream information");

        // Create codec contexts for video and audio streams
        _findCodecContext();

        pPacket = av_packet_alloc();
        if (!pPacket)
            throw runtime_error("Could not allocate memory for AVPacket");

        this->pVideoPresenter->setFrameRate(
            static_cast<double>(pFormatContext->streams[videoStreamIndex]->r_frame_rate.num) /
            static_cast<double>(pFormatContext->streams[videoStreamIndex]->r_frame_rate.den));
        this->pVideoPresenter->updateSwsContext(
            pVideoCodecContext->width,
            pVideoCodecContext->height,
            pVideoCodecContext->pix_fmt);
    }

    Player::~Player() {
        avformat_close_input(&pFormatContext);
        avcodec_free_context(&pVideoCodecContext);
        if (pAudioCodecContext)
            avcodec_free_context(&pAudioCodecContext);
        av_packet_free(&pPacket);
        avformat_free_context(pFormatContext);
    }

    void Player::play() {
        std::thread videoThread(&VideoPresenter::present, pVideoPresenter);
        while (av_read_frame(pFormatContext, pPacket) >= 0) {
            if (pPacket->stream_index == videoStreamIndex) {
                decodePacket(pVideoCodecContext, pVideoPresenter);
            } else if (pPacket->stream_index == audioStreamIndex) {
                // TODO: Decode audio packet and send received frame(s) into AudioPresenter queue
            }

            av_packet_unref(pPacket);
        }

        pVideoPresenter->setDone(true);
        videoThread.join();
    }

    AVCodecContext* Player::_initializeCodecContext(const AVCodec* pCodec, const AVCodecParameters* pParams) {
        AVCodecContext* pCodecContext = avcodec_alloc_context3(pCodec);
        if (!pCodecContext)
            throw runtime_error("Could not allocate AVCodecContext");

        // Fill the codec context based on the values provided by the codec parameters
        if (avcodec_parameters_to_context(pCodecContext, pParams) < 0)
            throw runtime_error("Failed to copy codec parameters to context");

        // Initialize the AVCodecContext to use given AVCodec
        if (avcodec_open2(pCodecContext, pCodec, nullptr) < 0)
            throw runtime_error("Could not open codec");

        return pCodecContext;
    }

    void Player::_findCodecContext() {
        const AVCodec *pVideoCodec = nullptr;
        const AVCodecParameters *pVideoCodecParams = nullptr;

        const AVCodec *pAudioCodec = nullptr;
        const AVCodecParameters *pAudioCodecParams = nullptr;

        for (unsigned int i = 0; i < pFormatContext->nb_streams; i++) {
            AVCodecParameters *pLocalCodecParams = pFormatContext->streams[i]->codecpar;
            logger.log(DEBUG , ("AVStream->time_base before open coded "s + to_string(pFormatContext->streams[i]->time_base.num) + "/" + to_string(pFormatContext->streams[i]->time_base.den)).c_str());
            logger.log(DEBUG, ("AVStream->r_frame_rate before open coded "s + to_string(pFormatContext->streams[i]->r_frame_rate.num) + "/" + to_string(pFormatContext->streams[i]->r_frame_rate.den)).c_str());
            logger.log(DEBUG, ("AVStream->start_time "s + to_string(pFormatContext->streams[i]->start_time)).c_str());
            logger.log(DEBUG, ("AVStream->duration "s + to_string(pFormatContext->streams[i]->duration)).c_str());

            logger.log(DEBUG, "finding the proper decoder (CODEC)");

            const AVCodec* pLocalCodec = avcodec_find_decoder(pLocalCodecParams->codec_id);
            if (pLocalCodec == nullptr)
                throw runtime_error("Unsupported codec!");

            // we will save the video stream's index, codec parameters
            if (pLocalCodecParams->codec_type == AVMEDIA_TYPE_VIDEO) {
                videoStreamIndex = i;
                pVideoCodec = pLocalCodec;
                pVideoCodecParams = pLocalCodecParams;
            } else if (pLocalCodecParams->codec_type == AVMEDIA_TYPE_AUDIO) {
                audioStreamIndex = i;
                pAudioCodec = pLocalCodec;
                pAudioCodecParams = pLocalCodecParams;
            }
        }

        if (videoStreamIndex == -1)
            throw runtime_error("Could not find a video stream!");

        pVideoCodecContext = _initializeCodecContext(pVideoCodec, pVideoCodecParams);
        pAudioCodecContext = _initializeCodecContext(pAudioCodec, pAudioCodecParams);
    }

    int Player::decodePacket(AVCodecContext* pCodecContext, Presenter *pPresenter) {
        int response = avcodec_send_packet(pVideoCodecContext, pPacket);
        if (response < 0) {
            string errmsg = av_err2str(response);
            logger.log(ERROR, ("Could not send packet to codec: "s + errmsg).c_str());
            return response;
        }

        while (response >= 0) {
            AVFrame *pFrame = av_frame_alloc();
            if (!pFrame)
                throw runtime_error("Could not allocate frame");
            response = avcodec_receive_frame(pCodecContext, pFrame);
            if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
                break;
            }

            if (response < 0) {
                const char* errmsg = av_err2str(response);
                logger.log(ERROR, ("Failed to receive a frame: "s + errmsg).c_str());
                return response;
            }

            pPresenter->addFrame(pFrame);
        }
        return 0;
    }
} // ap