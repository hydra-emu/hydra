#include "messagequeue.hxx"

namespace TKPEmu::Tools {
    Message MQBase::Pop() {
        std::unique_lock<std::mutex> lg(queue_mutex_);
        Message ret = messages_.front();
        messages_.pop();
        return ret;
    }
    bool MQBase::Poll() {
        std::unique_lock<std::mutex> lg(queue_mutex_);
        return !messages_.empty();
    }
    void MQBase::Push(Message message) {
        std::unique_lock<std::mutex> lg(queue_mutex_);
        messages_.push(message);
    }
}