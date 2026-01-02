#include "AudioPresenter.h"

extern "C" {
    #include <libswresample/swresample.h>
    #include <libavutil/audio_fifo.h>
}

using namespace std;

#define FRAMES_PER_BUFFER 1024
#define DST_SAMPLE_FORMAT AV_SAMPLE_FMT_S16
#define PA_DST_SAMPLE_FORMAT paInt16

namespace ap {
    AudioPresenter::AudioPresenter(int sampleRate) : Presenter(0, sampleRate) {
        PaError err = Pa_Initialize();
        if (err != paNoError)
            throw runtime_error("Failed to initialize portaudio");
    }

    void AudioPresenter::_setupAudioParameters(PaStreamParameters* pParams) {
        pParams->device = Pa_GetDefaultOutputDevice();
        pParams->channelCount = this->outputChannels;
        pParams->sampleFormat = PA_DST_SAMPLE_FORMAT;
        pParams->suggestedLatency = Pa_GetDeviceInfo(pParams->device)->defaultHighOutputLatency;
    }

    void AudioPresenter::present() {
        // Setup output stream
        PaStreamParameters outputParameters = {0};
        this->_setupAudioParameters(&outputParameters);

        SwrContext *pSwrContext = nullptr;
        int ret = swr_alloc_set_opts2(
            &pSwrContext,
            &this->channelLayout,
            DST_SAMPLE_FORMAT,
            this->sampleRate,
            &this->channelLayout,
            this->sampleFormat,
            this->sampleRate,
            0,
            nullptr
        );

        if (ret < 0)
            throw runtime_error("Failed to set SwrContext options");

        if ((ret = swr_init(pSwrContext)) < 0)
            throw runtime_error("Failed to initialize SwrContext");

        PaStream *stream;
        PaError err = Pa_OpenStream(
            &stream,
            nullptr,
            &outputParameters,
            this->sampleRate,
            FRAMES_PER_BUFFER,
            paNoFlag,
            nullptr,
            nullptr
        );

        if (err != paNoError)
            throw runtime_error("Failed to open PortAudio stream");

        err = Pa_StartStream(stream);
        if (err != paNoError)
            throw runtime_error("Failed to start PortAudio stream");

        int dstLineSize = 0;
        int dstNbSamples = FRAMES_PER_BUFFER;

        uint8_t **chunk = nullptr;
        int maxNbSamples = 0;
        ret = av_samples_alloc_array_and_samples(&chunk, &dstLineSize, this->outputChannels, dstNbSamples, DST_SAMPLE_FORMAT, 0);
        if (ret < 0)
            throw runtime_error("Failed to allocate output buffer");

        AVAudioFifo* fifo = av_audio_fifo_alloc(DST_SAMPLE_FORMAT, this->outputChannels, 2 * FRAMES_PER_BUFFER);

        uint8_t** buf = nullptr;

        // TODO: Audio processing here
        while (!workQueue.isDone() || !workQueue.empty()) {
            if (workQueue.empty()) {
                this_thread::sleep_for(chrono::milliseconds(2));
                continue;
            }

            AVFrame* pFrame = workQueue.pop();

            dstNbSamples = swr_get_out_samples(pSwrContext, pFrame->nb_samples);

            if (dstNbSamples > maxNbSamples) {
                if (buf) av_freep(&buf[0]);
                av_freep(&buf);

                ret = av_samples_alloc_array_and_samples(&buf, &dstLineSize, this->outputChannels, dstNbSamples, DST_SAMPLE_FORMAT, 0);
                if (ret < 0)
                    throw runtime_error("Failed to allocate output buffer");

                maxNbSamples = dstNbSamples;
            }

            ret = swr_convert(pSwrContext, buf, dstNbSamples, const_cast<const uint8_t**>(pFrame->extended_data), pFrame->nb_samples);
            if (ret < 0)
                throw runtime_error("Failed to convert samples");

            ret = av_audio_fifo_write(fifo, reinterpret_cast<void**>(buf), dstNbSamples);
            if (ret < 0)
                throw runtime_error("Failed to write samples to audio FIFO");

            while (av_audio_fifo_size(fifo) >= FRAMES_PER_BUFFER) {
                av_audio_fifo_read(fifo, reinterpret_cast<void**>(chunk), FRAMES_PER_BUFFER);
                Pa_WriteStream(stream, chunk[0], FRAMES_PER_BUFFER);
            }

            av_frame_free(&pFrame);
        }

        err = Pa_StopStream(stream);
        if (err != paNoError)
            throw runtime_error("Failed to stop PortAudio stream");

        err = Pa_CloseStream(stream);
        if (err != paNoError)
            throw runtime_error("Failed to close PortAudio stream");

        av_audio_fifo_free(fifo);

        if (buf) av_freep(&buf[0]);
        av_freep(&buf);

        if (chunk) av_freep(&chunk[0]);
        av_freep(&chunk);

        swr_free(&pSwrContext);
    }
}