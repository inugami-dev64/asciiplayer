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

namespace ap {
    /**
     * The main video player class
     * Just feed the filename and play the video, and it does the magic :D
     */
    class Player {
    public:
        explicit Player(const char* filename, Logger logger);
        ~Player();

        void play();

    private:
        Logger logger;
        AVFormatContext* pFormatContext = nullptr;
        AVCodecContext* pVideoCodecContext = nullptr;
        unsigned int videoStreamIndex = -1;
        AVCodecContext* pAudioCodecContext = nullptr;
        unsigned int audioStreamIndex = -1;

        // video rendering specific stuff
        AVPacket* pPacket = nullptr;

        static AVCodecContext* _initializeCodecContext(const AVCodec* pCodec, const AVCodecParameters* pParams);
        void _findCodecContext();
    };

} // ap

#endif //PLAYER_H
