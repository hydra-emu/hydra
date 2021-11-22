#pragma once
#ifndef TKP_GB_DISPLAY_H
#define TKP_GB_DISPLAY_H
#include "../glad/glad/glad.h"
#include <SDL2/SDL.h>
#include <string>
#include <mutex>
#include <atomic>
#include "emulator.h"
#include "TKPImage.h"
#include "../lib/imgui_impl_sdl.h"
#include "../lib/imgui_impl_opengl3.h"
#include "../lib/imgui_internal.h"
#include "widget_keyselector.h"
#include "base_application.h"
#include "imfilebrowser.h"
#include "settings_manager.h"
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
namespace TKPEmu::Applications {
    class BaseTracelogger;
    class BaseDisassembler;
}
namespace TKPEmu::Graphics {
    constexpr auto GameboyWidth = 160;
    constexpr auto GameboyHeight = 144;
    constexpr auto MenuBarHeight = 19;
    using TKPImage = TKPEmu::Tools::TKPImage;
    using KeySelector = TKPEmu::Tools::KeySelector;
    using EmuStartOptions = TKPEmu::EmuStartOptions;
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
        using SDL_GLContextType = std::remove_pointer_t<SDL_GLContext>;
        using BaseDisassembler = TKPEmu::Applications::BaseDisassembler;
        using BaseTracelogger = TKPEmu::Applications::BaseTracelogger;
        using SettingsManager = TKPEmu::Tools::SettingsManager;
        using DisInstr = TKPEmu::Tools::DisInstr;
        using GameboyPalettes = std::array<std::array<float, 3>,4>;
        using GameboyKeys = std::array<SDL_Keycode, 4>;
        const std::string glsl_version = "#version 430";
        std::string BackgroundImageFile = "tkp_bg.jpg";
        std::string ImGuiSettingsFile = "imgui.ini";
        std::string ResourcesDataDir = "/Resources/Data/";
        std::string ResourcesRomsDir = "/Resources/ROMs";
        std::string ResourcesImagesDir = "/Resources/Images/";
        std::vector<std::string> SupportedRoms = { ".gb" };
        #ifdef _WIN32
        wchar_t exe_dir[MAX_PATH];
        #endif
        std::string ExecutableDirectory;
    private:
        // RAII class for the initialization functions
        class DisplayInitializer {
        public:
            DisplayInitializer();
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

        // To be used with settings_manager, these are the settings keys with their default values
        TKPEmu::Tools::SettingsMap settings_ =
        {
            {"General.debug_mode", "1"},
            {"General.fast_mode", "0"},
            {"Video.limit_fps", "1"},
            {"Video.max_fps", "60"},
            {"Gameboy.color0", "d0d058"},
            {"Gameboy.color1", "a0a840"},
            {"Gameboy.color2", "708028"},
            {"Gameboy.color3", "405010"},
            {"Gameboy.key_right", std::to_string(SDLK_RIGHT)},
            {"Gameboy.key_left", std::to_string(SDLK_LEFT)},
            {"Gameboy.key_down", std::to_string(SDLK_DOWN)},
            {"Gameboy.key_up", std::to_string(SDLK_UP)},
            {"Gameboy.key_a", std::to_string(SDLK_z)},
            {"Gameboy.key_b", std::to_string(SDLK_x)},
            {"Gameboy.key_start", std::to_string(SDLK_RETURN)},
            {"Gameboy.key_select", std::to_string(SDLK_SPACE)},
        };
        SettingsManager settings_manager_;

        // Emulation specific settings
        GameboyPalettes gb_palettes_{};
        GameboyKeys gb_keys_directional_{};
        GameboyKeys gb_keys_action_{};

        bool limit_fps_ = true;
        int max_fps_ = 60;
        bool debug_mode_ = true;
        EmuStartOptions emulator_start_opt_ = EmuStartOptions::Normal;
        WindowSettings window_settings_;
        TKPImage background_image_;
        GLuint frame_buffer_;
        ImGui::FileBrowser file_browser_;
        std::unique_ptr<Emulator> emulator_ = nullptr;
        std::unique_ptr<BaseDisassembler> disassembler_;
        std::unique_ptr<BaseTracelogger> tracelogger_;
        std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window_ptr_;
        std::unique_ptr<SDL_GLContextType, decltype(&SDL_GL_DeleteContext)> gl_context_ptr_;
        std::chrono::system_clock::time_point a = std::chrono::system_clock::now();
        std::chrono::system_clock::time_point b = std::chrono::system_clock::now();
        // This variable helps for setting up the controls
        SDL_Keycode last_key_pressed_ = 0;
        EmulatorType emulator_type_ = EmulatorType::None;
        float sleep_time_ = 16.75f;

        // These bools determine whether certain windows are open
        // We aren't allowed to use static member variables so we use static functions that
        // return static members
        bool rom_loaded_ = false;
        bool rom_paused_ = false;
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
        void open_file_browser();
        
        // Helper functions
        inline bool load_image_from_file(const char* filename, TKPImage& out);
        inline void limit_fps();
        inline void init_settings_values();
        inline void init_gameboy_values();
        inline bool is_rom_loaded();
        inline bool is_rom_loaded_and_debugmode();

        // This function deals with scaling the gameboy screen texture without stretching it
        inline void image_scale(ImVec2& topleft, ImVec2& bottomright);

        void load_rom(std::filesystem::path&& path);
        // Sends the appropriate flags to close the running emulator thread and waits until its closed
        void close_emulator_and_wait();
        void step_emulator();
        void setup_gameboy_palette();
        void load_loop();
        void main_loop();
	};
}
#endif
