#include "commands.hxx"
#include "corewrapper.hxx"
#include "hydra/core.hxx"
#include "message.h"
#include "unicode_emoji.h"
#include <compatibility.hxx>
#include <dpp.h>
#include <filesystem>
#include <settings.hxx>
#include <string>

constexpr uint32_t operator""_hash(const char* str, size_t)
{
    return hydra::str_hash(str);
}

namespace fs = std::filesystem;

std::shared_ptr<hydra::EmulatorWrapper> emulator;

int bot_main()
{
    std::string token = Settings::Get("bot_token");

    if (token.empty())
    {
        token = std::string(std::getenv("BOT_TOKEN"));
        Settings::Set("bot_token", token);
    }

    if (token.empty())
    {
        std::cerr << "No bot token found. Please set the BOT_TOKEN environment variable."
                  << std::endl;
        return 1;
    }

    std::string core_path = Settings::Get("core_path");

    if (core_path.empty())
    {
        core_path = std::string(std::getenv("CORE_PATH"));
        Settings::Set("core_path", core_path);
    }

    if (core_path.empty())
    {
        std::cerr << "No core path found. Please set the CORE_PATH environment variable."
                  << std::endl;
        return 1;
    }

    emulator = hydra::EmulatorFactory::Create(fs::path(core_path) / "libAlber.so");

    dpp::cluster bot(token);

    bot.on_log(dpp::utility::cout_logger());

    bot.on_ready([&bot](const dpp::ready_t& event) {
        if (dpp::run_once<struct register_bot_commands>())
        {
            std::cout << "Registering commands" << std::endl;
            bot.global_bulk_command_create(
                Commands::Get(bot.me.id), [](const dpp::confirmation_callback_t& callback) {
                    if (callback.is_error())
                    {
                        std::cout << "Failed to register commands: " << callback.http_info.body
                                  << std::endl;
                    }
                    else
                    {
                        std::cout << "Successfully registered commands" << std::endl;
                    }
                });
        }
    });

    bot.on_slashcommand([](const dpp::slashcommand_t& event) {
        switch (hydra::str_hash(event.command.get_command_name().c_str()))
        {
            case "ping"_hash:
            {
                event.reply("Pong!");
                break;
            }
            case "status"_hash:
            {
                dpp::embed embed =
                    dpp::embed()
                        .set_title(emulator->GetInfo(hydra::InfoType::CoreName))
                        .set_url(emulator->GetInfo(hydra::InfoType::Website))
                        .set_description(emulator->GetInfo(hydra::InfoType::Description))
                        .set_author(emulator->GetInfo(hydra::InfoType::Author),
                                    "https://github.com/hydra-emu/hydra", "attachment://icon.png")
                        .set_thumbnail("attachment://icon.png");
                dpp::message msg = dpp::message(event.command.channel_id, embed);
                if (emulator->GetIcon().size() > 0)
                {
                    msg.add_file(
                        "icon.png",
                        std::string(emulator->GetIcon().begin(), emulator->GetIcon().end()),
                        "image/png");
                }
                event.reply(msg);
                break;
            }
            case "screen"_hash:
            {
                dpp::message msg(event.command.channel_id, "");
                msg.add_component(dpp::component()
                                      .add_component(dpp::component()
                                                         .set_label("L")
                                                         .set_type(dpp::cot_button)
                                                         .set_style(dpp::cos_secondary)
                                                         .set_id("l_button"))
                                      .add_component(dpp::component()
                                                         .set_label("↑")
                                                         .set_type(dpp::cot_button)
                                                         .set_style(dpp::cos_primary)
                                                         .set_id("up_button"))
                                      .add_component(dpp::component()
                                                         .set_label("R")
                                                         .set_type(dpp::cot_button)
                                                         .set_style(dpp::cos_secondary)
                                                         .set_id("r_button"))
                                      .add_component(dpp::component()
                                                         .set_label("A")
                                                         .set_type(dpp::cot_button)
                                                         .set_style(dpp::cos_success)
                                                         .set_id("a_button"))
                                      .add_component(dpp::component()
                                                         .set_label("X")
                                                         .set_type(dpp::cot_button)
                                                         .set_style(dpp::cos_secondary)
                                                         .set_id("x_button")))
                    .add_component(dpp::component()
                                       .add_component(dpp::component()
                                                          .set_label("←")
                                                          .set_type(dpp::cot_button)
                                                          .set_style(dpp::cos_primary)
                                                          .set_id("left_button"))
                                       .add_component(dpp::component()
                                                          .set_label("↓")
                                                          .set_type(dpp::cot_button)
                                                          .set_style(dpp::cos_primary)
                                                          .set_id("down_button"))
                                       .add_component(dpp::component()
                                                          .set_label("→")
                                                          .set_type(dpp::cot_button)
                                                          .set_style(dpp::cos_primary)
                                                          .set_id("right_button"))
                                       .add_component(dpp::component()
                                                          .set_label("Y")
                                                          .set_type(dpp::cot_button)
                                                          .set_style(dpp::cos_secondary)
                                                          .set_id("y_button"))
                                       .add_component(dpp::component()
                                                          .set_label("B")
                                                          .set_type(dpp::cot_button)
                                                          .set_style(dpp::cos_danger)
                                                          .set_id("b_button")))
                    .add_component(dpp::component()
                                       .add_component(dpp::component()
                                                          .set_label("👆")
                                                          .set_type(dpp::cot_button)
                                                          .set_style(dpp::cos_secondary)
                                                          .set_id("touch_button"))
                                       .add_component(dpp::component()
                                                          .set_label("\u200b")
                                                          .set_type(dpp::cot_button)
                                                          .set_style(dpp::cos_secondary)
                                                          .set_id("spacer2")
                                                          .set_disabled(true))
                                       .add_component(dpp::component()
                                                          .set_label("\u200b")
                                                          .set_type(dpp::cot_button)
                                                          .set_style(dpp::cos_secondary)
                                                          .set_id("spacer1")
                                                          .set_disabled(true))
                                       .add_component(dpp::component()
                                                          .set_label("➕")
                                                          .set_type(dpp::cot_button)
                                                          .set_style(dpp::cos_secondary)
                                                          .set_id("start_button"))
                                       .add_component(dpp::component()
                                                          .set_label("➖")
                                                          .set_type(dpp::cot_button)
                                                          .set_style(dpp::cos_secondary)
                                                          .set_id("select_button")));
                event.reply(msg);
                break;
            }
        }
    });

    bot.start(dpp::st_wait);
    return 0;
}