#include "messagequeue.hxx"

namespace TKPEmu::Tools {
    Response MQBase::PopResponse() {
        std::unique_lock lg(responses_mutex_);
        Response ret = responses_.front();
        responses_.pop();
        return ret;
    }
    bool MQBase::PollResponses() {
        std::shared_lock lg(responses_mutex_);
        return !responses_.empty();
    }
    void MQBase::PushResponse(Response message) {
        std::unique_lock lg(responses_mutex_);
        responses_.push(message);
    }
    ResponseId MQBase::PeekResponse() {
        return responses_.front().Id;
    }
    Request MQBase::PopRequest() {
        std::unique_lock lg(requests_mutex_);
        Request ret = requests_.front();
        requests_.pop();
        return ret;
    }
    bool MQBase::PollRequests() {
        std::shared_lock lg(requests_mutex_);
        return !requests_.empty();
    }
    void MQBase::PushRequest(Request message) {
        std::unique_lock lg(requests_mutex_);
        requests_.push(message);
    }
}