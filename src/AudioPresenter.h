#ifndef AUDIOPRESENTER_H
#define AUDIOPRESENTER_H

#include "Presenter.h"
#include <portaudio.h>

#define DEFAULT_SAMPLE_RATE 44100

namespace ap {
    class AudioPresenter final : public Presenter {
    public:
        AudioPresenter(int sampleRate = DEFAULT_SAMPLE_RATE);

        void present() override;
        void transform(AVFrame* frame) override {};

        // Setter methods
        inline void setSampleRate(int sampleRate) { this->sampleRate = sampleRate; }
        inline void setOutputChannels(int outputChannels) { this->outputChannels = outputChannels; }
        inline void setSampleFormat(AVSampleFormat sampleFormat) { this->sampleFormat = sampleFormat; }
        inline void setChannelLayout(const AVChannelLayout& layout) { this->channelLayout = layout; }
    private:
        int outputChannels = 0;
        int samplesPerFrame = 0;
        AVSampleFormat sampleFormat = AV_SAMPLE_FMT_NONE;
        AVChannelLayout channelLayout = {};
        size_t sampleStride = 0;
        bool isPlanar = 0;

        void _setupAudioParameters(PaStreamParameters* pParams);
    };
}

#endif