#pragma once

#include "hydra/core.hxx"
#include <core_loader.hxx>
#include <filesystem>
#include <memory>
#include <qaction.h>
#include <QWidget>

class QListWidget;

struct CheatMetadata
{
    bool enabled = false;
    std::string name{};
    std::string code{};
    uint32_t handle = hydra::BAD_CHEAT;
};

class CheatsWindow final : public QWidget
{
    Q_OBJECT

public:
    CheatsWindow(std::shared_ptr<hydra::EmulatorWrapper> wrapper,
                 std::shared_ptr<std::vector<CheatMetadata>> metadata,
                 const std::filesystem::path& path, QAction* action, QWidget* parent = nullptr);
    ~CheatsWindow() = default;

private:
    void hideEvent(QHideEvent* event) override
    {
        save_cheats();
        menu_action_->setChecked(false);
        QWidget::hideEvent(event);
    }

    void showEvent(QShowEvent* event) override
    {
        menu_action_->setChecked(true);
        QWidget::showEvent(event);
    }

private:
    QListWidget* cheat_list_;
    std::shared_ptr<hydra::EmulatorWrapper> wrapper_;
    std::shared_ptr<std::vector<CheatMetadata>> metadata_;
    std::filesystem::path cheat_path_;
    QAction* menu_action_;

    void save_cheats();
};
