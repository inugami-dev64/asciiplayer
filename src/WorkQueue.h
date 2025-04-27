//
// Created by user on 25/04/27.
//

#ifndef WORKQUEUE_H
#define WORKQUEUE_H

#ifndef BLOCK_SIZE
#define BLOCK_SIZE 256
#endif

#include <atomic>
#include <cstdint>
#include <mutex>
#include <stdexcept>

namespace ap {

    template<typename T>
    struct QueueNode {
        T data[BLOCK_SIZE] = {};
        QueueNode *next = nullptr;
    };

    template<typename T>
    class WorkQueue {
    public:
        WorkQueue() {
            pHead = new QueueNode<T>();
        }

        ~WorkQueue() {
            QueueNode<T> *pNode = pHead;
            while (pHead != nullptr) {}
        }

        void push(const T& val) {
            std::lock_guard lock(push_mutex);
            uint64_t node_id = length.load() >> 32;
            uint64_t node_length = length.load() & 0xFFFFFFFF;

            // find the last block
            QueueNode<T> *pNode = pHead;
            while (pNode->next)
                pNode = pNode->next;

            // here we will allocate a new node and add it to the list
            if (node_length + 1 >= BLOCK_SIZE) {
                pNode->next = new QueueNode<T>();
                pNode = pNode->next;
                ++node_id;
            }

            pNode->data[node_length++] = val;
            length.store((node_id << 32) | node_length);
        }

        T pop() {
            std::lock_guard lock(pop_mutex);
            uint32_t front_val = front.load();

            // the front counter is in the end of the first node
            if (front_val >= BLOCK_SIZE) {
                std::lock_guard del_lock(push_mutex);
                QueueNode<T> *pPrevious = pHead;
                pHead = pHead->next;
                delete pPrevious;
                front_val = 0;

                uint64_t node_id = (length.load() >> 32) - 1;
                uint64_t node_length = length.load() & 0xFFFFFFFF;
                length.store((node_id << 32) | node_length);
            }

            if (empty())
                throw std::out_of_range("WorkQueue::pop");

            front.store(front_val + 1);
            return std::move(pHead->data[front_val]);
        }

        [[nodiscard]]
        size_t size() const {
            return (length.load() >> 32) * BLOCK_SIZE + (length.load() & 0xFFFFFFFF) - front.load();
        }

        [[nodiscard]]
        bool empty() const {
            return size() == 0;
        }
    private:
        QueueNode<T> *pHead;

        // here comes the fun part, accounting on how many elements exist in the queue
        std::atomic<uint64_t> length = {};
        std::atomic<uint32_t> front = {};

        std::mutex push_mutex = {};
        std::mutex pop_mutex = {};
    };

} // ap

#endif //WORKQUEUE_H
