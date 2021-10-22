#ifndef TKP_GB_DISPLAY_H
#define TKP_GB_DISPLAY_H
#include <glad/glad.h>
#include <SDL.h>
#include <string>
#include <type_traits>
#include <mutex>
#include <map>
#ifdef _WIN32
#include <windows.h>
#endif
#ifdef linux
#include <unistd.h>
#include <linux/limits.h>
static_assert(false);
#endif
#include "../emulator.h"
#include "../Tools/prettyprinter.h"
#include "../Tools/TKPImage.h"
#include "../ImGui/imgui_impl_sdl.h"
#include "../ImGui/imgui_impl_opengl3.h"
#include "../ImGui/imgui_internal.h"
#include "Applications/imfilebrowser.h"
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
    using AppSettingsType = uint8_t;
    using TKPImage = TKPEmu::Tools::TKPImage;
    struct WindowSettings {
        int window_width = GameboyWidth * 4;
        int window_height = GameboyHeight * 4;
        int minimum_width = GameboyWidth;
        int minimum_height = GameboyHeight;
        int maximum_width = 1920;
        int maximum_height = 1080;
    };
    // Do not change the order of these
    struct AppSettings {
        AppSettingsType vsync = 1;
        AppSettingsType dmg_color0_r = 255;
        AppSettingsType dmg_color0_g = 208;
        AppSettingsType dmg_color0_b = 164;
        AppSettingsType dmg_color1_r = 244;
        AppSettingsType dmg_color1_g = 148;
        AppSettingsType dmg_color1_b = 156;
        AppSettingsType dmg_color2_r = 124;
        AppSettingsType dmg_color2_g = 154;
        AppSettingsType dmg_color2_b = 172;
        AppSettingsType dmg_color3_r = 104;
        AppSettingsType dmg_color3_g = 81;
        AppSettingsType dmg_color3_b = 138;
        // TODO: implement window size and fullscreen and widgets
        AppSettingsType window_size_index = 0;
        AppSettingsType window_fullscreen = 0;
    };
    enum class EmulatorType {
        None,
        Gameboy,
    };
    constexpr auto AppSettingsSize = sizeof(AppSettings) / sizeof(AppSettingsType);
    enum class FileAccess { Read, Write };
	class Display {
    private:
        using PrettyPrinter = TKPEmu::Tools::PrettyPrinter;
        using SDL_GLContextType = std::remove_pointer_t<SDL_GLContext>;
        using PPMessageType = TKPEmu::Tools::PrettyPrinterMessageType;
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
        TKPImage background_image_;
        ImGui::FileBrowser file_browser_;

        std::unique_ptr<Emulator> emulator_;
        std::unique_ptr<SDL_GLContextType, decltype(&SDL_GL_DeleteContext)> gl_context_ptr_;
        std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window_ptr_;

        // These bools determine whether certain windows are open
        // We aren't allowed to use static member variables so we use static functions that
        // return static members
        bool rom_loaded_ = false;
        EmulatorType emulator_type_ = EmulatorType::None;
        bool menu_bar_open_ = true;
        bool window_tracelogger_open_ = false;
        bool window_fpscounter_open_ = false;
        bool window_settings_open_ = false;
        bool window_file_browser_open_ = false;
        bool window_disassembler_open_ = false;

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

        // These two functions deal with scaling the gameboy screen texture without stretching it
        // They are only called upon window resize
        inline void image_scale_no_stretch_top_left(int width, int height, ImVec2& out);
        inline void image_scale_no_stretch_bot_right(int width, int height, ImVec2& out);
        // These two functions save and load user settings stored in the app_settings_ object
        void load_user_settings();
        void save_user_settings();
        // This function chooses fread or fwrite on compile time to minimize code duplication
        template<FileAccess acc>
        void access_user_settings(void* item, size_t size, size_t count, FILE* stream);

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