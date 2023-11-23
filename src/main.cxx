#include "mainwindow.hxx"
#include <argparse/argparse.h>
#include <filesystem>
#include <log.h>
#include <QApplication>
#include <QSurfaceFormat>
#include <settings.hxx>
#include <update.hxx>

const char* frontend = "qt";

int main_qt(int argc, char* argv[])
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
    ;
}

int version_cb(struct argparse* self, const struct argparse_option* option)
{
    (void)self;
    (void)option;
    std::cout << "hydra version " << HYDRA_VERSION << std::endl;
    return 0;
}

int start_frontend_cb(struct argparse* self, const struct argparse_option* option)
{
    (void)self;
    (void)option;
    std::string frontend_str(frontend);
    if (frontend_str == "qt")
    {
        return main_qt(self->argc, const_cast<char**>(self->argv));
    }
    else
    {
        std::cout << "Unknown frontend: " << frontend << std::endl;
        return 1;
    }
}

int print_settings_cb(struct argparse* self, const struct argparse_option* option)
{
    std::cout << Settings::Print() << std::endl;
    return 0;
}

int main(int argc, char* argv[])
{
    auto settings_path = Settings::GetSavePath() / "settings.json";
    Settings::Open(settings_path);
    Settings::InitCoreInfo();

    if (argc == 1)
    {
        return main_qt(argc, argv);
    }

    static const char* const usages[] = {
        "hydra [args]",
        nullptr,
    };
    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_GROUP("Options"),
        OPT_BOOLEAN('v', "version", nullptr, "show version information", version_cb),
        OPT_STRING('p', "print-settings", nullptr, "print settings", print_settings_cb),
        OPT_STRING('f', "frontend", &frontend, "frontend to use (default: qt)", start_frontend_cb),
        OPT_END(),
    };

    struct argparse argparse;
    argparse_init(&argparse, options, usages, 0);
    argparse_describe(&argparse, "\nThe hydra emulator", nullptr);
    argparse_parse(&argparse, argc, const_cast<const char**>(argv));
    return 0;
}
