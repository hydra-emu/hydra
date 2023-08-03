#include "mmioviewer.hxx"
#include "emulator_types.hxx"
#include "registered_mmio.hxx"

#include <qgridlayout.h>
#include <QGridLayout>
#include <QLabel>
#include <QListWidgetItem>
#include <QVBoxLayout>

MmioViewer::MmioViewer(hydra::Emulator* emulator, hydra::EmuType type, QWidget* parent)
    : emulator_(emulator), QWidget(parent, Qt::Window)
{
    switch (type)
    {
        case hydra::EmuType::N64:
        {
            initialize_n64();
            break;
        }
        default:
        {
            throw std::runtime_error("Unsupported emulator type");
        }
    }
    initialize_tab_list();
}

void MmioViewer::initialize_n64()
{
    static uint8_t myreg = 0x78;
    components_["Test"] = {};
    components_["Test"]["MyRegister"] =
        RegisteredMmio::Register("name", myreg, "xxxxxxxx", {"Full"});
}

void MmioViewer::initialize_tab_list()
{
    QGridLayout* main_layout = new QGridLayout;
    left_group_box_ = new QGroupBox;
    right_group_box_ = new QGroupBox;
    {
        tab_list_ = new QListWidget;
        QVBoxLayout* left_layout = new QVBoxLayout;
        left_layout->addWidget(tab_list_);
        left_group_box_->setLayout(left_layout);
        left_group_box_->setFixedWidth(200);
        left_group_box_->setMinimumHeight(400);
        connect(tab_list_, SIGNAL(itemSelectionChanged()), this, SLOT(on_tab_change()));
    }
    {
        tab_show_ = new QTabWidget;
        tab_show_->tabBar()->hide();
        QVBoxLayout* layout = new QVBoxLayout;
        layout->setContentsMargins(5, 5, 5, 5);
        layout->addWidget(tab_show_);
        right_group_box_->setLayout(layout);
        right_group_box_->setMinimumWidth(500);
        right_group_box_->setMinimumHeight(400);
    }

    for (auto [component_name, component] : components_)
    {
        QListWidgetItem* name = new QListWidgetItem(component_name.c_str());
        tab_list_->addItem(name);

        QWidget* tab = new QWidget;
        tab_show_->addTab(tab, component_name.c_str());

        QGridLayout* layout = new QGridLayout;
        tab->setLayout(layout);
    }

    main_layout->addWidget(left_group_box_, 0, 0, 1, 1);
    main_layout->addWidget(right_group_box_, 0, 1, 1, 5);
    main_layout->setColumnStretch(0, 0);
    main_layout->setColumnStretch(1, 1);
    setLayout(main_layout);
    setWindowTitle("Mmio Viewer");
    show();
}

void MmioViewer::on_tab_change() {}