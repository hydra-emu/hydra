#include "mmioviewer.hxx"
#include "c8/c8_tkpwrapper.hxx"
#include "emulator_types.hxx"
#include "gb/gb_tkpwrapper.hxx"
#include "nes/nes_tkpwrapper.hxx"
#include "registered_mmio.hxx"

#include <QBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidgetItem>
#include <QScrollArea>
#include <QVBoxLayout>

#include <log.hxx>

MmioViewer::MmioViewer(bool& open, hydra::Emulator* emulator, hydra::EmuType type, QWidget* parent)
    : open_(open), emulator_(emulator), QWidget(parent, Qt::Window)
{
    // TODO: thread safety - mutex locking
    switch (type)
    {
        case hydra::EmuType::Gameboy:
        {
            initialize_gb();
            break;
        }
        case hydra::EmuType::NES:
        {
            initialize_nes();
            break;
        }
        case hydra::EmuType::c8:
        {
            initialize_c8();
            break;
        }
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

#define REGISTER(group, name, value, mask, ...) \
    components_[group].push_back(RegisteredMmio::Register(name, value, mask, __VA_ARGS__))
#define FREGISTER(group, name, value, mask) REGISTER(group, name, value, mask, {"Full"})
#define FREGISTER64(group, name, value) \
    FREGISTER(group, name, value,       \
              "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx")

void MmioViewer::initialize_gb()
{
    hydra::Gameboy::Gameboy_TKPWrapper* emulator =
        dynamic_cast<hydra::Gameboy::Gameboy_TKPWrapper*>(emulator_);
    FREGISTER("CPU", "A", emulator->cpu_.A, "xxxxxxxx");
    FREGISTER("CPU", "B", emulator->cpu_.B, "xxxxxxxx");
    FREGISTER("CPU", "C", emulator->cpu_.C, "xxxxxxxx");
    FREGISTER("CPU", "D", emulator->cpu_.D, "xxxxxxxx");
    FREGISTER("CPU", "E", emulator->cpu_.E, "xxxxxxxx");
    FREGISTER("CPU", "H", emulator->cpu_.H, "xxxxxxxx");
    FREGISTER("CPU", "L", emulator->cpu_.L, "xxxxxxxx");
    FREGISTER("CPU", "F", emulator->cpu_.F, "xxxxxxxx");
    FREGISTER("CPU", "PC", emulator->cpu_.PC, "xxxxxxxxxxxxxxxx");
    FREGISTER("CPU", "SP", emulator->cpu_.SP, "xxxxxxxxxxxxxxxx");
    FREGISTER("CPU", "IF", emulator->cpu_.IF, "xxxxxxxx");
    FREGISTER("CPU", "IE", emulator->cpu_.IE, "xxxxxxxx");

    REGISTER("PPU", "LCDC", emulator->ppu_.LCDC, "x'x'x'x'x'x'x'x",
             {"LCD/PPU enable", "Window tile map area", "Window enable",
              "BG & window tile data area", "BG tile map area", "OBJ size", "OBJ enable",
              "BG enable"});
    FREGISTER("PPU", "STAT", emulator->ppu_.STAT, "xxxxxxxx");
    FREGISTER("PPU", "SCY", emulator->ppu_.SCY, "xxxxxxxx");
    FREGISTER("PPU", "SCX", emulator->ppu_.SCX, "xxxxxxxx");
    FREGISTER("PPU", "LY", emulator->ppu_.LY, "xxxxxxxx");
    FREGISTER("PPU", "LYC", emulator->ppu_.LYC, "xxxxxxxx");
    FREGISTER("PPU", "WX", emulator->ppu_.WX, "xxxxxxxx");
    FREGISTER("PPU", "WY", emulator->ppu_.WY, "xxxxxxxx");
}

void MmioViewer::initialize_nes()
{
    // hydra::NES::NES_TKPWrapper* emulator = dynamic_cast<hydra::NES::NES_TKPWrapper*>(emulator_);
}

void MmioViewer::initialize_c8()
{
    // hydra::c8::Chip8_TKPWrapper* emulator =
    // dynamic_cast<hydra::c8::Chip8_TKPWrapper*>(emulator_);
}

void MmioViewer::initialize_n64()
{
    // hydra::N64::N64_TKPWrapper* emulator = dynamic_cast<hydra::N64::N64_TKPWrapper*>(emulator_);
    // components_["CPU"] = {};
    // components_["RSP"] = {};
    // components_["RDP"] = {};
    // components_["VI"] = {};
    // components_["AI"] = {};
    // components_["PI"] = {};
    // for (size_t i = 0; i < 32; i++)
    // {
    //     FREGISTER64("CPU", "r" + std::to_string(i), emulator->n64_impl_.cpu_.gpr_regs_[i].UD);
    // }
}

#undef REGISTER
#undef FREGISTER
#undef FREGISTER64

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

    for (auto& [component_name, component] : components_)
    {
        QListWidgetItem* name = new QListWidgetItem(component_name.c_str());
        tab_list_->addItem(name);

        QWidget* tab = new QWidget;
        tab_show_->addTab(tab, component_name.c_str());

        QVBoxLayout* layout = new QVBoxLayout;
        tab->setLayout(layout);

        QScrollArea* scroll_area = new QScrollArea;
        layout->addWidget(scroll_area);

        QWidget* parent_widget = new QWidget;
        parent_widget->setLayout(new QVBoxLayout(parent_widget));
        parent_widget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        scroll_area->setBackgroundRole(QPalette::Dark);
        scroll_area->setWidget(parent_widget);
        scroll_area->setWidgetResizable(true);
        scroll_area->setFrameShape(QFrame::NoFrame);
        scroll_area->setFrameShadow(QFrame::Plain);

        for (auto& mmiowrapper : component)
        {
            QWidget* item = create_item(mmiowrapper);
            parent_widget->layout()->addWidget(item);
        }
    }

    main_layout->addWidget(left_group_box_, 0, 0, 1, 1);
    main_layout->addWidget(right_group_box_, 0, 1, 1, 5);
    main_layout->setColumnStretch(0, 0);
    main_layout->setColumnStretch(1, 1);
    setLayout(main_layout);
    setWindowTitle("Mmio Viewer");
    show();
}

void MmioViewer::on_tab_change()
{
    tab_show_->setCurrentIndex(tab_list_->currentRow());
}

QWidget* MmioViewer::create_item(RegisteredMmio::MmioWrapper& mmiowrapper)
{
    QWidget* item = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout;
    item->setLayout(layout);

    layout->addWidget(new QLabel((mmiowrapper.GetName() + ":").c_str()));

    if (mmiowrapper.GetRangeCount() == 1)
    {
        QLineEdit* line_edit = new QLineEdit;
        line_edit->setReadOnly(true);
        line_edit->setText(QString::number(mmiowrapper.GetValue()));
        layout->addWidget(line_edit);
    }
    else
    {
    }

    return item;
}