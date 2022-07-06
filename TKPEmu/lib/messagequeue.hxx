#ifndef TKP_MESSAGEQUEUE_H
#define TKP_MESSAGEQUEUE_H
#include <string>
#include <queue>
#include <mutex>
#include <shared_mutex>
#include <memory>

enum {
    TKPMQ_RESPONSE_MAIN = 1,
};

struct Response {
    uint32_t Recipient;
    std::string Type;
    void* Data;
    size_t Size;
};
using Request = std::string;
// My message queue design pattern implementation
namespace TKPEmu::Tools {
    class MQBase {
    public:
        void PushRequest(Request message);
        Request PopRequest();
        bool PollRequests();
        void PushResponse(Response message);
        Response PopResponse();
        bool PollResponses();
        // Checks the response type before copying the entire response over
        uint32_t PeekResponseRecipient();
    protected:
        std::queue<Request> requests_;
        std::queue<Response> responses_;
        std::shared_mutex requests_mutex_;
        std::shared_mutex responses_mutex_;
    };
}
#endif