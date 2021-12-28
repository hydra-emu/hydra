#define NOMINMAX
#define STB_IMAGE_IMPLEMENTATION
#include <memory>
#include <thread>
#include <iostream>
#include <algorithm>
#include "../include/display.h"
#include "../include/disassembly_instr.h"
#include "../GameboyTKP/gb_disassembler.h"
#include "../GameboyTKP/gb_tracelogger.h"

namespace TKPEmu::Graphics {
	Display::DisplayInitializer::DisplayInitializer() {
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            std::cout << SDL_GetError() << std::endl;
            throw("SDL could not be initialized");
        }
        else {
            std::cout << "SDL initialized successfully" << std::endl;
        }
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
        settings_manager_(settings_, "tkp_user.ini"),
        window_ptr_(nullptr, SDL_DestroyWindow),
        gl_context_ptr_(nullptr, SDL_GL_DeleteContext)
    {
        std::ios_base::sync_with_stdio(false);
        ImGuiSettingsFile = settings_manager_.SaveDataDir + ImGuiSettingsFile;
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
        if (!gladLoadGLLoader(static_cast<GLADloadproc>(SDL_GL_GetProcAddress))) {
            throw("Error: coudl not initialize glad");
        }
        else {
            std::cout << "Glad initialized successfully" << std::endl;
        }
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
        static KeySelector key_right("Direction Right:", "Gameboy.key_right", gb_keys_directional_[0]);
        static KeySelector  key_left("Direction Left: ", "Gameboy.key_left", gb_keys_directional_[1]);
        static KeySelector    key_up("Direction Up:   ", "Gameboy.key_up", gb_keys_directional_[2]);
        static KeySelector  key_down("Direction Down: ", "Gameboy.key_down", gb_keys_directional_[3]);
        static KeySelector     key_a("Action A:       ", "Gameboy.key_a", gb_keys_action_[0]);
        static KeySelector     key_b("Action B:       ", "Gameboy.key_b", gb_keys_action_[1]);
        static KeySelector   key_sel("Action Select:  ", "Gameboy.key_select", gb_keys_action_[2]);
        static KeySelector key_start("Action Start:   ", "Gameboy.key_start", gb_keys_action_[3]);
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
                    // TODO: use formatted strings (%02x or w.e it was)
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
                    settings_.at("Gameboy.color0") = "d0d058";
                    settings_.at("Gameboy.color1") = "a0a840";
                    settings_.at("Gameboy.color2") = "708028";
                    settings_.at("Gameboy.color3") = "405010";
                    gb_palettes_[0][0] = 0xd0 / 255.0f;
                    gb_palettes_[0][1] = 0xd0 / 255.0f;
                    gb_palettes_[0][2] = 0x58 / 255.0f;
                    gb_palettes_[1][0] = 0xa0 / 255.0f;
                    gb_palettes_[1][1] = 0xa8 / 255.0f;
                    gb_palettes_[1][2] = 0x40 / 255.0f;
                    gb_palettes_[2][0] = 0x70 / 255.0f;
                    gb_palettes_[2][1] = 0x80 / 255.0f;
                    gb_palettes_[2][2] = 0x28 / 255.0f;
                    gb_palettes_[3][0] = 0x40 / 255.0f;
                    gb_palettes_[3][1] = 0x50 / 255.0f;
                    gb_palettes_[3][2] = 0x10 / 255.0f;
                    setup_emulator_specific();
                }
                ImGui::Spacing();
                ImGui::TextUnformatted("Controls:");
                ImGui::Separator();
                key_right.Draw(last_key_pressed_);
                key_left.Draw(last_key_pressed_);
                key_up.Draw(last_key_pressed_);
                key_down.Draw(last_key_pressed_);
                key_a.Draw(last_key_pressed_);
                key_b.Draw(last_key_pressed_);
                key_sel.Draw(last_key_pressed_);
                key_start.Draw(last_key_pressed_);
            }
            ImGui::End();
        }
    }
    void Display::draw_tools() {
        for (const auto& app : emulator_tools_) {
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
                    if (draw && ImGui::MenuItem("Close")) *draw = false;
                    ImGui::EndPopup();
                }
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
                load_rom(std::move(file_browser_.GetSelected()));
                file_browser_.ClearSelected();
                window_file_browser_open_ = false;
            }
            if (file_browser_.ShouldClose()) {
                window_file_browser_open_ = false;
            }
        }
    }
    void Display::draw_game_background(bool* draw) {
        ImGui::GetBackgroundDrawList()->AddRectFilledMultiColor(ImVec2(0, 0), ImVec2(window_settings_.window_width, window_settings_.window_height), 0xFF0000FF, 0xFFFFFF00, 0xFF00FFFF, 0xFF00FF00);
        if (*draw) {
            std::lock_guard<std::mutex> lg(emulator_->DrawMutex);
            glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_);
            glBindTexture(GL_TEXTURE_2D, emulator_->EmulatorImage.texture);
            glTexSubImage2D(
                GL_TEXTURE_2D,
                0,
                0,
                0,
                emulator_->EmulatorImage.width,
                emulator_->EmulatorImage.height,
                GL_RGBA,
                GL_FLOAT,
                emulator_->GetScreenData()
            );
            glBindTexture(GL_TEXTURE_2D, 0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            ImGui::GetBackgroundDrawList()->AddImage((void*)(intptr_t)(emulator_->EmulatorImage.texture), emulator_->EmulatorImage.topleft, emulator_->EmulatorImage.botright);
            if (emulator_->Paused) {
                ImGui::GetBackgroundDrawList()->AddText(nullptr, 40.0f, emulator_->EmulatorImage.topleft, 0xFF000000, "Paused");
            }
        } else {
            // TODO: allow for undocked window, disable above line when that happens
            //ImGui::Image((void*)(intptr_t)_image_.texture, ImVec2(512,512));
            ImGui::GetBackgroundDrawList()->AddCircleFilled(ImVec2(window_settings_.window_width / 2 - 5, (MenuBarHeight + window_settings_.window_height) / 2 - 5), 64, 0xFF000000);
            ImGui::GetBackgroundDrawList()->AddCircleFilled(ImVec2(window_settings_.window_width / 2, (MenuBarHeight + window_settings_.window_height) / 2), 64, 0xFF00FFFF);
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
            if (!window_file_browser_open_) {
                window_file_browser_open_ = true;
                open_file_browser();
            }
        }
        if (ImGui::BeginMenu("Open Recent")) {
            draw_menu_bar_file_recent();
        }
        ImGui::Separator();
        // TODO: implement save and load state, disable these buttons if no rom loaded
        if (ImGui::MenuItem("Save State", "Ctrl+S", false, is_rom_loaded())) {}
        if (ImGui::MenuItem("Load State", "Ctrl+L", false, is_rom_loaded())) {}
        ImGui::Separator();
        if (ImGui::MenuItem("Screenshot", NULL, false, is_rom_loaded())) {
            emulator_->Screenshot("Screenshot");
            std::cout << emulator_->GetScreenshotHash() << std::endl;
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Settings", NULL, window_settings_open_, true)) { 
            window_settings_open_ ^= true; 
        }
        ImGui::EndMenu();
    }
    void Display::draw_menu_bar_file_recent() {
        // TODO: implement open recent (menu->file)
        ImGui::MenuItem("fish_hat.c");
        ImGui::MenuItem("fish_hat.inl");
        ImGui::MenuItem("fish_hat.h");
        ImGui::EndMenu();
    }
    void Display::draw_menu_bar_tools() {
        if (rom_loaded_) {
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
                if (!window_file_browser_open_) {
                    window_file_browser_open_ = true;
                    open_file_browser();
                }
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
    void Display::image_scale(ImVec2& topleft, ImVec2& bottomright, float wi, float hi) {
        float ws = window_settings_.window_width; float hs = window_settings_.window_height;
        float ri = wi / hi;
        float rs = ws / hs;
        float new_w;
        float new_h;
        if (rs > ri) {
            new_h = hs - MenuBarHeight;
            new_w = wi * (new_h / hi);
        }
        else {
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
            std::cerr << "Error loading rom: Path has no parent path" << std::endl;
            exit(1);
        }
        if (emulator_ != nullptr) {
            if (!emulator_->Loaded) {
                // Tried to open a new emulator before the old one finished loading.
                // Impossibly rare, but would avoid a program hang
                return;
            }
            emulator_->CloseAndWait();
            emulator_.reset();
        }
        auto old_emulator_type_ = emulator_type_;
        emulator_type_ = EmulatorFactory::GetEmulatorType(path);
        emulator_ = TKPEmu::EmulatorFactory::Create(emulator_type_, gb_keys_directional_, gb_keys_action_);
        if (emulator_type_ != old_emulator_type_) {
            emulator_tools_.clear();
            TKPEmu::EmulatorFactory::LoadEmulatorTools(emulator_tools_, emulator_.get(), emulator_type_);
        } else {
            for (auto& t : emulator_tools_) {
                t->Reset();
            }
        }
        setup_emulator_specific();
        rom_loaded_ = true;
        rom_paused_ = debug_mode_;
        emulator_->SkipBoot = skip_boot_;
        emulator_->FastMode = fast_mode_;
        emulator_->LoadFromFile(path.string());
        EmuStartOptions options = debug_mode_ ? EmuStartOptions::Debug : EmuStartOptions::Normal;
        glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, emulator_->EmulatorImage.texture, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        image_scale(emulator_->EmulatorImage.topleft, emulator_->EmulatorImage.botright, emulator_->EmulatorImage.width, emulator_->EmulatorImage.height);
        emulator_->Start(options);
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
            Gameboy* temp = static_cast<Gameboy*>(emulator_.get());
            auto& pal = temp->GetPalette();
            pal = gb_palettes_;
        }
    }
    void Display::load_loop(){
        gladLoadGL();
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
        ImGui::SetColorEditOptions(ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_RGB | ImGuiColorEditFlags_PickerHueWheel);
        // TODO: design better icon
        // SDL_Surface* icon = SDL_LoadBMP(std::string(std::filesystem::current_path().string() + "/Resources/Images/icon.bmp").c_str());
        // SDL_SetWindowIcon(window_ptr_.get(), icon);
        // SDL_FreeSurface(icon);
        file_browser_.SetWindowSize(400, 400);
    }
    void Display::load_theme() {
        ImVec4* colors = ImGui::GetStyle().Colors;
        colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        colors[ImGuiCol_WindowBg]               = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
        colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_PopupBg]                = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
        colors[ImGuiCol_Border]                 = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
        colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_FrameBg]                = ImVec4(0.44f, 0.44f, 0.44f, 0.60f);
        colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.57f, 0.57f, 0.57f, 0.70f);
        colors[ImGuiCol_FrameBgActive]          = ImVec4(0.76f, 0.76f, 0.76f, 0.80f);
        colors[ImGuiCol_TitleBg]                = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
        colors[ImGuiCol_TitleBgActive]          = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 0.60f);
        colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
        colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
        colors[ImGuiCol_CheckMark]              = ImVec4(0.13f, 0.75f, 0.55f, 0.80f);
        colors[ImGuiCol_SliderGrab]             = ImVec4(0.13f, 0.75f, 0.75f, 0.80f);
        colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
        colors[ImGuiCol_Button]                 = ImVec4(0.13f, 0.75f, 0.55f, 0.40f);
        colors[ImGuiCol_ButtonHovered]          = ImVec4(0.13f, 0.75f, 0.75f, 0.60f);
        colors[ImGuiCol_ButtonActive]           = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
        colors[ImGuiCol_Header]                 = ImVec4(0.13f, 0.75f, 0.55f, 0.40f);
        colors[ImGuiCol_HeaderHovered]          = ImVec4(0.13f, 0.75f, 0.75f, 0.60f);
        colors[ImGuiCol_HeaderActive]           = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
        colors[ImGuiCol_ResizeGrip]             = ImVec4(0.13f, 0.75f, 0.55f, 0.40f);
        colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.13f, 0.75f, 0.75f, 0.60f);
        colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
        colors[ImGuiCol_Tab]                    = ImVec4(0.13f, 0.75f, 0.55f, 0.80f);
        colors[ImGuiCol_TabHovered]             = ImVec4(0.13f, 0.75f, 0.75f, 0.80f);
        colors[ImGuiCol_TabActive]              = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
        colors[ImGuiCol_TabUnfocused]           = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
        colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.36f, 0.36f, 0.36f, 0.54f);
        colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
        colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
        colors[ImGuiCol_TableBorderLight]       = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
        colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_TableRowBgAlt]          = ImVec4(1.00f, 1.00f, 1.00f, 0.07f);
        colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
        colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
        colors[ImGuiCol_NavHighlight]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
    }
    void Display::open_file_browser() {
        file_browser_.SetTitle("Select a ROM...");
        file_browser_.SetTypeFilters(SupportedRoms);
        std::filesystem::path path = settings_manager_.SaveDataDir.substr(0, settings_manager_.SaveDataDir.length() - 1);
        if (!(settings_.at("General.last_dir").empty())) {
            path = settings_.at("General.last_dir");
        }
        file_browser_.SetPwd(path);
        file_browser_.Open();
    }
    void Display::main_loop() {
        load_loop();
        bool loop = true;
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
            handle_shortcuts();
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplSDL2_NewFrame(window_ptr_.get());
            ImGui::NewFrame();
            draw_game_background(&rom_loaded_);
            draw_menu_bar(&menu_bar_open_);
            draw_fps_counter(&window_fpscounter_open_);
            draw_about(&window_about_open_);
            draw_tools();
            draw_settings(&window_settings_open_);
            draw_file_browser(&window_file_browser_open_);
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            SDL_GL_SwapWindow(window_ptr_.get());
        }
        ImGui::SaveIniSettingsToDisk(ImGuiSettingsFile.c_str());
        // Locking this mutex ensures that the other thread gets
        // enough time to exit before we close the main application.
        // TODO: code smell. find a different way to do this.
        if (emulator_) {
            emulator_->CloseAndWait();
        }
    }
}
