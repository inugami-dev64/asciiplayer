//
// Created by user on 25/04/20.
//

#ifndef PLAYER_H
#define PLAYER_H

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
}

#include "Logger.h"
#include "VideoPresenter.h"
#include "AudioPresenter.h"

namespace ap {
    /**
     * The main video player class
     * Just feed the filename and play the video, and it does the magic :D
     */
    class Player {
    public:
        explicit Player(const char* filename, Logger logger, VideoPresenter* pVideoPresenter, AudioPresenter *pAudioPresenter);
        ~Player();

        void play();

    private:
        VideoPresenter* pVideoPresenter;
        AudioPresenter* pAudioPresenter;
        Logger logger;
        AVFormatContext* pFormatContext = nullptr;
        AVCodecContext* pVideoCodecContext = nullptr;
        unsigned int videoStreamIndex = -1;
        AVCodecContext* pAudioCodecContext = nullptr;
        unsigned int audioStreamIndex = -1;

        // Frametime will be used to prevent decoding thread from decoding too many
        // frames at once and thus avoid excessive memory consumption
        double frameTime = 0.0;

        // decoding specific stuff
        AVPacket* pPacket = nullptr;

        static AVCodecContext* _initializeCodecContext(const AVCodec* pCodec, const AVCodecParameters* pParams);
        void _findCodecContext();
        int _decodePacket(AVCodecContext *pCodecContext, Presenter *pPresenter);
    };

} // ap

#endif //PLAYER_H
