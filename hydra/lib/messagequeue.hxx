#ifndef TKP_MESSAGEQUEUE_H
#define TKP_MESSAGEQUEUE_H
#include <string>
#include <queue>
#include <mutex>
#include <shared_mutex>
#include <memory>
#include <any>

enum class ResponseId : int {
    COMMON_PAUSED = 0x100,
    COMMON_REGISTERS = 0x104,
};

struct Response {
    ResponseId Id;
    std::string Data;
};

enum class RequestId : int {
    COMMON_PAUSE = 0x100,
    COMMON_RESET = 0x101,
    COMMON_START_LOG = 0x102,
    COMMON_STOP_LOG = 0x103,
    COMMON_GET_REGISTERS = 0x104,
};

struct Request {
    RequestId Id;
    // Any type of data a request can have
    std::any Data;
};
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
        ResponseId PeekResponse();
    protected:
        std::queue<Request> requests_;
        std::queue<Response> responses_;
        std::shared_mutex requests_mutex_;
        std::shared_mutex responses_mutex_;
    };
}
#endif