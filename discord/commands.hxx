#pragma once

#include <dpp.h>
#include <vector>

struct Commands
{
    static const std::vector<dpp::slashcommand>& Get(uint64_t appid)
    {
        static std::vector<dpp::slashcommand> commands = {
            dpp::slashcommand("ping", "Ping the bot", appid),
            dpp::slashcommand("status", "Print the emulator status", appid),
            dpp::slashcommand("screen", "Show the game screen", appid)};
        return commands;
    }
};