#include <argparse/argparse.h>
#include <bot.hxx>
#include <filesystem>
#include <iostream>
#ifdef HYDRA_FRONTEND_QT
#include "mainwindow.hxx"
#include <QApplication>
#include <QSurfaceFormat>
#endif
#include <settings.hxx>

#ifdef HYDRA_WEB
#include <emscripten.h>
#endif

// clang-format off

const char* options =
// options.txt is generated by the convert_man_to_help.sh script by
// converting the hydra manpage OPTIONS section to a string literal
#include <options.txt>
;

// clang-format on

// Configurable options
const char* frontend = "imgui";
const char* rom_path = nullptr;
const char* core_name = nullptr;

int imgui_main(int argc, char* argv[]);

#ifdef HYDRA_FRONTEND_QT
int qt_main(int argc, char* argv[])
{
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setMajorVersion(3);
    format.setMinorVersion(3);
    format.setSwapInterval(0);

    QSurfaceFormat::setDefaultFormat(format);
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    if (argc > 1)
    {
        w.OpenFile(argv[1]);
    }

    return a.exec();
}
#endif

int version_cb(struct argparse*, const struct argparse_option*)
{
    std::cout << "hydra version " << HYDRA_VERSION << std::endl;
    return 0;
}

int start_frontend_cb(struct argparse* self, const struct argparse_option*)
{
    std::string frontend_str(frontend);
    if (frontend_str == "qt")
    {
#ifdef HYDRA_FRONTEND_QT
        return qt_main(self->argc, const_cast<char**>(self->argv));
#else
        std::cout << "This build of hydra does not support the qt frontend" << std::endl;
        return 1;
#endif
    }
    else
    {
        std::cout << "Unknown frontend: " << frontend << std::endl;
        return 1;
    }
}

int print_settings_cb(struct argparse*, const struct argparse_option*)
{
    std::cout << Settings::Print() << std::endl;
    return 0;
}

int help_cb(struct argparse* self, const struct argparse_option* option)
{
    version_cb(self, option);
    std::cout << "A multi-system emulator frontend\n\n" << options << std::endl;
    return 0;
}

int list_cores_cb(struct argparse*, const struct argparse_option*)
{
    Settings::InitCoreInfo();
    for (auto& info : Settings::GetCoreInfo())
    {
        std::string filename = std::filesystem::path(info.path).filename().string();
        std::cout << fmt::format("{} - {}\n", filename, info.core_name);
    }
    std::cout << std::flush;
    return 0;
}

int bot_main_cb(struct argparse*, const struct argparse_option*)
{
#ifdef HYDRA_DISCORD_BOT
    return bot_main();
#else
    hydra::log("This build of hydra does not support the discord bot");
    return 1;
#endif
}

#if defined(HYDRA_WEB)
// Setup the offline file system
EM_JS(void, em_init_fs, (),{
        FS.mkdir('/hydra');
        // Then mount with IDBFS type
        FS.mount(IDBFS, {}, '/hydra');
        FS.mkdir('/hydra/cores');
        FS.mount(IDBFS, {}, '/hydra/cores');
        FS.mkdir('/hydra/cache');
        FS.mount(IDBFS, {}, '/hydra/cache');
        // Then sync
        FS.syncfs(true, function (err) {
            Module.ccall('main_impl');
        });
  });
#endif

extern "C" int main_impl(int argc, char* argv[])
{
    printf("hydra version %s\n", HYDRA_VERSION);
    auto settings_path = Settings::GetSavePath() / "settings.json";
    Settings::Open(settings_path);
    Settings::InitCoreInfo();

#ifdef HYDRA_WEB
    return imgui_main(argc, argv);
#endif

    if (argc == 1)
    {
        return imgui_main(argc, argv);
    }

    static const char* const usages[] = {
        "hydra [args]",
        nullptr,
    };
    struct argparse_option options[] = {
        OPT_BOOLEAN('h', "help", NULL, nullptr, help_cb, 0, OPT_NONEG),
        OPT_GROUP("Options"),
        OPT_BOOLEAN('b', "discord-bot", nullptr, nullptr, bot_main_cb),
        OPT_STRING('o', "open-file", &rom_path, nullptr, nullptr),
        OPT_STRING('c', "use-core", &core_name, nullptr, nullptr),
        OPT_BOOLEAN('l', "list-cores", nullptr, nullptr, list_cores_cb),
        OPT_BOOLEAN('v', "version", nullptr, nullptr, version_cb),
        OPT_STRING('p', "print-settings", nullptr, nullptr, print_settings_cb),
        OPT_STRING('f', "frontend", &frontend, nullptr, start_frontend_cb),
        OPT_END(),
    };

    struct argparse argparse;
    argparse_init(&argparse, options, usages, 0);
    argparse_describe(&argparse, "\nThe hydra emulator", nullptr);
    argparse_parse(&argparse, argc, const_cast<const char**>(argv));
    return 0;
}

int main(int argc, char* argv[])
{
#ifdef HYDRA_WEB
    em_init_fs();
#else
    return main_impl(argc, argv);
#endif
}