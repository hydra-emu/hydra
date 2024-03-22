#pragma once

#include <filesystem>
#include <QMainWindow>

namespace hydra::qt
{
    class MainWindow : public QMainWindow
    {
        Q_OBJECT

    public:
        MainWindow(QWidget* parent = nullptr);
        ~MainWindow();

    private:
        void loadFiles(const std::vector<std::string>& files);
        void dropEvent(QDropEvent* event) override;
        void resizeEvent(QResizeEvent* event) override;
    };
} // namespace hydra::qt