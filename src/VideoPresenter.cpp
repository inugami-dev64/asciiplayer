//
// Created by user on 25/04/27.
//

#include "VideoPresenter.h"

#include <libavutil/imgutils.h>

#include "Console.h"

using namespace std;

namespace ap {
    VideoPresenter::VideoPresenter(double frameRate, int width, int height, AVPixelFormat pixelFormat) :
        Presenter(frameRate)
    {
        if (width != 0 && height != 0) {
            updateSwsContext(width, height, pixelFormat);
            this->strideY = height;
        }
    }

    VideoPresenter::~VideoPresenter() {
        // free previously allocated resources
        presenter.join();
        if (pFramebuffer)
            av_freep(&pFramebuffer);
        if (pConvertedFrame)
            av_frame_free(&pConvertedFrame);
        if (pConvertedFrameBuffer)
            av_freep(&pConvertedFrameBuffer);
        if (pSwsContext)
            sws_freeContext(pSwsContext);
    }

    void VideoPresenter::present() {
        auto beginTime = chrono::high_resolution_clock::now();
        while (!workQueue.isDone() || !workQueue.empty()) {
            if (workQueue.empty()) {
                this_thread::sleep_for(std::chrono::milliseconds(2));
                continue;
            }

            AVFrame* pFrame = workQueue.pop();
            this->transform(pFrame);

            Console::clear_console();
            memset(pFramebuffer, 0, (pConvertedFrame->width + 1) * (pConvertedFrame->height + 1));

            for (int i = 0; i < pConvertedFrame->height; i++) {
                for (int j = 0; j < pConvertedFrame->width; j++) {
                    int index = static_cast<int>(static_cast<double>(*(pConvertedFrame->data[0] + i * pConvertedFrame->linesize[0] + j)) / 25.6);
                    pFramebuffer[i * (pConvertedFrame->width + 1) + j] = symbols[index];
                }

                if (i != pConvertedFrame->height - 1) {
                    pFramebuffer[i * (pConvertedFrame->width + 1) + pConvertedFrame->width] = '\n';
                }
            }

            Console::output(pFramebuffer);
            fflush(stdout);


            auto endTime = chrono::high_resolution_clock::now();
            double diff = std::chrono::duration<double, std::milli>(endTime - beginTime).count();
            double frame_time = 1000 / frameRate;
            if (diff < frame_time)
                this_thread::sleep_for(chrono::milliseconds(static_cast<long>(frame_time - diff)));
            beginTime = chrono::high_resolution_clock::now();
        }

        Console::clear_console();
    }

    void VideoPresenter::transform(AVFrame *pFrame) {
        sws_scale(
            pSwsContext,
            pFrame->data,
            pFrame->linesize,
            0,
            strideY,
            pConvertedFrame->data,
            pConvertedFrame->linesize);
    }

    void VideoPresenter::updateSwsContext(int width, int height, AVPixelFormat pixelFormat) {
        if (width <= 0 || height <= 0)
            throw out_of_range("width and height must be greater than 0");

        // free previously allocated resources
        if (pFramebuffer)
            av_freep(&pFramebuffer);
        if (pConvertedFrame)
            av_frame_free(&pConvertedFrame);
        if (pConvertedFrameBuffer)
            av_freep(&pConvertedFrameBuffer);
        if (pSwsContext)
            sws_freeContext(pSwsContext);

        this->strideY = height;

        Rectangle<int> consoleDimensions = Console::get_console_dimensions();

        pSwsContext = sws_getContext(
            width,
            height,
            pixelFormat,
            consoleDimensions.width,
            consoleDimensions.height,
            AV_PIX_FMT_GRAY8,
            SWS_BICUBIC,
            nullptr, nullptr, nullptr);

        pConvertedFrame = av_frame_alloc();
        if (!pConvertedFrame)
            throw runtime_error("Could not allocate memory for AVFrame");

        pConvertedFrame->width = consoleDimensions.width;
        pConvertedFrame->height = consoleDimensions.height;

        int numBytes = av_image_get_buffer_size(AV_PIX_FMT_GRAY8, pConvertedFrame->width, pConvertedFrame->height, 1);
        pConvertedFrameBuffer = static_cast<uint8_t*>(av_malloc(numBytes * sizeof(uint8_t)));
        int response = av_image_fill_arrays(pConvertedFrame->data, pConvertedFrame->linesize, pConvertedFrameBuffer, AV_PIX_FMT_GRAY8, pConvertedFrame->width, pConvertedFrame->height, 1);
        if (response < 0) {
            av_free(pConvertedFrameBuffer);
            throw runtime_error("Could not fill buffer");
        }

        pFramebuffer = static_cast<char*>(av_malloc((pConvertedFrame->width + 1) * (pConvertedFrame->height + 1) * sizeof(char)));
    }
} // ap