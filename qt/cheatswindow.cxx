#include "cheatswindow.hxx"

#include "core_loader.hxx"
#include "json.hpp"
#include "settings.hxx"

#include <QListWidget>
#include <QVBoxLayout>

static std::vector<uint8_t> cheat_to_bytes(const std::string& cheat)
{
    std::vector<uint8_t> bytes;
    for (size_t i = 0; i < cheat.size(); i += 2)
    {
        std::string hex = cheat.substr(i, 2);
        bytes.push_back((uint8_t)std::stoul(hex, nullptr, 16));
    }
    return bytes;
}

CheatsWindow::CheatsWindow(std::shared_ptr<hydra::EmulatorWrapper> wrapper, bool& open,
                           const std::string& hash, QWidget* parent)
    : QWidget(parent), open_(open), wrapper_(wrapper)
{
    bool just_created = false;
    if (!std::filesystem::create_directories(Settings::GetSavePath() / "cheats"))
    {
        if (!std::filesystem::exists(Settings::GetSavePath() / "cheats"))
        {
            printf("Failed to create cheats directory\n");
            return;
        }
    }
    else
    {
        just_created = true;
    }

    if (!wrapper_->shell->hasInterface(hydra::InterfaceType::ICheat))
    {
        printf("Emulator does not have cheat interface, this dialog shouldn't have been opened?\n");
        return;
    }

    auto cheat_interface = wrapper_->shell->asICheat();
    cheat_path_ = Settings::GetSavePath() / "cheats" / (hash + ".json");

    if (!just_created)
    {
        // Check if this game already has saved cheats
        if (std::filesystem::exists(cheat_path_))
        {
            // Load the cheats
            std::ifstream cheat_file(cheat_path_);
            nlohmann::json cheat_json;
            cheat_file >> cheat_json;
            for (auto& cheat : cheat_json)
            {
                CheatMetadata cheat_metadata;
                cheat_metadata.enabled = cheat["enabled"] == "true";
                cheat_metadata.name = cheat["name"];
                cheat_metadata.description = cheat["description"];
                cheat_metadata.code = cheat["code"];
                std::vector<uint8_t> bytes = cheat_to_bytes(cheat_metadata.code);
                cheat_metadata.handle = cheat_interface->addCheat(bytes.data(), bytes.size());
                if (cheat_metadata.handle != hydra::BAD_CHEAT)
                {
                    cheats_.push_back(cheat_metadata);
                }
                else
                {
                    printf("Failed to add cheat %s\n", cheat_metadata.name.c_str());
                }
            }
        }
    }

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(6, 6, 6, 6);
    setLayout(layout);

    QListWidget* list = new QListWidget;
    list->addItem("Test cheat");
    layout->addWidget(list);

    open_ = true;
}

CheatsWindow::~CheatsWindow()
{
    // Save the cheats
    nlohmann::json cheat_json;
    for (auto& cheat : cheats_)
    {
        cheat_json.push_back({{"enabled", cheat.enabled ? "true" : "false"},
                              {"name", cheat.name},
                              {"description", cheat.description},
                              {"code", cheat.code}});
    }
    std::ofstream cheat_file(cheat_path_);
    cheat_file << cheat_json.dump(4);
}
