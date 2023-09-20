#include "settingswindow.hxx"
#include "keypicker.hxx"
#include <emulator_types.hxx>
#include <QCheckBox>
#include <QFileDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <settings.hxx>

SettingsWindow::SettingsWindow(bool& open, std::function<void(int)> volume_callback,
                               QWidget* parent)
    : open_(open), volume_callback_(volume_callback), QWidget(parent, Qt::Window)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("Settings");
    QGridLayout* main_layout = new QGridLayout;
    left_group_box_ = new QGroupBox;
    right_group_box_ = new QGroupBox;
    {
        tab_list_ = new QListWidget;
        QListWidgetItem* audio =
            new QListWidgetItem(QPixmap(":/images/sound.png"), QString("Audio"));
        QListWidgetItem* input =
            new QListWidgetItem(QPixmap(":/images/input.png"), QString("Input"));
        QListWidgetItem* n64 =
            new QListWidgetItem(QPixmap(":/images/n64.png"), QString("Nintendo 64"));
        tab_list_->addItem(audio);
        tab_list_->addItem(input);
        tab_list_->addItem(n64);
        connect(tab_list_, SIGNAL(itemSelectionChanged()), this, SLOT(on_tab_change()));
        QVBoxLayout* layout = new QVBoxLayout;
        layout->addWidget(tab_list_);
        left_group_box_->setLayout(layout);
        left_group_box_->setFixedWidth(200);
        left_group_box_->setMinimumHeight(400);
    }
    {
        tab_show_ = new QTabWidget;
        tab_show_->tabBar()->hide();
        create_tabs();
        QVBoxLayout* layout = new QVBoxLayout;
        layout->setContentsMargins(5, 5, 5, 5);
        layout->addWidget(tab_show_);
        right_group_box_->setLayout(layout);
        right_group_box_->setMinimumWidth(500);
        right_group_box_->setMinimumHeight(400);
    }
    main_layout->addWidget(left_group_box_, 0, 0, 1, 1);
    main_layout->addWidget(right_group_box_, 0, 1, 1, 5);
    main_layout->setColumnStretch(0, 0);
    main_layout->setColumnStretch(1, 1);
    setLayout(main_layout);
    show();
    open_ = true;
}

SettingsWindow::~SettingsWindow()
{
    open_ = false;
}

void SettingsWindow::create_tabs()
{
    {
        QGridLayout* audio_layout = new QGridLayout;
        audio_layout->setAlignment(Qt::AlignTop);
        QWidget* audio_tab = new QWidget;
        audio_tab->setLayout(audio_layout);
        tab_show_->addTab(audio_tab, "Audio");
        audio_layout->addWidget(new QLabel("Master volume:"), 0, 0);
        QSlider* audio_slider = new QSlider(Qt::Horizontal);
        audio_slider->setRange(0, 100);
        std::string volume_str = Settings::Get("master_volume");
        if (!volume_str.empty())
        {
            int volume = std::stoi(volume_str);
            audio_slider->setValue(volume);
        }
        else
        {
            audio_slider->setValue(100);
        }
        connect(audio_slider, &QSlider::valueChanged, this, [this](int value) {
            Settings::Set("master_volume", std::to_string(value));
            volume_callback_(value);
        });
        audio_layout->addWidget(audio_slider, 0, 1);
    }
    {
        KeyPickerPage* key_picker = new KeyPickerPage(this);
        tab_show_->addTab(key_picker, "Input");
    }
    {
        QGridLayout* n64_layout = new QGridLayout;
        n64_layout->setColumnStretch(1, 3);
        n64_layout->setAlignment(Qt::AlignTop);
        add_filepicker(n64_layout, "IPL path", "n64_ipl_path", "Binary files (*.bin)", 0, 0);
        QWidget* n64_tab = new QWidget;
        n64_tab->setLayout(n64_layout);
        tab_show_->addTab(n64_tab, "N64");
    }
}

void SettingsWindow::on_open_file_click(const std::string& setting, const std::string& extension)
{
    auto path = QFileDialog::getOpenFileName(this, tr("Open IPL"), "", extension.c_str(), nullptr,
                                             QFileDialog::ReadOnly);
    if (!path.isEmpty())
    {
        Settings::Set(setting, path.toStdString());
        ipl_path_->setText(path);
    }
}

void SettingsWindow::add_filepicker(QGridLayout* layout, const std::string& name,
                                    const std::string& setting, const std::string& extension,
                                    int row, int column)
{
    auto path = Settings::Get(setting);
    QLineEdit* edit = new QLineEdit;
    edit->setReadOnly(true);
    edit->setText(path.c_str());
    QPushButton* button = new QPushButton;
    button->setFixedWidth(50);
    button->setIcon(QIcon(":/images/open.png"));
    connect(button, &QPushButton::clicked, this,
            std::bind(&SettingsWindow::on_open_file_click, this, setting, extension));
    layout->addWidget(new QLabel(QString::fromStdString(name + ": ")), row, column);
    layout->addWidget(edit, row, column + 1);
    layout->addWidget(button, row, column + 2);
}

void SettingsWindow::on_tab_change()
{
    tab_show_->setCurrentIndex(tab_list_->currentRow());
}