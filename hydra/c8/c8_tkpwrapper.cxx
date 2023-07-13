#include <c8/c8_tkpwrapper.hxx>
#include <iostream>

namespace hydra::c8 {
    c8::c8() {}
    // c8::c8(std::unique_ptr<OptionsBase> args) : c8() {
	// 	// key_mappings_ = std::any_cast<c8Keys>(args);
	// }
	c8::~c8() {
		Stopped.store(true);
	}
    bool c8::load_file(std::string path) {
		inter_.load_file(path);
        Loaded = true;
        return true;
    }
    void c8::update() {
        inter_.Update();
    }
    void c8::reset() {
        inter_.reset();
    }
	void c8::HandleKeyDown(uint32_t key) {
		for (int i = 0; i < 16; i++) {
			if (key_mappings_[i] == key) {
				inter_.key_pressed_[i] = true;
				break;
			}
		}
	}
	void c8::HandleKeyUp(uint32_t key) {
		for (int i = 0; i < 16; i++) {
			if (key_mappings_[i] == key) {
				inter_.key_pressed_[i] = false;
				break;
			}
		}
	}
	void* c8::GetScreenData() {
		return inter_.GetScreenData();
	}
}