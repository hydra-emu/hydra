#ifndef MAINWINDOW_HXX
#define MAINWINDOW_HXX

#include <QMainWindow>
#include <QMenuBar>
#include <QFileDialog>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QLabel>
#include <memory>
#include "../include/emulator_factory.h"
#include "../include/emulator.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

    // Initialization functions
    void create_actions();
    void create_menus();

    // Menu bar actions
    void open_file();

    // Emulation functions
    void pause_emulator();
    void stop_emulator();
    void enable_emulation_actions(bool should);

private slots:
    void redraw_screen();

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QMenu* file_menu_;
    QMenu* emulation_menu_;
    QAction* open_act_;
    QAction* pause_act_;
    QAction* stop_act_;
    QLabel *lbl_;
    QPixmap texture_;
    std::shared_ptr<TKPEmu::Tools::MQBase> message_queue_;
    std::shared_ptr<TKPEmu::Emulator> emulator_;
    std::thread emulator_thread_;
};
#endif // MAINWINDOW_HXX
