#pragma once
#ifndef TKP_GB_DISPLAY_H
#define TKP_GB_DISPLAY_H
#include <glad/glad.h>
#include <SDL2/SDL.h>
#include <string>
#include <mutex>
#include <atomic>
#ifdef _WIN32
#include <windows.h>
#endif
#ifdef linux
#include <unistd.h>
#include <linux/limits.h>
#include <libgen.h>
#endif
#include "../emulator.h"
#include "../Tools/prettyprinter.h"
#include "../Tools/TKPImage.h"
#include "../ImGui/imgui_impl_sdl.h"
#include "../ImGui/imgui_impl_opengl3.h"
#include "../ImGui/imgui_internal.h"
#include "Applications/application_settings.h"
#include "Applications/base_disassembler.h"
#include "Applications/base_tracelogger.h"
#include "Applications/imfilebrowser.h"
#include "Applications/settings.h"
// Helper Macros - IM_FMTARGS, IM_FMTLIST: Apply printf-style warnings to our formatting functions.
#if !defined(IMGUI_USE_STB_SPRINTF) && defined(__MINGW32__) && !defined(__clang__)
#define IM_FMTARGS(FMT)             __attribute__((format(gnu_printf, FMT, FMT+1)))
#define IM_FMTLIST(FMT)             __attribute__((format(gnu_printf, FMT, 0)))
#elif !defined(IMGUI_USE_STB_SPRINTF) && (defined(__clang__) || defined(__GNUC__))
#define IM_FMTARGS(FMT)             __attribute__((format(printf, FMT, FMT+1)))
#define IM_FMTLIST(FMT)             __attribute__((format(printf, FMT, 0)))
#else
#define IM_FMTARGS(FMT)
#define IM_FMTLIST(FMT)
#endif
namespace TKPEmu::Graphics {
    constexpr auto GameboyWidth = 160;
    constexpr auto GameboyHeight = 144;
    constexpr auto MenuBarHeight = 19;
    using TKPImage = TKPEmu::Tools::TKPImage;
    enum class FileAccess { Read, Write };
    struct WindowSettings {
        int window_width = 640;
        int window_height = 480;
        int minimum_width = GameboyWidth;
        int minimum_height = GameboyHeight + MenuBarHeight;
        // TODO: replace these magic numbers with screen height / width / -1
        int maximum_width = 1920;
        int maximum_height = 1080;
    };
    enum class EmulatorType {
        None,
        Gameboy,
    };
	class Display {
    private:
        using PrettyPrinter = TKPEmu::Tools::PrettyPrinter;
        using SDL_GLContextType = std::remove_pointer_t<SDL_GLContext>;
        using PPMessageType = TKPEmu::Tools::PrettyPrinterMessageType;
        using BaseDisassembler = TKPEmu::Applications::BaseDisassembler;
        using BaseTracelogger = TKPEmu::Applications::BaseTracelogger;
        using Settings = TKPEmu::Applications::Settings;
        const std::string glsl_version = "#version 430";
        std::string BackgroundImageFile = "tkp_bg.jpg";
        std::string ImGuiSettingsFile = "imgui.ini";
        std::string UserSettingsFile = "tkpuser.ini";
        std::string ResourcesDataDir = "\\Resources\\Data\\";
        std::string ResourcesRomsDir = "\\Resources\\ROMs";
        std::string ResourcesImagesDir = "\\Resources\\Images\\";
        std::vector<std::string> SupportedRoms = { ".gb" };
        #ifdef _WIN32
        wchar_t exe_dir[MAX_PATH];
        #endif
        #ifdef linux
        char result[PATH_MAX];
        ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
        std::string exe_dir = dirname(result);
        #endif
        std::string ExecutableDirectory;
    private:
        // RAII class for the initialization functions
        class DisplayInitializer {
        public:
            DisplayInitializer(PrettyPrinter&);
            ~DisplayInitializer();
        };
    public:
        Display();
        ~Display();
        Display(const Display&) = delete;
        Display& operator=(const Display&) = delete;
        void EnterMainLoop();
    private:
        // This member being first means that it gets constructed first and gets destructed last
        // which is what we want to happen with the SDL_Init and the destroy functions
        DisplayInitializer display_initializer_;

        // PrettyPrinter class is used to add strings to a buffer that
        // the user can later get to print however they want.
        PrettyPrinter pretty_printer_;
        WindowSettings window_settings_;
        AppSettings app_settings_;
        Settings settings_;
        TKPImage background_image_;
        GLuint frame_buffer_;
        ImGui::FileBrowser file_browser_;

        std::unique_ptr<Emulator> emulator_ = nullptr;
        std::unique_ptr<BaseDisassembler> disassembler_ = nullptr;
        std::unique_ptr<BaseTracelogger> tracelogger_ = nullptr;
        std::unique_ptr<SDL_GLContextType, decltype(&SDL_GL_DeleteContext)> gl_context_ptr_;
        std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window_ptr_;
        std::chrono::system_clock::time_point a = std::chrono::system_clock::now();
        std::chrono::system_clock::time_point b = std::chrono::system_clock::now();
        float sleep_time_ = 16.75f;

        // These bools determine whether certain windows are open
        // We aren't allowed to use static member variables so we use static functions that
        // return static members
        bool rom_loaded_ = false;
        bool rom_paused_ = false;
        // Log mode is a mode that waits for the tracelogger to log each instruction before
        // stepping into the next one
        bool log_mode_ = false;
        EmulatorType emulator_type_ = EmulatorType::None;
        bool menu_bar_open_ = true;
        bool window_tracelogger_open_ = false;
        bool window_fpscounter_open_ = false;
        bool window_settings_open_ = false;
        bool window_file_browser_open_ = false;
        bool window_disassembler_open_ = false;

        // Shortcut pressed booleans
        bool reset_pressed_ = false;
        bool pause_pressed_ = false;

        // Window drawing functions for ImGui
        void draw_settings(bool* draw);
        void draw_trace_logger(bool* draw);
        void draw_disassembler(bool* draw);
        void draw_fps_counter(bool* draw);
        void draw_file_browser(bool* draw);
        void draw_game_background(bool* draw);
        void draw_menu_bar(bool* draw);
        void draw_menu_bar_file();
        void draw_menu_bar_file_recent();
        void draw_menu_bar_tools();
        void draw_menu_bar_view();
        
        // Helper functions
        bool load_image_from_file(const char* filename, TKPImage& out);
        void limit_fps();
        inline bool is_rom_loaded();
        inline bool is_rom_loaded_and_debugmode();
        inline bool is_rom_loaded_and_logmode();

        // This function deals with scaling the gameboy screen texture without stretching it
        inline void image_scale(ImVec2& topleft, ImVec2& bottomright);
        // These two functions save and load user settings stored in the app_settings_ object
        void load_user_settings();
        void save_user_settings();
        
        void load_rom(std::filesystem::path&& path);
        // Sends the appropriate flags to close the running emulator thread and waits until its closed
        void close_emulator_and_wait();
        void step_emulator();
        // This function chooses fread or fwrite on compile time to minimize code duplication
        template<FileAccess acc>
        void access_user_settings(void* item, size_t size, size_t count, FILE* stream);
        void setup_gameboy_palette();
        void main_loop();
	};
    template<>
    inline void Display::access_user_settings<FileAccess::Read>(void* item, size_t size, size_t count, FILE* stream) {
        fread(item, size, count, stream);
    }
    template<>
    inline void Display::access_user_settings<FileAccess::Write>(void* item, size_t size, size_t count, FILE* stream) {
        fwrite(item, size, count, stream);
    }
}
#endif