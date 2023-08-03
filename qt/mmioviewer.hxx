#pragma once

#include <emulator.hxx>
#include <QGroupBox>
#include <QListWidget>
#include <QTabWidget>
#include <QWidget>
#include <registered_mmio.hxx>

class MmioViewer : public QWidget
{
    Q_OBJECT

public:
    MmioViewer(hydra::Emulator* emulator, hydra::EmuType type, QWidget* parent = nullptr);

private slots:
    void on_tab_change();

private:
    QListWidget* tab_list_;
    QTabWidget* tab_show_;
    QGroupBox *right_group_box_, *left_group_box_;

    hydra::Emulator* emulator_;
    RegisteredMmio::ConsoleComponents components_;

    void initialize_n64();
    void initialize_tab_list();
};