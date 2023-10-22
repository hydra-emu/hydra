#define NULL 0
#include "server.hxx"
#include <argparse.h>
#include <settings.hxx>
#include <string>

std::vector<core_info> Settings::CoreInfo;
std::map<std::string, std::string> Settings::map_;
std::string Settings::save_path_;
bool Settings::initialized_ = false;
bool Settings::core_info_initialized_ = false;

static const char* const usages[] = {
    "hydra_server [args]",
    nullptr,
};

int main(int argc, const char** argv)
{
    const char* settings_path = nullptr;
    std::string settings_path_string;
    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_GROUP("Options"),
        OPT_STRING('s', "settings_path", &settings_path, "path to settings.json"),
        OPT_END(),
    };
    struct argparse argparse;
    argparse_init(&argparse, options, usages, 0);
    argparse_describe(&argparse, "\nThe headless hydra server", nullptr);
    argc = argparse_parse(&argparse, argc, argv);

    if (!settings_path)
    {
        settings_path_string = Settings::GetSavePath() / "settings.json";
    }
    else
    {
        settings_path_string = std::string(settings_path);
    }

    Settings::Open(settings_path_string);

    hydra::server_t server;

    return 0;
}