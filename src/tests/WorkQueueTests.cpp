//
// Created by user on 25/04/27.
//
#include <gtest/gtest.h>
#include <thread>

#define BLOCK_SIZE 8
#include "../WorkQueue.h"


TEST(WorkQueueTests, PushAndPopWorksAndFIFOIsPresent) {
    ap::WorkQueue<int> queue;
    queue.push(10);
    queue.push(20);
    queue.push(30);
    queue.push(40);

    EXPECT_EQ(10, queue.pop());
    EXPECT_EQ(20, queue.pop());
    EXPECT_EQ(30, queue.pop());
    EXPECT_EQ(40, queue.pop());
}

TEST(WorkQueueTests, PushItems_EnsureThatQueueSizeIsCorrect) {
    ap::WorkQueue<int> queue;
    queue.push(10);
    queue.push(20);
    queue.push(30);

    EXPECT_EQ(3, queue.size());
}

TEST(WorkQueueTests, PushAndPopItems_EnsureThatQueueSizeIsCorrect) {
    ap::WorkQueue<int> queue;
    queue.push(10);
    queue.push(20);
    queue.push(30);

    queue.pop();
    queue.pop();
    EXPECT_EQ(1, queue.size());
}

TEST(WorkQueueTests, PushAndPopItems_NodeReallocation_EnsureThatQueueSizeRemainsCorrect) {
    ap::WorkQueue<int> queue;
    queue.push(10);
    queue.push(20);
    queue.push(30);
    queue.push(40);
    queue.push(50);
    queue.push(60);
    queue.push(70);
    queue.push(80);
    queue.push(90);
    queue.push(100);

    queue.pop();
    queue.pop();
    queue.pop();
    queue.pop();
    queue.pop();
    queue.pop();
    queue.pop();
    queue.pop();
    queue.pop();

    EXPECT_EQ(1, queue.size());
}

TEST(WorkQueueTests, PopTooManyTimes_ExpectRuntimeException) {
    ap::WorkQueue<int> queue;
    EXPECT_THROW(queue.pop(), std::out_of_range);
}

TEST(WorkQueueTests, PushAndPopOnSeparateThreads) {
    ap::WorkQueue<int> queue;

    std::thread pusher([](ap::WorkQueue<int>* q) {
        int values[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150, 160, 170, 180, 190, 200, 210, 220, 230, 240, 250};
        for (int val : values)
            q->push(val);
        q->setDone(true);
    }, &queue);

    std::thread popper([](ap::WorkQueue<int>* q) {
        while (!q->isDone() || !q->empty()) {
            if (q->empty()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }
            int val = q->pop();
            EXPECT_EQ(val * 1000, val * 1000);
        }
    }, &queue);

    pusher.join();
    popper.join();
}