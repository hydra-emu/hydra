#pragma once
#ifndef TKP_THREADPOOL_H
#define TKP_THREADPOOL_H
#include <vector>
#include <thread>
#include <functional>

namespace TKPEmu::Tools {
    class FixedTaskThreadPool {
    public:
        FixedTaskThreadPool(std::vector<std::function<void()>> jobs) : jobs_(std::move(jobs)) {}
        ~FixedTaskThreadPool() = default;
        FixedTaskThreadPool(const FixedTaskThreadPool&) = delete;

        void StartAllAndWait() {
            auto cores = std::max(1u, std::thread::hardware_concurrency());
            for (auto i = 0u; i < cores; i++) {
                threads_.push_back(std::thread(&FixedTaskThreadPool::ThreadLoop, this));
            }
            for (auto i = 0u; i < cores; i++) {
                threads_[i].join();
            }
        }
    private:
        void ThreadLoop() {
            while(true) {
                std::function<void()> job;
                {
                    std::lock_guard<std::mutex> lock(jobs_mutex_);
                    if (jobs_.size() == 0)
                        return;
                    job = jobs_.back();
                    jobs_.pop_back();
                }
                job();
            }
        }
        std::vector<std::thread> threads_;
        std::vector<std::function<void()>> jobs_;
        std::mutex jobs_mutex_;
    };
}
#endif