#include "n64_cpu.hxx"
#include <iostream>
#include "utils.hxx"
#include "n64_addresses.hxx"
#include <log.hxx>
#include <fmt/format.h>

namespace hydra::N64 {
    void CPU::handle_event() {
        auto event_type = scheduler_.top().type;
        switch (event_type) {
            case SchedulerEventType::PiDmaComplete: {
                break;
            }
        }
        scheduler_.pop();
    }

    void CPU::queue_event(SchedulerEventType type, int time) {
        SchedulerEvent event{type, cpubus_.time_ + time};
        scheduler_.push(event);
    }
}