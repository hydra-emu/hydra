#include "display.h"
#include <memory>
#include <thread>

// TODO: refactor TKP::Graphics to TKP::Gameboy::Graphics and TKP::Devices to TKP::Gameboy::Devices
namespace TKP::Graphics {
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

            // Options menu
            if (ImGui::BeginPopup("Options"))
            {
                ImGui::Checkbox("Auto-scroll", &AutoScroll);
                ImGui::EndPopup();
            }

            // Main window
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
                // In this example we don't use the clipper when Filter is enabled.
                // This is because we don't have a random access on the result on our filter.
                // A real application processing logs with ten of thousands of entries may want to store the result of
                // search/filter.. especially if the filtering function is not trivial (e.g. reg-exp).
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
                // The simplest and easy way to display the entire buffer:
                //   ImGui::TextUnformatted(buf_begin, buf_end);
                // And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward
                // to skip non-visible lines. Here we instead demonstrate using the clipper to only process lines that are
                // within the visible area.
                // If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them
                // on your side is recommended. Using ImGuiListClipper requires
                // - A) random access into your data
                // - B) items all being the  same height,
                // both of which we can handle since we an array pointing to the beginning of each line of text.
                // When using the filter (in the block of code above) we don't have random access into the data to display
                // anymore, which is why we don't use the clipper. Storing or skimming through the search result would make
                // it possible (and would be recommended if you want to search through tens of thousands of entries).
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
    
	Display::DisplayInitializer::DisplayInitializer(PrettyPrinter& pprinter) {
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            pprinter.PrettyAdd<PPMessageType::ERROR>(SDL_GetError());
        }
        else {
            pprinter.PrettyAdd<PPMessageType::SUCCESS>("SDL initialized successfully.");
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
        SDL_GL_MakeCurrent(window_ptr_.get(), gl_context_ptr_.get());
        if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
            pretty_printer_.PrettyAdd<PPMessageType::ERROR>("Couldn't initialize glad.");
        }
        else {
            pretty_printer_.PrettyAdd<PPMessageType::SUCCESS>("Glad initialized successfully.");
        }
        SDL_GL_SetSwapInterval(0);
    }
    void Display::EnterMainLoop() {
        main_loop();
    }

    void Display::draw_settings(bool* draw) {
        if (*draw) {
            //static LogApp trace_logger;
            //ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
            //trace_logger.Draw("Trace Logger", draw);
        }
    }

    void Display::draw_trace_logger(bool* draw) {
        if (*draw) {
            static LogApp trace_logger;
            ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
            trace_logger.Draw("Trace Logger", draw);
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
                ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
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

    void Display::draw_menu_bar(bool* draw) {
        if (*draw) {
            if (ImGui::BeginMainMenuBar())
            {
                if (ImGui::BeginMenu("File")) {
                    draw_menu_bar_file();
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
        if (ImGui::MenuItem("Open ROM", "Ctrl+O")) {}
        if (ImGui::BeginMenu("Open Recent")) {
            draw_menu_bar_file_recent();
        }
        ImGui::Separator();
        // TODO: implement save and load state, disable these buttons if no rom loaded
        if (ImGui::MenuItem("Save State", "Ctrl+S", false, rom_loaded_)) {}
        if (ImGui::MenuItem("Load State", "Ctrl+L", false, rom_loaded_)) {}
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
        if (ImGui::MenuItem("Trace Logger", NULL, window_tracelogger_open_, rom_loaded_)) { window_tracelogger_open_ ^= true; }
        ImGui::EndMenu();
    }

    void Display::draw_menu_bar_view() {
        if (ImGui::MenuItem("FPS Counter", NULL, window_fpscounter_open_, true)) { 
            window_fpscounter_open_ ^= true;
        }
        ImGui::EndMenu();
    }

    void Display::load_user_settings() {
        FILE* file;
        fopen_s(&file, UserSettingsFile, "r+");
        if (file) {
            fseek(file, 0, SEEK_SET);
            set_user_settings<FileAccess::Read>(file);
            fclose(file);
        }
    }

    void Display::save_user_settings() {
        FILE* file;
        fopen_s(&file, UserSettingsFile, "w+");
        fseek(file, 0, SEEK_SET);
        set_user_settings<FileAccess::Write>(file);
        fclose(file);
    }

    void Display::main_loop() {
        glViewport(0, 0, window_settings_.window_width, window_settings_.window_height);
        IMGUI_CHECKVERSION();
        auto g = ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::StyleColorsDark();
        ImGui_ImplOpenGL3_Init(glsl_version.c_str());
        ImGui_ImplSDL2_InitForOpenGL(window_ptr_.get(), gl_context_ptr_.get());
        ImVec4 background = ImVec4(35 / 255.0f, 35 / 255.0f, 35 / 255.0f, 1.00f);
        glClearColor(background.x, background.y, background.z, background.w);
        load_user_settings();
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

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplSDL2_NewFrame(window_ptr_.get());
            ImGui::NewFrame();

            draw_menu_bar(&menu_bar_open_);
            draw_trace_logger(&window_tracelogger_open_);
            draw_fps_counter(&window_fpscounter_open_);

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            SDL_GL_SwapWindow(window_ptr_.get());
        }
    }
}
