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

namespace ap {
    /**
     * The main video player class
     * Just feed the filename and play the video, and it does the magic :D
     */
    class Player {
    public:
        explicit Player(const char* filename, Logger logger, VideoPresenter* pVideoPresenter);
        ~Player();

        void play();

    private:
        VideoPresenter* pVideoPresenter;
        Logger logger;
        AVFormatContext* pFormatContext = nullptr;
        AVCodecContext* pVideoCodecContext = nullptr;
        unsigned int videoStreamIndex = -1;
        AVCodecContext* pAudioCodecContext = nullptr;
        unsigned int audioStreamIndex = -1;

        // decoding specific stuff
        AVPacket* pPacket = nullptr;

        static AVCodecContext* _initializeCodecContext(const AVCodec* pCodec, const AVCodecParameters* pParams);
        void _findCodecContext();
        int decodePacket(AVCodecContext *pCodecContext, Presenter *pPresenter);
    };

} // ap

#endif //PLAYER_H
