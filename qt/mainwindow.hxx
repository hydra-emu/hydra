#pragma once

#include "screenwidget.hxx"
#include <array>
#include <emulator.hxx>
#include <emulator_factory.hxx>
#include <memory>
#include <QFileDialog>
#include <QLabel>
#include <QMainWindow>
#include <QMenuBar>
#include <QStatusBar>
#include <QVBoxLayout>

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

    // Initialization functions
    void create_actions();
    void create_menus();

    // Menu bar actions
    void open_file();
    void open_file_impl(const std::string& file);
    void open_settings();
    void open_about();
    void open_shaders();
    void open_debugger();
    void open_tracelogger();
    void screenshot();
    void close_tools();

    // Emulation functions
    void pause_emulator();
    void reset_emulator();
    void stop_emulator();
    void enable_emulation_actions(bool should);
    void setup_emulator_specific();
    void empty_screen();

private slots:
    void redraw_screen();

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
    void OpenFile(const std::string& file);
    QMenu* file_menu_;
    QMenu* emulation_menu_;
    QMenu* tools_menu_;
    QMenu* help_menu_;
    QAction* open_act_;
    QAction* pause_act_;
    QAction* reset_act_;
    QAction* about_act_;
    QAction* stop_act_;
    QAction* settings_act_;
    QAction* screenshot_act_;
    QAction* shaders_act_;
    QAction* debugger_act_;
    QAction* tracelogger_act_;
    ScreenWidget* screen_;
    std::shared_ptr<hydra::Emulator> emulator_;
    std::array<QWidget*, 2> emulator_tools_{};
    hydra::EmuType emulator_type_;
    std::thread emulator_thread_;
    bool settings_open_ = false;
    bool about_open_ = false;
    bool shaders_open_ = false;
    bool debugger_open_ = false;
    bool tracelogger_open_ = false;
};
