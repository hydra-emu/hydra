#ifndef MAINWINDOW_HXX
#define MAINWINDOW_HXX

#include <QMainWindow>
#include <QMenuBar>
#include <QFileDialog>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QLabel>
#include "../include/emulator_factory.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    // Initialization functions
    void create_actions();
    void create_menus();

    // Menu bar actions
    void open_file();
    void pause_emulator();

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QMenu* file_menu_;
    QMenu* emulation_menu_;
    QAction* open_act_;
    QAction* pause_act_;
    QLabel *lbl_;
    QPixmap texture_;
};
#endif // MAINWINDOW_HXX
