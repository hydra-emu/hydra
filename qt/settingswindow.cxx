#include "settingswindow.hxx"
#include "keypicker.hxx"
#include <common/compatibility.hxx>
#include <fmt/format.h>
#include <QCheckBox>
#include <QFileDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <settings.hxx>
#include <ui_common.hxx>

SettingsWindow::SettingsWindow(bool& open, std::function<void(int)> volume_callback,
                               QWidget* parent)
    : open_(open), volume_callback_(volume_callback), QWidget(parent, Qt::Window)
{
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("Settings");
    QGridLayout* main_layout = new QGridLayout;
    left_group_box_ = new QGroupBox;
    right_group_box_ = new QGroupBox;
    {
        tab_list_ = new QListWidget;
#define add_item(var_name, name, image)                                                         \
    QListWidgetItem* var_name = new QListWidgetItem(QPixmap(":/images/" image), QString(name)); \
    tab_list_->addItem(var_name)

        add_item(general, "General", "support.png");
        add_item(cores, "Cores", "core.png");
        add_item(audio, "Audio", "sound.png");
        add_item(input, "Input", "input.png");
        add_item(n64, "Nintendo 64", "n64.png");
#undef add_item
        // clang-format off
        tab_list_->setStyleSheet(R"(
            QListWidget::item:selected
            {
                background: rgb(180,85,85);
            }
        )");
        // clang-format on
        tab_list_->setCurrentRow(0);
        tab_list_->setDragEnabled(false);
        tab_list_->setSelectionMode(QAbstractItemView::SingleSelection);
        connect(tab_list_, SIGNAL(currentRowChanged(int)), this, SLOT(on_tab_change(int)));
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
        layout->setContentsMargins(0, 0, 0, 0);
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
        QGridLayout* general_layout = new QGridLayout;
        general_layout->setAlignment(Qt::AlignTop);
        QWidget* general_tab = new QWidget;
        general_tab->setLayout(general_layout);
        tab_show_->addTab(general_tab, "General");
        add_filepicker(general_layout, "Screenshot directory", "screenshot_path", "", 0, 0, true);
        QCheckBox* use_cwd = new QCheckBox("...or use current working directory");
        connect(use_cwd, &QCheckBox::stateChanged, this, [general_layout](int state) {
            static std::string last_path = Settings::Get("screenshot_path");
            QLineEdit* path_edit = (QLineEdit*)(general_layout->itemAtPosition(0, 1)->widget());
            if (state == Qt::Checked)
            {
                last_path = Settings::Get("screenshot_path");
                Settings::Set("screenshot_path", "");
                general_layout->itemAtPosition(0, 1)->widget()->setEnabled(false);
                general_layout->itemAtPosition(0, 2)->widget()->setEnabled(false);
                path_edit->setText(std::filesystem::current_path().string().c_str());
            }
            else
            {
                if (Settings::Get("screenshot_path").empty())
                {
                    Settings::Set("screenshot_path", last_path);
                    path_edit->setText(last_path.c_str());
                }
                general_layout->itemAtPosition(0, 1)->widget()->setEnabled(true);
                general_layout->itemAtPosition(0, 2)->widget()->setEnabled(true);
            }
        });
        use_cwd->setCheckState(Settings::Get("screenshot_path").empty() ? Qt::Checked
                                                                        : Qt::Unchecked);
        general_layout->addWidget(use_cwd, 1, 0, 1, 2);
    }
    {
        Settings::InitCoreInfo();
        QGridLayout* cores_layout = new QGridLayout;
        cores_layout->setAlignment(Qt::AlignTop);
        QWidget* cores_tab = new QWidget;
        cores_tab->setLayout(cores_layout);
        tab_show_->addTab(cores_tab, "Cores");
        if (Settings::Get("core_path").empty())
        {
            Settings::Set("core_path", (std::filesystem::current_path() / "cores").string());
        }
        QListWidget* core_list = new QListWidget;
        core_list->setDragEnabled(false);
        QFile file(":/core.html");
        file.open(QIODevice::ReadOnly);
        QString html = file.readAll();
        file.close();
        for (size_t i = 0; i < Settings::CoreInfo.size(); i++)
        {
            QListWidgetItem* item = new QListWidgetItem;
            core_list->addItem(item);
            QLabel* label = new QLabel;
            label->setTextFormat(Qt::RichText);
            label->setTextInteractionFlags(Qt::TextBrowserInteraction);
            label->setOpenExternalLinks(true);
            const auto& core = Settings::CoreInfo[i];
            label->setText(fmt::format(fmt::runtime(html.toStdString()), core.core_name,
                                       core.version, core.system_name, core.license,
                                       core.description, core.author, core.url)
                               .c_str());
            item->setSizeHint(label->sizeHint());
            core_list->setItemWidget(item, label);
        }
        cores_layout->addWidget(core_list, 0, 0, 1, 0);
        add_filepicker(cores_layout, "Core directory", "core_path", "", 1, 0, true);
    }
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
        key_picker_ = new KeyPickerPage;
        tab_show_->addTab(key_picker_, "Input");
    }
    {
        QGridLayout* n64_layout = new QGridLayout;
        n64_layout->setColumnStretch(1, 3);
        n64_layout->setAlignment(Qt::AlignTop);
        add_filepicker(n64_layout, "IPL path", "IPL", "Binary files (*.bin)", 0, 0);
        QFrame* separator = new QFrame(this);
        separator->setLineWidth(1);
        separator->setMidLineWidth(1);
        separator->setFrameShape(QFrame::HLine);
        separator->setPalette(QPalette(QColor(0, 0, 0)));
        n64_layout->addWidget(separator, 1, 0, 1, 0);
        for (int i = 1; i <= 4; i++)
        {
            n64_layout->addWidget(new QLabel("Controller port " + QString::number(i) + ":"), i + 1,
                                  0);
            QCheckBox* active = new QCheckBox("Active");
            std::string is_active =
                Settings::Get("n64_controller_" + std::to_string(i) + "_active");
            active->setChecked(is_active == "true");
            connect(active, &QCheckBox::stateChanged, this, [this, i](int state) {
                Settings::Set("n64_controller_" + std::to_string(i) + "_active",
                              state == Qt::Checked ? "true" : "false");
            });
            if (is_active.empty() && i == 1)
            {
                active->setChecked(true);
            }
            n64_layout->addWidget(new QComboBox, i + 1, 1);
            n64_layout->addWidget(active, i + 1, 2);
        }
        QWidget* n64_tab = new QWidget;
        n64_tab->setLayout(n64_layout);
        tab_show_->addTab(n64_tab, "N64");
    }
}

void SettingsWindow::on_open_file_click(QLineEdit* edit, const std::string& name,
                                        const std::string& setting, const std::string& extension)
{
    auto path = QFileDialog::getOpenFileName(this, name.c_str(), "", extension.c_str(), nullptr,
                                             QFileDialog::ReadOnly);
    if (!path.isEmpty())
    {
        Settings::Set(setting, path.toStdString());
        edit->setText(path);
    }
}

void SettingsWindow::on_open_dir_click(QLineEdit* edit, const std::string& name,
                                       const std::string& setting)
{
    auto path = QFileDialog::getExistingDirectory(this, name.c_str(), "", QFileDialog::ReadOnly);

    if (!path.isEmpty())
    {
        Settings::Set(setting, path.toStdString());
        edit->setText(path);
    }
}

void SettingsWindow::add_filepicker(QGridLayout* layout, const std::string& name,
                                    const std::string& setting, const std::string& extension,
                                    int row, int column, bool dir)
{
    auto path = Settings::Get(setting);
    QLineEdit* edit = new QLineEdit;
    edit->setReadOnly(true);
    edit->setText(path.c_str());
    QPushButton* button = new QPushButton;
    button->setFixedWidth(50);
    button->setIcon(QIcon(":/images/open.png"));
    if (!dir)
    {
        connect(
            button, &QPushButton::clicked, this,
            std::bind(&SettingsWindow::on_open_file_click, this, edit, name, setting, extension));
    }
    else
    {
        connect(button, &QPushButton::clicked, this,
                std::bind(&SettingsWindow::on_open_dir_click, this, edit, name, setting));
    }
    layout->addWidget(new QLabel(QString::fromStdString(name + ": ")), row, column);
    layout->addWidget(edit, row, column + 1);
    layout->addWidget(button, row, column + 2);
}

void SettingsWindow::on_tab_change(int tab)
{
    tab_show_->setCurrentIndex(tab);
}

void SettingsWindow::keyPressEvent(QKeyEvent* event)
{
    key_picker_->KeyPressed(event);
}
