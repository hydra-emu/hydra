#pragma once

#include "hydra/core.hxx"
#include <corewrapper.hxx>
#include <filesystem>
#include <memory>
#include <qaction.h>
#include <QWidget>

class QListWidget;

class CheatsWindow final : public QWidget
{
    Q_OBJECT

public:
    CheatsWindow(std::shared_ptr<hydra::EmulatorWrapper> wrapper, const std::filesystem::path& path,
                 QAction* action, QWidget* parent = nullptr);
    ~CheatsWindow() = default;

private:
    void hideEvent(QHideEvent* event) override
    {
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
    std::filesystem::path cheat_path_;
    QAction* menu_action_;
};
