#pragma once
#ifndef TKP_GB_DISPLAY_H
#define TKP_GB_DISPLAY_H
#include <SDL2/SDL.h>
#include <string>
#include <mutex>
#include <deque>
#include <atomic>
#include <GL/glew.h>
#include <lib/imgui_impl_sdl.h>
#include <lib/imgui_impl_opengl3.h>
#include <imgui/imgui_internal.h>
#include <lib/widget_keyselector.h>
#include <lib/imfilebrowser.h>
#include <include/generic_drawable.h>
#include "emulator_types.hxx"
#include "TKPImage.h"
#include "settings_manager.h"
#include "base_application.h"
#include "emulator_factory.h"
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
// runs a function that may throw - to be used inside display member functions
#define TKP_MAY_THROW(func) do { \
                                try { \
                                    func; \
                                } catch (const std::runtime_error& ex) { \
                                    throw_error(ex); \
                                } \
                            } while(0)
// runs a function that may throw - to be used inside display member functions
// and runs an action after throwing
#define TKP_MAY_THROW_ACTION(func, action)  do { \
                                                try { \
                                                    func; \
                                                } catch (const std::runtime_error& ex) { \
                                                    throw_error(ex); \
                                                    action; \
                                                } \
                                            } while(0)


namespace TKPEmu::Graphics {
    constexpr auto GameboyWidth = 160;
    constexpr auto GameboyHeight = 144;
    constexpr auto MenuBarHeight = 19;
    constexpr auto RecentRomsMaxSize = 5;
    struct WindowSettings {
        int window_width = 640;
        int window_height = 480;
        int minimum_width = GameboyWidth;
        int minimum_height = GameboyHeight + MenuBarHeight;
        // TODO: replace these magic numbers with screen height / width / -1
        int maximum_width = 1920;
        int maximum_height = 1080;
    };
	class Display {
    private:
        using TKPShortcut = TKPEmu::Applications::TKPShortcut;
        using TKPImage = TKPEmu::Tools::TKPImage;
        using KeySelector = TKPEmu::Tools::KeySelector;
        using EmuStartOptions = TKPEmu::EmuStartOptions;
        using SDL_GLContextType = std::remove_pointer_t<SDL_GLContext>;
        using IMApplication = TKPEmu::Applications::IMApplication;
        using Drawable = TKPEmu::Applications::Drawable;
        using SettingsManager = TKPEmu::Tools::SettingsManager;
        using DisInstr = TKPEmu::Tools::DisInstr;
        using GameboyPalettes = std::array<std::array<float, 3>,4>;
        using GameboyKeys = std::array<SDL_Keycode, 4>;
        using Chip8Keys = std::array<SDL_Keycode, 16>;
        const std::string GLSLVersion = "#version 130";
        #ifdef _WIN32
        wchar_t exe_dir[MAX_PATH];
        #endif
        std::string ImGuiSettingsFile = "imgui.ini";
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
        void WS_SetActionFlag(int* action_ptr);
        void WS_LoadRom(std::string path);
    private:
        // This member being first means that it gets constructed first and gets destructed last
        // which is what we want to happen with the SDL_Init and the destroy functions
        DisplayInitializer display_initializer_;
        std::deque<std::filesystem::path> recent_paths_;

        // To be used with settings_manager, these are the settings keys with their default values
        TKPEmu::Tools::SettingsMap settings_ =
        {
            {"General.debug_mode", "1"},
            {"General.skip_boot", "0"},
            {"General.fast_mode", "0"},
            {"General.last_dir", ""},
            {"General.recent0", ""},
            {"General.recent1", ""},
            {"General.recent2", ""},
            {"General.recent3", ""},
            {"General.recent4", ""},
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
            {"Chip8.key_1", std::to_string(SDLK_1)},
            {"Chip8.key_2", std::to_string(SDLK_2)},
            {"Chip8.key_3", std::to_string(SDLK_3)},
            {"Chip8.key_c", std::to_string(SDLK_4)},
            {"Chip8.key_4", std::to_string(SDLK_q)},
            {"Chip8.key_5", std::to_string(SDLK_w)},
            {"Chip8.key_6", std::to_string(SDLK_e)},
            {"Chip8.key_d", std::to_string(SDLK_r)},
            {"Chip8.key_7", std::to_string(SDLK_a)},
            {"Chip8.key_8", std::to_string(SDLK_s)},
            {"Chip8.key_9", std::to_string(SDLK_d)},
            {"Chip8.key_e", std::to_string(SDLK_f)},
            {"Chip8.key_a", std::to_string(SDLK_z)},
            {"Chip8.key_0", std::to_string(SDLK_x)},
            {"Chip8.key_b", std::to_string(SDLK_c)},
            {"Chip8.key_f", std::to_string(SDLK_v)},
            {"N64.ipl_loc", "not set"},
        };
        SettingsManager settings_manager_;

        // Emulation specific settings
        GameboyPalettes gb_palettes_{};
        GameboyKeys gb_keys_direction_{};
        GameboyKeys gb_keys_action_{};
        Chip8Keys chip8_keys_{};
        std::filesystem::path n64_ipl_loc_{};

        bool limit_fps_ = true;
        int max_fps_ = 60;
        int* action_ptr_ = nullptr;
        bool debug_mode_ = true;
        bool skip_boot_ = false;
        bool fast_mode_ = false;
        WindowSettings window_settings_;
        GLuint frame_buffer_;
        ImGui::FileBrowser file_browser_;
        using file_browser_callback_fncptr = void (Display::*)(std::filesystem::path);
        file_browser_callback_fncptr file_browser_callback_;
        EmuType emulator_type_ = EmuType::None;
        std::shared_ptr<Emulator> emulator_;
        // Applications loaded according to the emulator (such as disassembler, tracelogger, other plugins etc)
        std::vector<std::unique_ptr<IMApplication>> emulator_tools_;
        // Generic tools that work across emulators
        std::vector<std::unique_ptr<Drawable>> generic_tools_;
        std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window_ptr_;
        std::unique_ptr<SDL_GLContextType, decltype(&SDL_GL_DeleteContext)> gl_context_ptr_;
        std::chrono::system_clock::time_point a = std::chrono::system_clock::now();
        std::chrono::system_clock::time_point b = std::chrono::system_clock::now();
        // This variable helps for setting up the controls
        SDL_Keycode last_key_pressed_ = 0;
        float sleep_time_ = 16.75f;

        // These bools determine whether certain windows are open
        // We aren't allowed to use static member variables so we use static functions that
        // return static members
        bool rom_loaded_ = false;
        bool rom_paused_ = false;
        bool menu_bar_open_ = true;
        bool window_fpscounter_open_ = false;
        bool window_disassembly_open_ = false;
        bool window_settings_open_ = false;
        bool window_file_browser_open_ = false;
        bool window_about_open_ = false;
        bool window_messagebox_open_ = false;
        bool ipl_changed_ = false;
        std::string messagebox_body_;
        std::string WS_path_;
        TKPShortcut last_shortcut_ = TKPShortcut::NONE;

        // Window drawing functions for ImGui
        void draw_settings(bool* draw);
        void draw_fps_counter(bool* draw);
        void draw_disassembly(bool* draw);
        void draw_file_browser(bool* draw);
        void draw_about(bool* draw);
        void draw_game_background(bool* draw);
        void draw_menu_bar(bool* draw);
        void draw_messagebox(bool* draw);
        void draw_menu_bar_file();
        void draw_menu_bar_file_recent();
        void draw_menu_bar_tools();
        void draw_menu_bar_view();
        void draw_menu_bar_help();
        void draw_tools();
        void open_file_browser(std::string title, const std::vector<std::string>& extensions);
        void open_file_browser_rom();
        void handle_shortcuts();
        void save_recent_files();
        
        // Helper functions
        bool load_image_from_file(const char* filename, TKPImage& out);
        void limit_fps();
        void init_settings_values();
        void init_gameboy_values();
        void init_n64_values();
        bool is_rom_loaded();
        bool is_rom_loaded_and_debugmode();

        // This function deals with scaling the gameboy screen texture without stretching it
        void image_scale(ImVec2& topleft, ImVec2& bottomright, float wi, float hi);

        void load_rom(std::filesystem::path path);
        void load_ipl(std::filesystem::path path);
        std::any get_emu_specific_args(EmuType type);
        void setup_emulator_specific();
        void setup_gameboy_palette();
        void load_loop();
        void load_theme();
        void main_loop();
        void throw_error(const std::runtime_error& ex);
        std::runtime_error generate_exception(std::string func_name, int line_num, std::string description);
	};
}
#endif
