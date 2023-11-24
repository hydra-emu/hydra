#pragma once

#include <cstddef>
#include <cstdint>

namespace hydra
{

    template <std::size_t count>
    class ringbuffer
    {
    public:
        ringbuffer()
        {
            buffer_ = new uint8_t[count];
        }

        ~ringbuffer()
        {
            delete[] buffer_;
        }

        ringbuffer(ringbuffer&&) = default;
        ringbuffer& operator=(ringbuffer&&) = default;

        void write(void* in, std::size_t size)
        {
            uint8_t* in8 = static_cast<uint8_t*>(in);
            for (std::size_t i = 0; i < size; i++)
            {
                buffer_[head_] = in8[i];
                head_ = (head_ + 1) % count;
            }
        }

        void read(void* out, std::size_t size)
        {
            uint8_t* out8 = static_cast<uint8_t*>(out);
            for (std::size_t i = 0; i < size; i++)
            {
                out8[i] = buffer_[tail_];
                tail_ = (tail_ + 1) % count;
            }
        }

        std::size_t size() const
        {
            return (head_ - tail_ + count) % count;
        }

        bool empty() const
        {
            return head_ == tail_;
        }

        void clear()
        {
            head_ = tail_ = 0;
        }

    private:
        ringbuffer(const ringbuffer&) = delete;
        ringbuffer& operator=(const ringbuffer&) = delete;

        uint8_t* buffer_;
        std::size_t head_ = 0;
        std::size_t tail_ = 0;
    };

} // namespace hydra