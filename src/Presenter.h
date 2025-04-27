//
// Created by user on 25/04/28.
//

#ifndef PRESENTER_H
#define PRESENTER_H

#include <thread>

extern "C" {
    #include <libavutil/frame.h>
}
#include "WorkQueue.h"

namespace ap {
    /**
     * The interface type for stream presenters
     */
    class Presenter {
    public:
        Presenter(double _frameRate, unsigned int _sampleRate = 48000) :
            frameRate(_frameRate),
            sampleRate(_sampleRate) {}
        virtual ~Presenter() = default;

        /**
         * Starts the presenter thread
         */
        virtual void present() = 0;

        void setFrameRate(double _frameRate) {
            frameRate = _frameRate;
        }

        double getFrameRate() const {
            return frameRate;
        }
    protected:
        double frameRate;
        unsigned int sampleRate;
        std::thread presenter;
        WorkQueue<AVFrame*> workQueue;

        /**
         * Transforms given frame into something that can be consumed by the presenter
         * @param frame
         */
        virtual void transform(AVFrame* frame) = 0;
    };

} // ap

#endif //PRESENTER_H
