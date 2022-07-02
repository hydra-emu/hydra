#ifndef TKP_MESSAGEQUEUE_H
#define TKP_MESSAGEQUEUE_H
#include <string>
#include <queue>
#include <mutex>
#include <memory>

struct Message {
    std::string Type;
    std::shared_ptr<void> Data;
    size_t Size;
};
// My many <-> one message queue design pattern implementation
namespace TKPEmu::Tools {
    class MQBase {
    public:
        void Push(Message message);
        Message Pop();
        bool Poll();
    protected:
        std::queue<Message> messages_;
        std::mutex queue_mutex_;
    };
    class MQClient final : public MQBase {
        MQClient(std::shared_ptr<MQBase> server);
    private:
        std::shared_ptr<MQBase> server_;
    };
    class MQServer final : public MQBase {};
}
#endif