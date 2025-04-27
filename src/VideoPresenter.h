//
// Created by user on 25/04/27.
//

#ifndef VIDEOPRESENTER_H
#define VIDEOPRESENTER_H
#include "Player.h"
#include "Presenter.h"

namespace ap {

    class VideoPresenter final : public Presenter {
    public:
        VideoPresenter(double frameRate = 24.0, int width = 0, int height = 0, AVPixelFormat pixelFormat = 0);
        ~VideoPresenter() override;

        void present() override;
        void transform(AVFrame *frame) override;
        void updateSwsContext(int width, int height, AVPixelFormat pixelFormat);

    private:
        static const char symbols[] = " .-:*+=%@#";
        int strideY = 0;
        char* pFramebuffer = nullptr;
        SwsContext* pSwsContext = nullptr;

        AVFrame* pConvertedFrame = nullptr;
        uint8_t* pConvertedFrameBuffer = nullptr;
    };

} // ap

#endif //VIDEOPRESENTER_H
