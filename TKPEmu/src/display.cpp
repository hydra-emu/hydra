#define NOMINMAX
#define STB_IMAGE_IMPLEMENTATION
#include <memory>
#include <thread>
#include <iostream>
#include <algorithm>
#include <include/display.h>
#include <include/disassembly_instr.h>
#include <GameboyTKP/gb_disassembler.h>
#include <GameboyTKP/gb_tracelogger.h>
#include <N64TKP/n64_tkpargs.hxx>
#include <include/emulator_disassembler.hxx>
#include <include/error_factory.hxx>

namespace TKPEmu::Graphics {
	Display::DisplayInitializer::DisplayInitializer() {
        if (SDL_Init(SDL_INIT_VIDEO) != 0) [[unlikely]] {
            SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
            exit(1);
        }
        if(SDL_Init(SDL_INIT_AUDIO) != 0) [[unlikely]] {
            SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
            exit(1);
        }
        std::cout << "SDL initialized successfully" << std::endl;
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	}
    Display::DisplayInitializer::~DisplayInitializer() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        SDL_Quit();
    }
    Display::Display() :
        display_initializer_(),
        settings_manager_(settings_, "/tkp_user.ini"),
        window_ptr_(nullptr, SDL_DestroyWindow),
        gl_context_ptr_(nullptr, SDL_GL_DeleteContext)
    {
        std::ios_base::sync_with_stdio(false);
        ImGuiSettingsFile = settings_manager_.GetSavePath() + ImGuiSettingsFile;
        for (int i = 0; i < RecentRomsMaxSize; ++i) {
            std::string loc = settings_.at("General.recent" + std::to_string(i));
            std::filesystem::path path = loc;
            if (!loc.empty() && path.has_filename()) {
                recent_paths_.push_back(path);
            }
        }
        // TODO: get rid of this enum warning
        SDL_WindowFlags window_flags = static_cast<SDL_WindowFlags>(
            SDL_WINDOW_OPENGL
            | SDL_WINDOW_RESIZABLE
            | SDL_WINDOW_ALLOW_HIGHDPI
            );
        window_ptr_.reset(SDL_CreateWindow(
            "TKPEmu",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            window_settings_.window_width,
            window_settings_.window_height,
            window_flags
        ));
        gl_context_ptr_.reset(SDL_GL_CreateContext(window_ptr_.get()));
        SDL_SetWindowMinimumSize(window_ptr_.get(), window_settings_.minimum_width, window_settings_.minimum_height);
        SDL_SetWindowMaximumSize(window_ptr_.get(), window_settings_.maximum_width, window_settings_.maximum_height);
        SDL_GL_MakeCurrent(window_ptr_.get(), gl_context_ptr_.get());
        glewInit();
        glGenFramebuffers(1, &frame_buffer_);
    }
    Display::~Display() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &frame_buffer_);
    }
    void Display::EnterMainLoop() {
        main_loop();
    }
    void Display::draw_settings(bool* draw) {
        static KeySelector gb_key_right("Direction Right:", "Gameboy.key_right", gb_keys_direction_[0]);
        static KeySelector  gb_key_left("Direction Left: ", "Gameboy.key_left", gb_keys_direction_[1]);
        static KeySelector    gb_key_up("Direction Up:   ", "Gameboy.key_up", gb_keys_direction_[2]);
        static KeySelector  gb_key_down("Direction Down: ", "Gameboy.key_down", gb_keys_direction_[3]);
        static KeySelector     gb_key_a("Action A:       ", "Gameboy.key_a", gb_keys_action_[0]);
        static KeySelector     gb_key_b("Action B:       ", "Gameboy.key_b", gb_keys_action_[1]);
        static KeySelector   gb_key_sel("Action Select:  ", "Gameboy.key_select", gb_keys_action_[2]);
        static KeySelector gb_key_start("Action Start:   ", "Gameboy.key_start", gb_keys_action_[3]);

        static KeySelector     c8_key_0("Key 0:", "Chip8.key_0", chip8_keys_[0]);
        static KeySelector     c8_key_1("Key 1:", "Chip8.key_1", chip8_keys_[1]);
        static KeySelector     c8_key_2("Key 2:", "Chip8.key_2", chip8_keys_[2]);
        static KeySelector     c8_key_3("Key 3:", "Chip8.key_3", chip8_keys_[3]);
        static KeySelector     c8_key_4("Key 4:", "Chip8.key_4", chip8_keys_[4]);
        static KeySelector     c8_key_5("Key 5:", "Chip8.key_5", chip8_keys_[5]);
        static KeySelector     c8_key_6("Key 6:", "Chip8.key_6", chip8_keys_[6]);
        static KeySelector     c8_key_7("Key 7:", "Chip8.key_7", chip8_keys_[7]);
        static KeySelector     c8_key_8("Key 8:", "Chip8.key_8", chip8_keys_[8]);
        static KeySelector     c8_key_9("Key 9:", "Chip8.key_9", chip8_keys_[9]);
        static KeySelector     c8_key_a("Key A:", "Chip8.key_a", chip8_keys_[10]);
        static KeySelector     c8_key_b("Key B:", "Chip8.key_b", chip8_keys_[11]);
        static KeySelector     c8_key_c("Key C:", "Chip8.key_c", chip8_keys_[12]);
        static KeySelector     c8_key_d("Key D:", "Chip8.key_d", chip8_keys_[13]);
        static KeySelector     c8_key_e("Key E:", "Chip8.key_e", chip8_keys_[14]);
        static KeySelector     c8_key_f("Key F:", "Chip8.key_f", chip8_keys_[15]);
        if (*draw) {
            TKPEmu::Applications::IMApplication::SetupWindow(ImVec2(400, 400), ImVec2(700, 700));
            if (!ImGui::Begin("Settings", draw)) {
                ImGui::End();
                return;
            }
            if (ImGui::CollapsingHeader("General")) {
                ImGui::Text("General emulation settings:");
                ImGui::Separator();
                if (ImGui::Checkbox("Debug mode", &debug_mode_)) {
                    settings_.at("General.debug_mode") = debug_mode_ ? "1" : "0";
                }
                if (ImGui::Checkbox("Fast mode", &fast_mode_)) {
                    settings_.at("General.fast_mode") = fast_mode_ ? "1" : "0";
                    if (emulator_ != nullptr) {
                        emulator_->FastMode = fast_mode_;
                    }
                }
                if (ImGui::Checkbox("Skip boot sequence", &skip_boot_)) {
                    settings_.at("General.skip_boot") = skip_boot_ ? "1" : "0";
                    if (emulator_ != nullptr) {
                        emulator_->SkipBoot = skip_boot_;
                    }
                }
                ImGui::NewLine();
            }
            if (ImGui::CollapsingHeader("Video")) {
                ImGui::Text("General video settings:");
                ImGui::Separator();
                if (ImGui::Checkbox("Limit FPS", &limit_fps_)) {
                    settings_.at("Video.limit_fps") = limit_fps_ ? "1" : "0";

                }
                if (ImGui::IsItemHovered()) {
                    ImGui::BeginTooltip();
                    ImGui::Text("Enable to save CPU cycles.");
                    ImGui::EndTooltip();
                }
                if (limit_fps_) {
                    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.4f);
                    if (ImGui::InputInt("##max_fps", &max_fps_, 15, 15, ImGuiInputTextFlags_AllowTabInput)) {
                        max_fps_ = ((max_fps_ + 15 / 2) / 15) * 15;
                        if (max_fps_ <= 0) {
                            max_fps_ = 15;
                        }
                        settings_.at("Video.max_fps") = std::to_string(max_fps_);
                        sleep_time_ = 1000.0f / max_fps_;
                    }
                    ImGui::PopItemWidth();
                }
                ImGui::NewLine();
            }
            if (ImGui::CollapsingHeader("Gameboy")) {
                ImGui::Text("Palette:");
                ImGui::Separator();
                if (ImGui::ColorEdit3("Color 1", gb_palettes_[0].data(), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel)) {
                    std::stringstream color_stream;
                    color_stream << std::setfill('0') 
                        << std::hex << std::setw(2) << (static_cast<int>(gb_palettes_[0][0] * 255.0f) & 0xFF)
                        << std::hex << std::setw(2) << (static_cast<int>(gb_palettes_[0][1] * 255.0f) & 0xFF)
                        << std::hex << std::setw(2) << (static_cast<int>(gb_palettes_[0][2] * 255.0f) & 0xFF);
                    settings_.at("Gameboy.color0") = color_stream.str();
                    setup_emulator_specific();
                }
                ImGui::SameLine();
                if (ImGui::ColorEdit3("Color 2", gb_palettes_[1].data(), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel)) {
                    std::stringstream color_stream;
                    color_stream << std::setfill('0') 
                        << std::hex << std::setw(2) << (static_cast<int>(gb_palettes_[1][0] * 255.0f) & 0xFF)
                        << std::hex << std::setw(2) << (static_cast<int>(gb_palettes_[1][1] * 255.0f) & 0xFF)
                        << std::hex << std::setw(2) << (static_cast<int>(gb_palettes_[1][2] * 255.0f) & 0xFF);
                    settings_.at("Gameboy.color1") = color_stream.str();
                    setup_emulator_specific();
                }
                ImGui::SameLine();
                if (ImGui::ColorEdit3("Color 3", gb_palettes_[2].data(), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel)) {
                    std::stringstream color_stream;
                    color_stream << std::setfill('0') 
                        << std::hex << std::setw(2) << (static_cast<int>(gb_palettes_[2][0] * 255.0f) & 0xFF)
                        << std::hex << std::setw(2) << (static_cast<int>(gb_palettes_[2][1] * 255.0f) & 0xFF)
                        << std::hex << std::setw(2) << (static_cast<int>(gb_palettes_[2][2] * 255.0f) & 0xFF);
                    settings_.at("Gameboy.color2") = color_stream.str();
                    setup_emulator_specific();
                }
                ImGui::SameLine();
                if (ImGui::ColorEdit3("Color 4", gb_palettes_[3].data(), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel)) {
                    std::stringstream color_stream;
                    color_stream << std::setfill('0') 
                        << std::hex << std::setw(2) << (static_cast<int>(gb_palettes_[3][0] * 255.0f) & 0xFF)
                        << std::hex << std::setw(2) << (static_cast<int>(gb_palettes_[3][1] * 255.0f) & 0xFF)
                        << std::hex << std::setw(2) << (static_cast<int>(gb_palettes_[3][2] * 255.0f) & 0xFF);
                    settings_.at("Gameboy.color3") = color_stream.str();
                    setup_emulator_specific();
                }
                ImGui::SameLine();
                ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("Click on a color to open the color picker.");
                }
                if (ImGui::Button("Reset")) {
                    // TODO: Cleanup code duplication
                    settings_.at("Gameboy.color0") = "ffffff";
                    settings_.at("Gameboy.color1") = "aaaaaa";
                    settings_.at("Gameboy.color2") = "555555";
                    settings_.at("Gameboy.color3") = "000000";
                    gb_palettes_[0][0] = 0xff / 255.0f;
                    gb_palettes_[0][1] = 0xff / 255.0f;
                    gb_palettes_[0][2] = 0xff / 255.0f;
                    gb_palettes_[1][0] = 0xaa / 255.0f;
                    gb_palettes_[1][1] = 0xaa / 255.0f;
                    gb_palettes_[1][2] = 0xaa / 255.0f;
                    gb_palettes_[2][0] = 0x55 / 255.0f;
                    gb_palettes_[2][1] = 0x55 / 255.0f;
                    gb_palettes_[2][2] = 0x55 / 255.0f;
                    gb_palettes_[3][0] = 0x00 / 255.0f;
                    gb_palettes_[3][1] = 0x00 / 255.0f;
                    gb_palettes_[3][2] = 0x00 / 255.0f;
                    setup_emulator_specific();
                }
                ImGui::Spacing();
                ImGui::Text("Controls:");
                ImGui::Separator();
                gb_key_right.Draw(last_key_pressed_);
                gb_key_left.Draw(last_key_pressed_);
                gb_key_up.Draw(last_key_pressed_);
                gb_key_down.Draw(last_key_pressed_);
                gb_key_a.Draw(last_key_pressed_);
                gb_key_b.Draw(last_key_pressed_);
                gb_key_sel.Draw(last_key_pressed_);
                gb_key_start.Draw(last_key_pressed_);
            }
            if (ImGui::CollapsingHeader("Chip8")) {
                c8_key_0.Draw(last_key_pressed_);
                ImGui::SameLine();
                c8_key_1.Draw(last_key_pressed_);
                ImGui::SameLine();
                c8_key_2.Draw(last_key_pressed_);
                ImGui::SameLine();
                c8_key_3.Draw(last_key_pressed_);

                c8_key_4.Draw(last_key_pressed_);
                ImGui::SameLine();
                c8_key_5.Draw(last_key_pressed_);
                ImGui::SameLine();
                c8_key_6.Draw(last_key_pressed_);
                ImGui::SameLine();
                c8_key_7.Draw(last_key_pressed_);

                c8_key_8.Draw(last_key_pressed_);
                ImGui::SameLine();
                c8_key_9.Draw(last_key_pressed_);
                ImGui::SameLine();
                c8_key_a.Draw(last_key_pressed_);
                ImGui::SameLine();
                c8_key_b.Draw(last_key_pressed_);

                c8_key_c.Draw(last_key_pressed_);
                ImGui::SameLine();
                c8_key_d.Draw(last_key_pressed_);
                ImGui::SameLine();
                c8_key_e.Draw(last_key_pressed_);
                ImGui::SameLine();
                c8_key_f.Draw(last_key_pressed_);
            }
            if (ImGui::CollapsingHeader("N64")) {
                static std::string ipl_loc = "IPL location: " + settings_.at("N64.ipl_loc");
                static std::vector<std::string> ipl_ext_ = { ".bin" };
                if (ipl_changed_) {
                    ipl_loc = "IPL location: " + settings_.at("N64.ipl_loc");
                }
                ImGui::TextUnformatted(ipl_loc.c_str());
                if (ImGui::Button("Browse IPL...", ImVec2(200, 0))) {
                    if (!window_file_browser_open_) {
                        window_file_browser_open_ = true;
                        file_browser_callback_ = &Display::load_ipl;
                        open_file_browser("Browse IPL...", ipl_ext_);
                    }
                }
            }
            ImGui::End();
        }
    }
    void Display::draw_tools() {
        for (const auto& app : emulator_tools_) {
            app->Draw();
        }
        for (const auto& app : generic_tools_) {
            app->Draw();
        }
    }
    void Display::draw_fps_counter(bool* draw){
        if (*draw) {
            static int corner = 0;
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
            if (corner != -1)
            {
                const float PAD = 10.0f;
                const ImGuiViewport* viewport = ImGui::GetMainViewport();
                ImVec2 work_pos = viewport->WorkPos;
                ImVec2 work_size = viewport->WorkSize;
                ImVec2 window_pos, window_pos_pivot;
                window_pos.x = (corner & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD);
                window_pos.y = (corner & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + PAD);
                window_pos_pivot.x = (corner & 1) ? 1.0f : 0.0f;
                window_pos_pivot.y = (corner & 2) ? 1.0f : 0.0f;
                ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
                window_flags |= ImGuiWindowFlags_NoMove;
            }
            ImGui::SetNextWindowBgAlpha(0.35f);
            if (ImGui::Begin("Overlay", draw, window_flags)) {
                ImGui::Text("FPS: %.2f", ImGui::GetIO().Framerate);
                if (ImGui::BeginPopupContextWindow())
                {
                    if (ImGui::MenuItem("Custom", NULL, corner == -1)) corner = -1;
                    if (ImGui::MenuItem("Top-left", NULL, corner == 0)) corner = 0;
                    if (ImGui::MenuItem("Top-right", NULL, corner == 1)) corner = 1;
                    if (ImGui::MenuItem("Bottom-left", NULL, corner == 2)) corner = 2;
                    if (ImGui::MenuItem("Bottom-right", NULL, corner == 3)) corner = 3;
                    if (ImGui::MenuItem("Close")) *draw = false;
                    ImGui::EndPopup();
                }
            }
            ImGui::End();
        }
    }
    void Display::draw_disassembly(bool* draw) {
        if (*draw) {
            if (ImGui::Begin("Disassembly", draw)) {
                constexpr size_t buf_size = 8 + 1; // null terminated
                static char buf[buf_size] = "";
                static std::string instr_string;
                ImGui::InputText("Instruction##disassembly", buf, buf_size, 
                        ImGuiInputTextFlags_CharsHexadecimal |
                        ImGuiInputTextFlags_CharsUppercase);
                if (ImGui::Button("Disassemble")) {
                    // future will have selectable emulator with different
                    // instruction types
                    uint32_t instr;   
                    std::stringstream ss;
                    ss << std::hex << buf;
                    ss >> instr;
                    instr_string = TKPEmu::GeneralDisassembler::GetDisassembledString(EmuType::N64, instr);
                }
                ImGui::TextUnformatted(instr_string.c_str());
            }
            ImGui::End();
        }
    }
    void Display::draw_about(bool* draw) {
        if (*draw) {
            ImGui::SetNextWindowSize(ImVec2(350, 200));
            if (ImGui::Begin("About", draw, ImGuiWindowFlags_NoResize)) {
                
            }
            ImGui::End();
        }
    }
    void Display::draw_file_browser(bool* draw) {
        if (*draw) {
            file_browser_.Display();
            if (file_browser_.HasSelected()) {
                auto path = file_browser_.GetSelected();
                TKP_MAY_THROW((this->*file_browser_callback_)(path));
                if (recent_paths_.size() == RecentRomsMaxSize) {
                    recent_paths_.pop_back();
                }
                recent_paths_.push_front(path);
                save_recent_files();
                file_browser_.ClearSelected();
                window_file_browser_open_ = false;
            }
            if (file_browser_.ShouldClose()) {
                window_file_browser_open_ = false;
            }
        }
    }
    void Display::draw_game_background(bool* draw) {
        ImGui::GetBackgroundDrawList()->AddRectFilledMultiColor(ImVec2(0, 0), ImVec2(window_settings_.window_width, window_settings_.window_height), 0xC0FFFFC0, 0xC0C0FFFF,  0xC0FFC0FF, 0xC0FFC0C0);
        if (*draw) {
            glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_);
            glBindTexture(GL_TEXTURE_2D, emulator_->EmulatorImage.texture);
            {
                std::lock_guard<std::mutex> lg(emulator_->DrawMutex);
                void* data = emulator_->GetScreenData();
                bool& should_resize = emulator_->IsResized();
                if (data && should_resize) {
                    glTexImage2D(
                        GL_TEXTURE_2D,
                        0,
                        GL_RGB,
                        emulator_->EmulatorImage.width,
                        emulator_->EmulatorImage.height,
                        0,
                        emulator_->EmulatorImage.format,
                        emulator_->EmulatorImage.type,
                        data
                    );
                    should_resize = false;
                }
                bool& should_draw = emulator_->IsReadyToDraw();
                if (data && should_draw) {
                    should_draw = false;
                    glTexSubImage2D(
                        GL_TEXTURE_2D,
                        0,
                        0,
                        0,
                        emulator_->EmulatorImage.width,
                        emulator_->EmulatorImage.height,
                        emulator_->EmulatorImage.format,
                        emulator_->EmulatorImage.type,
                        data
                    );
                }
            }
            glBindTexture(GL_TEXTURE_2D, 0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            ImGui::GetBackgroundDrawList()->AddImage(reinterpret_cast<void*>(static_cast<intptr_t>(emulator_->EmulatorImage.texture)), emulator_->EmulatorImage.topleft, emulator_->EmulatorImage.botright);
            if (emulator_->Paused) {
                ImGui::GetBackgroundDrawList()->AddText(nullptr, 40.0f, emulator_->EmulatorImage.topleft, 0xFF0000FF, "Paused");
            }
        } else {
            // Any logo goes here
        }
    }
    void Display::draw_menu_bar(bool* draw) {
        if (*draw) {
            if (ImGui::BeginMainMenuBar())
            {
                if (ImGui::BeginMenu("File")) {
                    draw_menu_bar_file();
                }
                if (ImGui::BeginMenu("Emulation")) {
                    IMApplication::DrawMenuEmulation(emulator_.get());
                }
                if (ImGui::BeginMenu("View")) {
                    draw_menu_bar_view();
                }
                if (ImGui::BeginMenu("Tools")) {
                    draw_menu_bar_tools();
                }
                if (ImGui::BeginMenu("Help")) {
                    draw_menu_bar_help();
                }
                ImGui::EndMainMenuBar();
            }
        }
    }
    void Display::draw_menu_bar_file() {
        if (ImGui::MenuItem("Open ROM", "Ctrl+O")) {
            open_file_browser_rom();
        }
        if (ImGui::BeginMenu("Open Recent")) {
            draw_menu_bar_file_recent();
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Save State", "Ctrl+S", false, is_rom_loaded())) {
            last_shortcut_ = TKPShortcut::CTRL_S;
        }
        if (ImGui::MenuItem("Load State", "Ctrl+L", false, is_rom_loaded())) {
            last_shortcut_ = TKPShortcut::CTRL_L;
        }
        ImGui::Separator();
        // TODO: implement save and load state, disable these buttons if no rom loaded
        if (ImGui::MenuItem("Save State File", nullptr, false, is_rom_loaded())) {}
        if (ImGui::MenuItem("Load State File", nullptr, false, is_rom_loaded())) {}
        ImGui::Separator();
        if (ImGui::MenuItem("Screenshot", NULL, false, is_rom_loaded())) {
            emulator_->Screenshot("Screenshot.bmp");
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Settings", NULL, window_settings_open_, true)) { 
            window_settings_open_ ^= true; 
        }
        ImGui::EndMenu();
    }
    void Display::draw_menu_bar_file_recent() {
        if (recent_paths_.empty()) {
            ImGui::MenuItem("No recent roms", NULL, false, false);
        } else {
            for (size_t i = 0; i < recent_paths_.size(); ++i) {
                std::string menu_name = recent_paths_[i].filename().string();
                if (ImGui::MenuItem(menu_name.c_str())) {
                    auto temp = recent_paths_[i];
                    recent_paths_.erase(recent_paths_.begin() + i);
                    if (std::filesystem::exists(temp)) {
                        recent_paths_.push_front(temp);
                        TKP_MAY_THROW(load_rom(temp));
                    } else {
                        messagebox_body_ = "File not found: ";
                        messagebox_body_ += temp.c_str();
                        window_messagebox_open_ = true;
                    }
                    save_recent_files();
                }
            }
        }
        ImGui::EndMenu();
    }
    void Display::save_recent_files() {
        for (size_t i = 0; i < RecentRomsMaxSize; i++) {
            std::string new_val = "";
            if (i < recent_paths_.size()) {
                new_val = recent_paths_[i];
            }
            settings_.at("General.recent" + std::to_string(i)) = new_val;
        }
    }
    void Display::draw_menu_bar_tools() {
        if (rom_loaded_) {
            for (const auto& app : generic_tools_) {
                bool* drawing_ = app->IsDrawing();
                if (ImGui::MenuItem(app->GetName(), NULL, *drawing_)) {
                    *drawing_ ^= true;
                }
            }
            ImGui::Separator();
            for (const auto& app : emulator_tools_) {
                app->DrawMenuItem();
            }
        } else {
            ImGui::TextDisabled("Emulator not loaded");
        }
        ImGui::EndMenu();
    }
    void Display::draw_menu_bar_view() {
        if (ImGui::MenuItem("FPS Counter", nullptr, window_fpscounter_open_, true)) {
              window_fpscounter_open_ ^= true; 
        }
        if (ImGui::MenuItem("Disassembly", nullptr, window_disassembly_open_, true)) {
              window_disassembly_open_ ^= true; 
        }
        ImGui::EndMenu();
    }
    void Display::draw_menu_bar_help() {
        if (ImGui::MenuItem("About", nullptr, window_about_open_, true)) {
            window_about_open_ ^= true;
        }
        ImGui::EndMenu();
    }
    void Display::handle_shortcuts() {
        switch(last_shortcut_) {
            case TKPShortcut::CTRL_O: {
                open_file_browser_rom();
                last_shortcut_ = TKPShortcut::NONE;
                break;
            }
            case TKPShortcut::CTRL_L: {
                if (emulator_)
                    TKP_MAY_THROW(emulator_->LoadState(settings_manager_.GetSavePath() + "/save_states/" + emulator_->RomHash + ".state"));
                last_shortcut_ = TKPShortcut::NONE;
                break;
            }
            case TKPShortcut::CTRL_S: {
                if (emulator_)
                    TKP_MAY_THROW(emulator_->SaveState(settings_manager_.GetSavePath() + "/save_states/" + emulator_->RomHash + ".state"));
                last_shortcut_ = TKPShortcut::NONE;
                break;
            }
            case TKPShortcut::CTRL_R: {
                if (emulator_ != nullptr) {
                    emulator_->CloseAndWait();
                    EmuStartOptions options = debug_mode_ ? EmuStartOptions::Debug : EmuStartOptions::Normal;
                    emulator_->Start(options);
                }
                last_shortcut_ = TKPShortcut::NONE;
                break;
            }
            case TKPShortcut::CTRL_P: {
                if (emulator_ != nullptr) {
                    emulator_->Paused.store(!emulator_->Paused);
                    emulator_->Step.store(true);
                    emulator_->Step.notify_all();
                }
                last_shortcut_ = TKPShortcut::NONE;
                break;
            }
            default: {
                break;
            }
        }
        // Shortcut was not handled by display, pass to applications
        if (last_shortcut_ != TKPShortcut::NONE) {
            for (const auto& app : emulator_tools_) {
                app->HandleShortcut(last_shortcut_);
                if (last_shortcut_ == TKPShortcut::NONE) {
                    // Shortcut was handled by this app
                    return;
                }
            }
        }
    }
    void Display::draw_messagebox(bool* draw) {
        if (*draw) {
            if (!ImGui::IsPopupOpen("Message")) {
                ImGui::OpenPopup("Message");
            }
            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            if (ImGui::BeginPopupModal("Message", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::TextUnformatted(messagebox_body_.c_str());
                if (ImGui::Button("Ok", ImVec2(120, 0))) {
                    window_messagebox_open_ = false;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }
    }
    void Display::image_scale(ImVec2& topleft, ImVec2& bottomright, float wi, float hi) {
        float ws = window_settings_.window_width;
        float hs = window_settings_.window_height;
        float ri = wi / (hi + MenuBarHeight);
        float rs = ws / hs;
        float new_w;
        float new_h;
        if (rs > ri) {
            new_h = hs - MenuBarHeight;
            new_w = wi * (new_h / hi);
        } else {
            new_w = ws;
            new_h = hi * (ws / wi);
        }
        new_h += MenuBarHeight;
        topleft.y = (hs - new_h) / 2 + MenuBarHeight;
        topleft.x = (ws - new_w) / 2;
        bottomright.x = new_w + topleft.x;
        bottomright.y = new_h + topleft.y - MenuBarHeight;
    }
    void Display::load_rom(std::filesystem::path path) {
        if (path.has_parent_path()) {
            settings_.at("General.last_dir") = path.parent_path();
        } else {
            throw ErrorFactory::generate_exception(__func__, __LINE__, "Rom has no parent path");
        }
        auto old_emulator_type_ = emulator_type_;
        // First check if the path is valid
        TKP_MAY_THROW_ACTION(
            emulator_type_ = EmulatorFactory::GetEmulatorType(path),
            emulator_type_ = old_emulator_type_
        );
        std::shared_ptr<Emulator> emu_copy_ = emulator_;
        if (emulator_) {
            if (!emulator_->Loaded) {
                // Tried to open a new emulator before the old one finished loading.
                // Impossibly rare, but would avoid a program hang
                return;
            }
            emulator_->CloseAndWait();
            emulator_.reset();
        }
        std::any emu_specific_args = get_emu_specific_args(emulator_type_);
        emulator_ = TKPEmu::EmulatorFactory::Create(emulator_type_, emu_specific_args);
        emulator_tools_.clear();
        generic_tools_.clear();
        if (emu_copy_) {
            TKP_MAY_THROW_ACTION(
                // Check that only instance of emulator shared_ptr is this copy
                // to make sure it's properly destroyed
                if (emu_copy_.use_count() != 1) {
                    throw ErrorFactory::generate_exception(__func__, __LINE__, "Leaked memory");
                } else {
                    emu_copy_.reset();
                },
                return
            );
        }
        TKPEmu::EmulatorFactory::LoadEmulatorTools(emulator_tools_, emulator_, emulator_type_);
        // TKPEmu::EmulatorFactory::LoadGenericTools(generic_tools_, emulator_, emulator_type_);
        setup_emulator_specific();
        rom_loaded_ = true;
        emulator_->SkipBoot = skip_boot_;
        emulator_->FastMode = fast_mode_;
        if (!emulator_->LoadFromFile(path.string())) {
            return;
        }
        EmuStartOptions options = debug_mode_ ? EmuStartOptions::Debug : EmuStartOptions::Normal;
        glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, emulator_->EmulatorImage.texture, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        image_scale(emulator_->EmulatorImage.topleft, emulator_->EmulatorImage.botright, emulator_->EmulatorImage.width, emulator_->EmulatorImage.height);
        emulator_->Start(options);
        settings_manager_.Save();
    }
    void Display::load_ipl(std::filesystem::path path) {
        settings_.at("N64.ipl_loc") = path.string();
        n64_ipl_loc_ = path;
        ipl_changed_ = true;
    }
    // TODO: get rid of this ugly function and the std::any constructors
    std::any Display::get_emu_specific_args(EmuType type) {
        std::any ret;
        switch(type) {
            case EmuType::Gameboy: {
                ret = std::make_pair(gb_keys_direction_, gb_keys_action_);
                break;
            }
            case EmuType::N64: {
                ret = N64Args {
                    .IPLPath = n64_ipl_loc_.string(),
                };
                break;
            }
            case EmuType::Chip8: {
                ret = chip8_keys_;
                break;
            }
            default: {
                throw ErrorFactory::generate_exception(__func__, __LINE__, "Unhandled emulator type");
            }
        }
        return ret;
    }
    void Display::limit_fps() {
        a = std::chrono::system_clock::now();
        std::chrono::duration<double, std::milli> work_time = a - b;
        if (work_time.count() < sleep_time_) {
            std::chrono::duration<double, std::milli> delta_ms(sleep_time_ - work_time.count());
            auto delta_ms_duration = std::chrono::duration_cast<std::chrono::milliseconds>(delta_ms);
            std::this_thread::sleep_for(std::chrono::milliseconds(delta_ms_duration.count()));
        }
        b = std::chrono::system_clock::now();
    }
    void Display::init_settings_values() {
        limit_fps_ = std::stoi(settings_.at("Video.limit_fps"));
        max_fps_ = std::stoi(settings_.at("Video.max_fps"));
        debug_mode_ = std::stoi(settings_.at("General.debug_mode"));
        skip_boot_ = std::stoi(settings_.at("General.skip_boot"));
        fast_mode_ = std::stoi(settings_.at("General.fast_mode"));
        sleep_time_ = 1000.0f / max_fps_;
        KeySelector::Initialize(&settings_);
        init_gameboy_values();
        init_n64_values();
    }
    void Display::init_gameboy_values(){
        int color0 = std::stoi(settings_.at("Gameboy.color0"), 0, 16);
        int color1 = std::stoi(settings_.at("Gameboy.color1"), 0, 16);
        int color2 = std::stoi(settings_.at("Gameboy.color2"), 0, 16);
        int color3 = std::stoi(settings_.at("Gameboy.color3"), 0, 16);
        gb_palettes_[0][0] = ((color0 >> 16) & 0xFF) / 255.0f;
        gb_palettes_[0][1] = ((color0 >> 8) & 0xFF) / 255.0f;
        gb_palettes_[0][2] = ((color0) & 0xFF) / 255.0f;
        gb_palettes_[1][0] = ((color1 >> 16) & 0xFF) / 255.0f;
        gb_palettes_[1][1] = ((color1 >> 8) & 0xFF) / 255.0f;
        gb_palettes_[1][2] = ((color1) & 0xFF) / 255.0f;
        gb_palettes_[2][0] = ((color2 >> 16) & 0xFF) / 255.0f;
        gb_palettes_[2][1] = ((color2 >> 8) & 0xFF) / 255.0f;
        gb_palettes_[2][2] = ((color2) & 0xFF) / 255.0f;
        gb_palettes_[3][0] = ((color3 >> 16) & 0xFF) / 255.0f;
        gb_palettes_[3][1] = ((color3 >> 8) & 0xFF) / 255.0f;
        gb_palettes_[3][2] = ((color3) & 0xFF) / 255.0f;
    }
    void Display::init_n64_values() {
        auto path = settings_.at("N64.ipl_loc");
        if (std::filesystem::exists(path))
            n64_ipl_loc_ = path;
    }
    bool Display::is_rom_loaded() {
        return rom_loaded_;
    }
    bool Display::is_rom_loaded_and_debugmode() {
        return rom_loaded_ && debug_mode_;
    }
    void Display::setup_emulator_specific() {
        // Emulator specific options that are display related
        switch (emulator_type_) {
            case EmuType::Gameboy: {
                setup_gameboy_palette();
                break;
            }
            default: {
                break;
            }
        }
    }
    void Display::setup_gameboy_palette() {
        if (emulator_ != nullptr) {
            using Gameboy = TKPEmu::Gameboy::Gameboy;
            Gameboy* temp = dynamic_cast<Gameboy*>(emulator_.get());
            auto& pal = temp->GetPalette();
            pal = gb_palettes_;
        }
    }
    void Display::load_loop(){
        //gladLoadGL();
        glViewport(0, 0, window_settings_.window_width, window_settings_.window_height);
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = NULL;
        load_theme();
        ImGui_ImplOpenGL3_Init(GLSLVersion.c_str());
        ImGui_ImplSDL2_InitForOpenGL(window_ptr_.get(), gl_context_ptr_.get());
        ImVec4 background = ImVec4(35 / 255.0f, 35 / 255.0f, 35 / 255.0f, 1.00f);
        glClearColor(background.x, background.y, background.z, background.w);
        SDL_GL_SetSwapInterval(0);
        init_settings_values();
        ImGui::LoadIniSettingsFromDisk(ImGuiSettingsFile.c_str());
        ImGui::SetColorEditOptions(ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_PickerHueWheel);
        // TODO: design better icon
        // SDL_Surface* icon = SDL_LoadBMP(std::string(std::filesystem::current_path().string() + "/Resources/Images/icon.bmp").c_str());
        // SDL_SetWindowIcon(window_ptr_.get(), icon);
        // SDL_FreeSurface(icon);
        file_browser_.SetWindowSize(400, 400);
    }
    void Display::load_theme() {
        ImGuiStyle& style = ImGui::GetStyle();
        style.Colors[ImGuiCol_Text] = ImVec4(0.31f, 0.25f, 0.24f, 1.00f);
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 0.90f);
        style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.74f, 0.74f, 0.94f, 1.00f);
        style.Colors[ImGuiCol_ChildBg] = ImVec4(0.68f, 0.68f, 0.68f, 0.00f);
        style.Colors[ImGuiCol_Border] = ImVec4(0.50f, 0.50f, 0.50f, 0.60f);
        style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        style.Colors[ImGuiCol_FrameBg] = ImVec4(0.62f, 0.70f, 0.72f, 0.56f);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.95f, 0.33f, 0.14f, 0.47f);
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.97f, 0.31f, 0.13f, 0.81f);
        style.Colors[ImGuiCol_TitleBg] = ImVec4(0.42f, 0.75f, 1.00f, 1.00f);
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.42f, 0.75f, 1.00f, 1.00f);
        style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.40f, 0.65f, 0.80f, 1.00f);
        style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.40f, 0.62f, 0.80f, 0.15f);
        style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.39f, 0.64f, 0.80f, 0.30f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.28f, 0.67f, 0.80f, 0.59f);
        style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.25f, 0.48f, 0.53f, 0.67f);
        style.Colors[ImGuiCol_PopupBg] = ImVec4(0.89f, 0.98f, 1.00f, 0.99f);
        style.Colors[ImGuiCol_CheckMark] = ImVec4(0.48f, 0.47f, 0.47f, 0.71f);
        style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.31f, 0.47f, 0.99f, 1.00f);
        style.Colors[ImGuiCol_Button] = ImVec4(1.00f, 0.79f, 0.18f, 0.78f);
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.42f, 0.82f, 1.00f, 0.81f);
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.72f, 1.00f, 1.00f, 0.86f);
        style.Colors[ImGuiCol_Header] = ImVec4(0.65f, 0.78f, 0.84f, 0.80f);
        style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.75f, 0.88f, 0.94f, 0.80f);
        style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.55f, 0.68f, 0.74f, 0.80f);
        style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.60f, 0.60f, 0.80f, 0.30f);
        style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
        style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
        style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(1.00f, 0.99f, 0.54f, 0.43f);
        style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
        style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.97f, 0.97f, 0.97f, 1.00f);
        style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.79f, 0.79f, 0.99f, 1.00f);
        style.Alpha = 1.0f;
        style.FrameRounding = 4;
        style.IndentSpacing = 12.0f;
    }
    void Display::open_file_browser(std::string title, const std::vector<std::string>& extensions) {
        file_browser_.SetTitle(title);
        file_browser_.SetTypeFilters(extensions);
        TKP_MAY_THROW(
            std::filesystem::path path = settings_manager_.GetSavePath();
            if (!(settings_.at("General.last_dir").empty())) {
                path = settings_.at("General.last_dir");
            }
            file_browser_.SetPwd(path);
            file_browser_.Open();
        );
    }
    void Display::open_file_browser_rom() {
        if (!window_file_browser_open_) {
            window_file_browser_open_ = true;
            file_browser_callback_ = &Display::load_rom;
            open_file_browser("Browse ROM...", EmulatorFactory::GetSupportedExtensions());
        }
    }
    void Display::main_loop() {
        load_loop();
        bool loop = true;
        if (!WS_path_.empty()) {
            bool temp_bool = false;
            draw_settings(&temp_bool); // TODO: ugly, get rid of this. (initializes settings)
            fast_mode_ = false;
            load_rom(WS_path_);
        }
        while (loop)
        {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            SDL_Event event;
            last_key_pressed_ = 0;
            while (SDL_PollEvent(&event))
            {
                ImGui_ImplSDL2_ProcessEvent(&event);
                switch (event.type)
                {
                    case SDL_QUIT:
                        loop = false;
                        break;

                    case SDL_WINDOWEVENT:
                        switch (event.window.event)
                        {
                            case SDL_WINDOWEVENT_RESIZED:
                            window_settings_.window_width = event.window.data1;
                            window_settings_.window_height = event.window.data2;
                            if (emulator_ != nullptr)
                                image_scale(emulator_->EmulatorImage.topleft, emulator_->EmulatorImage.botright, emulator_->EmulatorImage.width, emulator_->EmulatorImage.height);
                            glViewport(0, 0, window_settings_.window_width, window_settings_.window_height);
                            break;
                        }
                        break;

                    case SDL_KEYDOWN: {
                        auto key = event.key.keysym.sym;
                        // Handle the shortcuts
                        const auto k = SDL_GetKeyboardState(NULL);
                        if ((k[SDL_SCANCODE_RCTRL] || k[SDL_SCANCODE_LCTRL]) && k[SDL_SCANCODE_P]) {
                            last_shortcut_ = TKPShortcut::CTRL_P;
                            break;
                        }
                        if ((k[SDL_SCANCODE_RCTRL] || k[SDL_SCANCODE_LCTRL]) && k[SDL_SCANCODE_R]) {
                            last_shortcut_ = TKPShortcut::CTRL_R;
                            break;
                        }
                        if ((k[SDL_SCANCODE_RCTRL] || k[SDL_SCANCODE_LCTRL]) && k[SDL_SCANCODE_F]) {
                            last_shortcut_ = TKPShortcut::CTRL_F;
                            break;
                        }
                        if ((k[SDL_SCANCODE_RCTRL] || k[SDL_SCANCODE_LCTRL]) && k[SDL_SCANCODE_O]) {
                            last_shortcut_ = TKPShortcut::CTRL_O;
                            break;
                        }
                        if ((k[SDL_SCANCODE_RCTRL] || k[SDL_SCANCODE_LCTRL]) && k[SDL_SCANCODE_S]) {
                            last_shortcut_ = TKPShortcut::CTRL_S;
                            break;
                        }
                        if ((k[SDL_SCANCODE_RCTRL] || k[SDL_SCANCODE_LCTRL]) && k[SDL_SCANCODE_L]) {
                            last_shortcut_ = TKPShortcut::CTRL_L;
                            break;
                        }
                        if (k[SDL_SCANCODE_F7]) {
                            last_shortcut_ = TKPShortcut::F7;
                            break;
                        }
                        last_key_pressed_ = key;
                        if (emulator_ != nullptr) {
                            emulator_->HandleKeyDown(key);
                        }
                        break;
                    }
                    case SDL_KEYUP:{
                        if (emulator_ != nullptr) {
                            auto key = event.key.keysym.sym;
                            emulator_->HandleKeyUp(key);
                        }
                        break;
                    }
                }
            }

            if (limit_fps_) {
                limit_fps();
            }
            if (emulator_) {
                if (emulator_->HasException) [[unlikely]]{
					// std::lock_guard<std::mutex> lg(emulator_->DebugUpdateMutex);
                    TKP_MAY_THROW(std::rethrow_exception(emulator_->CurrentException));
                    emulator_->HasException = false;
                }
            }
            handle_shortcuts();
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplSDL2_NewFrame(window_ptr_.get());
            ImGui::NewFrame();
            draw_game_background(&rom_loaded_);
            draw_menu_bar(&menu_bar_open_);
            draw_fps_counter(&window_fpscounter_open_);
            draw_disassembly(&window_disassembly_open_);
            draw_about(&window_about_open_);
            TKP_MAY_THROW(draw_tools());
            draw_settings(&window_settings_open_);
            draw_file_browser(&window_file_browser_open_);
            draw_messagebox(&window_messagebox_open_);
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            SDL_GL_SwapWindow(window_ptr_.get());
        }
        ImGui::SaveIniSettingsToDisk(ImGuiSettingsFile.c_str());
        // Locking this mutex ensures that the other thread gets
        // enough time to exit before we close the main application.
        if (emulator_) {
            emulator_->CloseAndWait();
        }
    }
    void Display::WS_SetActionFlag(int* action_ptr) {
        action_ptr_ = action_ptr;
    }
    void Display::WS_LoadRom(std::string path) {
        std::cout << "Loading rom for web server:" << path << std::endl;
        WS_path_ = path;
    }
    void Display::throw_error(const std::runtime_error& ex) {
        window_messagebox_open_ = true;
        messagebox_body_ = ex.what();
        if (emulator_)
            emulator_->CloseAndWait();
    }
}