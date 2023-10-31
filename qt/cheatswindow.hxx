#pragma once

#include "hydra/core.hxx"
#include <core_loader.hxx>
#include <filesystem>
#include <memory>
#include <QWidget>

class QListWidget;

struct CheatMetadata
{
    bool enabled = true;
    std::string name{};
    std::string code{};
    uint32_t handle = hydra::BAD_CHEAT;
};

class CheatsWindow : public QWidget
{
    Q_OBJECT

public:
    CheatsWindow(std::shared_ptr<hydra::EmulatorWrapper> wrapper, bool& open,
                 const std::string& hash, QWidget* parent = nullptr);
    ~CheatsWindow();

    void Show();
    void Hide();

private:
    void closeEvent(QCloseEvent* event) override;

    bool& open_;
    QListWidget* cheat_list_;
    std::shared_ptr<hydra::EmulatorWrapper> wrapper_;
    std::filesystem::path cheat_path_;

    void save_cheats();
};
