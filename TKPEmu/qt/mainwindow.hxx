#ifndef MAINWINDOW_HXX
#define MAINWINDOW_HXX

#include <QMainWindow>
#include <QMenuBar>
#include <QFileDialog>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QLabel>
#include <memory>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "../include/emulator_factory.h"
#include "../include/emulator.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    // Initialization functions
    void create_actions();
    void create_menus();

    // Menu bar actions
    void open_file();

    // Emulation functions
    void pause_emulator();

private slots:
    void redraw_screen();

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QMenu* file_menu_;
    QMenu* emulation_menu_;
    QAction* open_act_;
    QAction* pause_act_;
    QLabel *lbl_;
    QPixmap texture_;
    std::shared_ptr<TKPEmu::Emulator> emulator_;
    std::thread emulator_thread_;
};
#endif // MAINWINDOW_HXX
