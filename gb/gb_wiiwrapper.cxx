#include <gb/gb_wiiwrapper.hxx>
#include <wiiuse/wpad.h>

GB_WiiWrapper::GB_WiiWrapper() :
    channel_array_ptr_(std::make_shared<ChannelArray>()),
    bus_(channel_array_ptr_),
    apu_(channel_array_ptr_, bus_.GetReference(addr_NR52)),
    ppu_(bus_),
    timer_(channel_array_ptr_, bus_),
    cpu_(bus_, ppu_, apu_, timer_),
    joypad_(bus_.GetReference(addr_joy)),
    interrupt_flag_(bus_.GetReference(addr_if))
{}

void GB_WiiWrapper::Update() {
    uint8_t old_if = interrupt_flag_;
    int clk = 0;
    if (!cpu_.skip_next_)
        clk = cpu_.Update();
    cpu_.skip_next_ = false;
    if (timer_.Update(clk, old_if)) {
        if (cpu_.halt_) {
            cpu_.halt_ = false;
            cpu_.skip_next_ = true;
        }
    }
    ppu_.Update(clk);
    apu_.Update(clk);
}

void GB_WiiWrapper::LoadCartridge(void* data) {
    bus_.LoadCartridge((uint8_t*)data);
    ppu_.UseCGB = bus_.UseCGB;
}

void* GB_WiiWrapper::GetScreenData() {
    return ppu_.GetScreenData();
}

void GB_WiiWrapper::HandleKeyDown(uint32_t key) {
    if ((key & WPAD_BUTTON_RIGHT) || (key & WPAD_BUTTON_LEFT) || (key & WPAD_BUTTON_UP) || (key & WPAD_BUTTON_DOWN)) {
        auto index =
            (key & WPAD_BUTTON_DOWN) ? 0 :
            (key & WPAD_BUTTON_UP) ? 1 :
            (key & WPAD_BUTTON_RIGHT) ? 2 :
            (key & WPAD_BUTTON_LEFT) ? 3 : 0;
        bus_.DirectionKeys &= (~(1UL << index));
        interrupt_flag_ |= IFInterrupt::JOYPAD;
    }
    if ((key & WPAD_BUTTON_1) || (key & WPAD_BUTTON_2) || (key & WPAD_BUTTON_PLUS) || (key & WPAD_BUTTON_MINUS)) {
        auto index =
            (key & WPAD_BUTTON_1) ? 0 :
            (key & WPAD_BUTTON_2) ? 1 :
            (key & WPAD_BUTTON_MINUS) ? 2 :
            (key & WPAD_BUTTON_PLUS) ? 3 : 0;
        bus_.ActionKeys &= (~(1UL << index));
        interrupt_flag_ |= IFInterrupt::JOYPAD;
    }
}
void GB_WiiWrapper::HandleKeyUp(uint32_t key) {
    if ((key & WPAD_BUTTON_RIGHT) || (key & WPAD_BUTTON_LEFT) || (key & WPAD_BUTTON_UP) || (key & WPAD_BUTTON_DOWN)) {
        auto index =
            (key & WPAD_BUTTON_DOWN) ? 0 :
            (key & WPAD_BUTTON_UP) ? 1 :
            (key & WPAD_BUTTON_RIGHT) ? 2 :
            (key & WPAD_BUTTON_LEFT) ? 3 : 0;
        bus_.DirectionKeys |= (1UL << index);
    }
    if ((key & WPAD_BUTTON_1) || (key & WPAD_BUTTON_2) || (key & WPAD_BUTTON_PLUS) || (key & WPAD_BUTTON_MINUS)) {
    auto index =
        (key & WPAD_BUTTON_1) ? 0 :
        (key & WPAD_BUTTON_2) ? 1 :
        (key & WPAD_BUTTON_MINUS) ? 2 :
        (key & WPAD_BUTTON_PLUS) ? 3 : 0;
        bus_.ActionKeys |= (1UL << index);
    }
}