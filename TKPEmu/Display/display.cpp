#define NOMINMAX
#define STB_IMAGE_IMPLEMENTATION
#include "display.h"
#include <memory>
#include <thread>
#include "../ImGui/stb_image.h"
#include "../Gameboy/gameboy.h"
#include "../Tools/disassembly_instr.h"
#include <iostream>
#include <algorithm>
namespace TKPEmu::Graphics {
    using TKPEmu::Tools::DisInstr;
    struct LogApp
    {
        ImGuiTextBuffer     Buf;
        ImGuiTextFilter     Filter;
        ImVector<int>       LineOffsets; // Index to lines offset. We maintain this with AddLog() calls.
        bool                AutoScroll;  // Keep scrolling if already at the bottom.

        LogApp()
        {
            AutoScroll = true;
            Clear();
        }

        void Clear()
        {
            Buf.clear();
            LineOffsets.clear();
            LineOffsets.push_back(0);
        }

        void AddLog(const char* fmt, ...) IM_FMTARGS(2)
        {
            int old_size = Buf.size();
            va_list args;
            va_start(args, fmt);
            Buf.appendfv(fmt, args);
            va_end(args);
            for (int new_size = Buf.size(); old_size < new_size; old_size++)
                if (Buf[old_size] == '\n')
                    LineOffsets.push_back(old_size + 1);
        }
        void Draw(const char* title, bool* p_open = NULL)
        {
            if (!ImGui::Begin(title, p_open))
            {
                ImGui::End();
                return;
            }
            if (ImGui::BeginPopup("Options"))
            {
                ImGui::Checkbox("Auto-scroll", &AutoScroll);
                ImGui::EndPopup();
            }
            if (ImGui::Button("Options"))
                ImGui::OpenPopup("Options");
            ImGui::SameLine();
            bool clear = ImGui::Button("Clear");
            ImGui::SameLine();
            bool copy = ImGui::Button("Copy");
            ImGui::SameLine();
            Filter.Draw("Filter", -100.0f);

            ImGui::Separator();
            ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

            if (clear)
                Clear();
            if (copy)
                ImGui::LogToClipboard();

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            const char* buf = Buf.begin();
            const char* buf_end = Buf.end();
            if (Filter.IsActive())
            {
                for (int line_no = 0; line_no < LineOffsets.Size; line_no++)
                {
                    const char* line_start = buf + LineOffsets[line_no];
                    const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                    if (Filter.PassFilter(line_start, line_end))
                        ImGui::TextUnformatted(line_start, line_end);
                }
            }
            else
            {
                ImGuiListClipper clipper;
                clipper.Begin(LineOffsets.Size);
                while (clipper.Step())
                {
                    for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
                    {
                        const char* line_start = buf + LineOffsets[line_no];
                        const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                        ImGui::TextUnformatted(line_start, line_end);
                    }
                }
                clipper.End();
            }
            ImGui::PopStyleVar();

            if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                ImGui::SetScrollHereY(1.0f);

            ImGui::EndChild();
            ImGui::End();
        }
    };
    
    struct SettingsApp {
        AppSettings* current_settings;
        float* sleep_time;
        SettingsApp(AppSettings* cur, float* sleep_time) : current_settings(cur), sleep_time(sleep_time) {  }
        static auto GetFpsValue(int i) {
            static const int fps_values[] = { 30, 60, 120, 144, 240 };
            return fps_values[i];
        }
        void Draw(const char* title, bool* p_open = NULL)
        {
            if (!ImGui::Begin(title, p_open))
            {
                ImGui::End();
                return;
            }
            if (ImGui::CollapsingHeader("Video")) {
                ImGui::Text("General video settings:");
                if (ImGui::Checkbox("FPS Limit", (bool*)(&(current_settings->limit_fps)))) { };
                if (ImGui::IsItemHovered()) {
                    ImGui::BeginTooltip();
                    ImGui::Text("Enable to save CPU cycles.");
                    ImGui::EndTooltip();
                }
                if (current_settings->limit_fps) {
                    ImGui::SameLine();
                    static const char* fps_types[] = { "30", "60", "120", "144", "240" };
                    static int curr = ((int)current_settings->max_fps_index);
                    if (ImGui::Combo("combo", &curr, fps_types, IM_ARRAYSIZE(fps_types))) {
                        *sleep_time = 1000.0f / GetFpsValue(curr);
                        current_settings->max_fps_index = curr;
                    }
                }
                ImGui::Separator();
                ImGui::Text("Set the 4 colors used by the Gameboy:");
                static float c1[3] = { (float)current_settings->dmg_color0_r / 255, (float)current_settings->dmg_color0_g / 255, (float)current_settings->dmg_color0_b / 255 };
                static float c2[3] = { (float)current_settings->dmg_color1_r / 255, (float)current_settings->dmg_color1_g / 255, (float)current_settings->dmg_color1_b / 255 };
                static float c3[3] = { (float)current_settings->dmg_color2_r / 255, (float)current_settings->dmg_color2_g / 255, (float)current_settings->dmg_color2_b / 255 };
                static float c4[3] = { (float)current_settings->dmg_color3_r / 255, (float)current_settings->dmg_color3_g / 255, (float)current_settings->dmg_color3_b / 255 };
                if (ImGui::ColorEdit3("Color 1", c1)) {
                    current_settings->dmg_color0_r = (int)(c1[0] * 255.0f);
                    current_settings->dmg_color0_g = (int)(c1[1] * 255.0f);
                    current_settings->dmg_color0_b = (int)(c1[2] * 255.0f);
                }
                if (ImGui::ColorEdit3("Color 2", c2)) {
                    current_settings->dmg_color1_r = (int)(c2[0] * 255.0f);
                    current_settings->dmg_color1_g = (int)(c2[1] * 255.0f);
                    current_settings->dmg_color1_b = (int)(c2[2] * 255.0f);
                }
                if (ImGui::ColorEdit3("Color 3", c3)) {
                    current_settings->dmg_color2_r = (int)(c3[0] * 255.0f);
                    current_settings->dmg_color2_g = (int)(c3[1] * 255.0f);
                    current_settings->dmg_color2_b = (int)(c3[2] * 255.0f);
                }
                if (ImGui::ColorEdit3("Color 4", c4)) {
                    current_settings->dmg_color3_r = (int)(c4[0] * 255.0f);
                    current_settings->dmg_color3_g = (int)(c4[1] * 255.0f);
                    current_settings->dmg_color3_b = (int)(c4[2] * 255.0f);
                }
            }

            ImGui::End();
        }
    };
	Display::DisplayInitializer::DisplayInitializer(PrettyPrinter& pprinter) {
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            pprinter.PrettyAdd<PPMessageType::Error>(SDL_GetError());
        }
        else {
            pprinter.PrettyAdd<PPMessageType::Success>("SDL initialized successfully.");
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
        window_ptr_(nullptr, SDL_DestroyWindow),
        gl_context_ptr_(nullptr, SDL_GL_DeleteContext),
        display_initializer_(pretty_printer_)
    {
        #ifdef _WIN32
        GetModuleFileNameW(NULL, exe_dir, MAX_PATH);
        char DefChar = ' ';
        char res[260];
        WideCharToMultiByte(CP_ACP, 0, exe_dir, -1, res, MAX_PATH, &DefChar, NULL);
        ExecutableDirectory = res;
        const size_t last_slash_idx = ExecutableDirectory.rfind('\\');
        if (std::string::npos != last_slash_idx)
        {
            ExecutableDirectory = ExecutableDirectory.substr(0, last_slash_idx);
        }
        ImGuiSettingsFile = ExecutableDirectory + ResourcesDataDir + ImGuiSettingsFile;
        UserSettingsFile = ExecutableDirectory + ResourcesDataDir + UserSettingsFile;
        #endif
        // TODO: Implement linux path
        SDL_WindowFlags window_flags = (SDL_WindowFlags)(
            SDL_WINDOW_OPENGL
            | SDL_WINDOW_RESIZABLE
            | SDL_WINDOW_ALLOW_HIGHDPI
            );
        window_ptr_.reset(SDL_CreateWindow(
            "GameboyEmuTKP",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            window_settings_.window_width,
            window_settings_.window_height,
            window_flags
        ));
        gl_context_ptr_.reset(SDL_GL_CreateContext(window_ptr_.get()));
        SDL_SetWindowMinimumSize(window_ptr_.get(), window_settings_.minimum_width, window_settings_.minimum_height);
        SDL_SetWindowMaximumSize(window_ptr_.get(), window_settings_.maximum_width, window_settings_.maximum_height);
        std::filesystem::create_directory(ExecutableDirectory + ResourcesDataDir);
        std::filesystem::create_directory(ExecutableDirectory + ResourcesImagesDir);
        std::filesystem::create_directory(ExecutableDirectory + ResourcesRomsDir);
        SDL_GL_MakeCurrent(window_ptr_.get(), gl_context_ptr_.get());
        if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
            pretty_printer_.PrettyAdd<PPMessageType::Error>("Couldn't initialize glad.");
        }
        else {
            pretty_printer_.PrettyAdd<PPMessageType::Success>("Glad initialized successfully.");
        }

    }
    void Display::EnterMainLoop() {
        main_loop();
    }

    void Display::draw_settings(bool* draw) {
        if (*draw) {
            static SettingsApp settings(&app_settings_, &sleep_time_);
            ImGui::SetNextWindowSize(ImVec2(400,400), ImGuiCond_FirstUseEver);
            settings.Draw("Settings", draw);
        }
    }

    void Display::draw_trace_logger(bool* draw) {
        if (*draw && rom_loaded_) {
            static LogApp trace_logger;
            ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
            trace_logger.Draw("Trace Logger", draw);
        }
    }

    // TODO: add filtering, breakpoints, searching
    void Display::draw_disassembler(bool* draw) {
        if (*draw && rom_loaded_) {
            if (disassembler_.Loaded == false && !disassembler_.Loading) {
                disassembler_.Loading = true;
                emulator_->LoadInstrToVec(disassembler_.Instrs, disassembler_.Loaded);
            }
            ImGui::SetNextWindowSize(ImVec2(500, 500), ImGuiCond_FirstUseEver);
            disassembler_.Draw("Disassembler", draw);
        }
    }

    void Display::draw_fps_counter(bool* draw){
        if (*draw) {
            static int corner = 0;
            ImGuiIO& io = ImGui::GetIO();
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
            ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
            if (ImGui::Begin("Overlay", draw, window_flags))
            {
                ImGui::Text("FPS: %.2f", ImGui::GetIO().Framerate);
                ImGui::Separator();
                ImGui::Text("(right click for options)");
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

    void Display::draw_file_browser(bool* draw) {
        if (*draw) {
            file_browser_.Display();
            if (file_browser_.HasSelected()) {
                std::cout << "Selected filename" << file_browser_.GetSelected().string() << std::endl;
                window_file_browser_open_ = false;
                auto ext = file_browser_.GetSelected().extension();
                auto t = file_browser_.GetSelected();
                rom_loaded_ = true;
                if (ext == ".gb") {
                    emulator_ = std::make_unique<Gameboy::Gameboy>();
                    emulator_->LoadFromFile(file_browser_.GetSelected().string());
                    emulator_->Paused.store(true);
                    emulator_->Start();
                    disassembler_.Loaded = false;
                    disassembler_.Loading = false;
                    disassembler_.Instrs.clear();
                }
                file_browser_.ClearSelected();
            }
            if (file_browser_.ShouldClose()) {
                window_file_browser_open_ = false;
            }
        }
    }

    void Display::draw_game_background(bool* draw) {
        if (*draw) {
            //ImGui::GetBackgroundDrawList()->AddImage((void*)(GLuint*)(emulator_->EmulatorImage->texture), emulator_->EmulatorImage->topleft, emulator_->EmulatorImage->botright);
        }
        else {
            ImGui::GetBackgroundDrawList()->AddImage((void*)(GLuint*)(background_image_.texture), background_image_.topleft, background_image_.botright);
            if (rom_paused_)
                ImGui::GetBackgroundDrawList()->AddText(background_image_.topleft, ImColor(255, 0, 0), "Paused.");
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
                    draw_menu_bar_emulation();
                }
                if (ImGui::BeginMenu("View")) {
                    draw_menu_bar_view();
                }
                if (ImGui::BeginMenu("Tools")) {
                    draw_menu_bar_tools();
                }
                ImGui::EndMainMenuBar();
            }
        }
    }

    void Display::draw_menu_bar_file() {
        if (ImGui::MenuItem("Open ROM", "Ctrl+O", window_file_browser_open_, true)) {
            window_file_browser_open_ ^= true;
            if (window_file_browser_open_) {
                file_browser_.SetTitle("Select a ROM...");
                file_browser_.SetTypeFilters(SupportedRoms);
                file_browser_.SetPwd(ExecutableDirectory + ResourcesRomsDir);
                file_browser_.Open();
            }
        }
        if (ImGui::BeginMenu("Open Recent")) {
            draw_menu_bar_file_recent();
        }
        ImGui::Separator();
        // TODO: implement save and load state, disable these buttons if no rom loaded
        if (ImGui::MenuItem("Save State", "Ctrl+S", false, rom_loaded_)) {}
        if (ImGui::MenuItem("Load State", "Ctrl+L", false, rom_loaded_)) {}
        ImGui::Separator();
        if (ImGui::MenuItem("Settings", NULL, window_settings_open_, true)) { 
            window_settings_open_ ^= true; 
        }
        ImGui::EndMenu();
    }

    void Display::draw_menu_bar_emulation() {
        if (ImGui::MenuItem("Stop", nullptr, false, rom_loaded_)) {
            rom_loaded_ = false;
            emulator_->Stopped.store(true, std::memory_order_relaxed);
        }
        if (ImGui::MenuItem("Pause", nullptr, rom_paused_, rom_loaded_)) {
            rom_paused_ ^= true;
            emulator_->Paused.store(rom_paused_, std::memory_order_relaxed);
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
        // TODO: add seperator and text that tells you what to do to activate these
        if (ImGui::MenuItem("Trace Logger", NULL, window_tracelogger_open_, rom_loaded_debug_)) { window_tracelogger_open_ ^= true; }
        if (ImGui::MenuItem("Disassembler", NULL, window_disassembler_open_, rom_loaded_debug_)) { window_disassembler_open_ ^= true; }
        ImGui::EndMenu();
    }

    void Display::draw_menu_bar_view() {
        if (ImGui::MenuItem("FPS Counter", NULL, window_fpscounter_open_, true)) {  window_fpscounter_open_ ^= true; }
        ImGui::EndMenu();
    }

    bool Display::load_image_from_file(const char* filename, TKPImage& out) {
        // Load from file
        unsigned char* image_data = stbi_load(filename, &out.width, &out.height, NULL, 4);
        if (image_data == NULL)
            return false;

        // Create a OpenGL texture identifier
        GLuint image_texture;
        glGenTextures(1, &image_texture);
        glBindTexture(GL_TEXTURE_2D, image_texture);

        // Setup filtering parameters for display
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

        // Upload pixels into texture
        #if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
                glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        #endif
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, out.width, out.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
        stbi_image_free(image_data);
        out.texture = image_texture;
        image_scale(out.topleft, out.botright);
        return true;
    }

    inline void Display::image_scale(ImVec2& topleft, ImVec2& bottomright)
    {
        float wi = background_image_.width; float hi = background_image_.height;
        float ws = window_settings_.window_width; float hs = window_settings_.window_height;
        float ri = wi / hi;
        float rs = ws / hs;
        float new_w;
        float new_h;
        if (rs > ri) {
            new_w = wi * (hs / hi);
            new_h = hs;
        }
        else {
            new_w = ws;
            new_h = hi * (ws / wi);
        }
        topleft.y = (hs - new_h) / 2 + MenuBarHeight / 2;
        topleft.x = (ws - new_w) / 2;
        bottomright.x = new_w + topleft.x;
        bottomright.y = new_h + topleft.y;
    }

    void Display::load_user_settings() {
        FILE* file;
        fopen_s(&file, UserSettingsFile.c_str(), "r+");
        if (file) {
            fseek(file, 0, SEEK_SET);
            access_user_settings<FileAccess::Read>(&app_settings_, sizeof(AppSettingsType), AppSettingsSize, file);
            fclose(file);
        }
    }

    void Display::save_user_settings() {
        FILE* file;
        fopen_s(&file, UserSettingsFile.c_str(), "w+");
        if (file) {
            fseek(file, 0, SEEK_SET);
            access_user_settings<FileAccess::Write>(&app_settings_, sizeof(AppSettingsType), AppSettingsSize, file);
            fclose(file);
        }
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
        std::chrono::duration<double, std::milli> sleep_time = b - a;
        
    }

    void Display::main_loop() {
        glViewport(0, 0, window_settings_.window_width, window_settings_.window_height);
        IMGUI_CHECKVERSION();
        auto g = ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = NULL;
        ImGui::StyleColorsDark();
        ImGui_ImplOpenGL3_Init(glsl_version.c_str());
        ImGui_ImplSDL2_InitForOpenGL(window_ptr_.get(), gl_context_ptr_.get());
        ImVec4 background = ImVec4(35 / 255.0f, 35 / 255.0f, 35 / 255.0f, 1.00f);
        glClearColor(background.x, background.y, background.z, background.w);
        load_user_settings();
        SDL_GL_SetSwapInterval(0);
        sleep_time_ = 1000.0f / SettingsApp::GetFpsValue(app_settings_.max_fps_index);
        ImGui::LoadIniSettingsFromDisk(ImGuiSettingsFile.c_str());
        load_image_from_file((ExecutableDirectory + ResourcesImagesDir + BackgroundImageFile).c_str(), background_image_);
        file_browser_.SetWindowSize(300, 300);
        bool loop = true;
        while (loop)
        {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                // without it you won't have keyboard input and other things
                ImGui_ImplSDL2_ProcessEvent(&event);
                // you might also want to check io.WantCaptureMouse and io.WantCaptureKeyboard
                // before processing events

                switch (event.type)
                {
                case SDL_QUIT:
                    save_user_settings();
                    loop = false;
                    break;

                case SDL_WINDOWEVENT:
                    switch (event.window.event)
                    {
                    case SDL_WINDOWEVENT_RESIZED:
                        window_settings_.window_width = event.window.data1;
                        window_settings_.window_height = event.window.data2;
                        image_scale(background_image_.topleft, background_image_.botright);
                        glViewport(0, 0, window_settings_.window_width, window_settings_.window_height);
                        break;
                    }
                    break;

                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym)
                    {
                    case SDLK_ESCAPE:
                        loop = false;
                        break;
                    }
                    break;
                }
            }
            if (app_settings_.limit_fps) {
                limit_fps();
            }

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplSDL2_NewFrame(window_ptr_.get());
            ImGui::NewFrame();
            draw_game_background(&rom_loaded_);
            draw_menu_bar(&menu_bar_open_);
            draw_trace_logger(&window_tracelogger_open_);
            draw_fps_counter(&window_fpscounter_open_);
            draw_settings(&window_settings_open_);
            draw_file_browser(&window_file_browser_open_);
            draw_disassembler(&window_disassembler_open_);

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            SDL_GL_SwapWindow(window_ptr_.get());
        }
        save_user_settings();
        ImGui::SaveIniSettingsToDisk(ImGuiSettingsFile.c_str());
    }
}
