#include "settingswindow.hxx"
#include "keypicker.hxx"
#include <compatibility.hxx>
#include <fmt/format.h>
#include <QCheckBox>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <settings.hxx>

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

#define add_item(var_name, name, image)                                            \
    QListWidgetItem* var_name =                                                    \
        new QListWidgetItem(QPixmap(QString(":/images/") + image), QString(name)); \
    tab_list_->addItem(var_name)

        add_item(general, "General", "support.png");
        add_item(cores, "Cores", "core.png");
        add_item(audio, "Audio", "sound.png");
        add_item(input, "Input", "input.png");

        if (!std::filesystem::create_directories(Settings::GetSavePath() / "cache"))
        {
            if (!std::filesystem::exists(Settings::GetSavePath() / "cache"))
            {
                QMessageBox::critical(this, "Error", "Cannot create cache directory");
                return;
            }
        }

        for (size_t i = 0; i < Settings::CoreInfo.size(); i++)
        {
            std::filesystem::path path = Settings::GetSavePath() / "cache" /
                                         std::string(Settings::CoreInfo[i].core_name + ".png");
            if (QFile::exists(path))
            {
                QListWidgetItem* item = new QListWidgetItem(
                    QPixmap((path).string().c_str()), Settings::CoreInfo[i].core_name.c_str());
                tab_list_->addItem(item);
            }
            else
            {
                auto wrapper = hydra::EmulatorFactory::Create(Settings::CoreInfo[i].path);
                QString cache_path = path.string().c_str();
                if (wrapper->GetInfo(hydra::InfoType::IconData))
                {
                    uint32_t* icon_data = (uint32_t*)wrapper->GetInfo(hydra::InfoType::IconData);
                    int width = std::atoi(wrapper->GetInfo(hydra::InfoType::IconWidth));
                    int height = std::atoi(wrapper->GetInfo(hydra::InfoType::IconHeight));
                    if (width <= 0 || height <= 0)
                    {
                        printf("Invalid icon size in core %s\n",
                               Settings::CoreInfo[i].core_name.c_str());
                        continue;
                    }
                    QImage image(width, height, QImage::Format_RGBA8888);
                    for (int y = 0; y < height; y++)
                    {
                        for (int x = 0; x < width; x++)
                        {
                            uint32_t color = icon_data[y * width + x];
                            image.setPixelColor(x, y,
                                                QColor(color >> 24, (color >> 16) & 0xFF,
                                                       (color >> 8) & 0xFF, color & 0xFF));
                        }
                    }
                    image.save(cache_path);

                    QListWidgetItem* item = new QListWidgetItem(
                        QPixmap::fromImage(image), Settings::CoreInfo[i].core_name.c_str());
                    tab_list_->addItem(item);
                }
                else
                {
                    add_item(core, Settings::CoreInfo[i].core_name.c_str(), "core.png");
                    QFile::copy(QString(":/images/core.png"), cache_path);
                }
            }
        }
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

QWidget* create_separator()
{
    QFrame* separator = new QFrame;
    separator->setLineWidth(1);
    separator->setMidLineWidth(1);
    separator->setFrameShape(QFrame::HLine);
    separator->setPalette(QPalette(QColor(0, 0, 0)));
    return separator;
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
        QGridLayout* cores_layout = new QGridLayout;
        cores_layout->setAlignment(Qt::AlignTop);
        QWidget* cores_tab = new QWidget;
        cores_tab->setLayout(cores_layout);
        tab_show_->addTab(cores_tab, "Cores");
        QListWidget* core_list = new QListWidget;
        core_list->setStyleSheet("QListWidget::item { border-bottom: 1px solid black; }");
        core_list->setDragEnabled(false);
        QFile file(":/core.html");
        file.open(QIODevice::ReadOnly);
        QString html = file.readAll();
        file.close();
        for (size_t i = 0; i < Settings::CoreInfo.size(); i++)
        {
            const auto& core = Settings::CoreInfo[i];
            QListWidgetItem* item = new QListWidgetItem(core.core_name.c_str());
            core_list->addItem(item);
        }
        connect(core_list, &QListWidget::itemDoubleClicked, this,
                [this, html, core_list](QListWidgetItem* item) {
                    int index = core_list->row(item);
                    auto core = Settings::CoreInfo[index];
                    QMessageBox msg;
                    msg.setWindowTitle(core.core_name.c_str());
                    std::filesystem::path path =
                        Settings::GetSavePath() / "cache" /
                        std::string(Settings::CoreInfo[index].core_name + ".png");
                    QPixmap pixmap;
                    if (!std::filesystem::exists(path))
                    {
                        pixmap = QPixmap(":/images/core.png");
                        pixmap =
                            pixmap.scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    }
                    else
                    {
                        pixmap = QPixmap(path.string().c_str());
                        pixmap =
                            pixmap.scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    }
                    msg.setIconPixmap(pixmap);
                    msg.setText(fmt::format(fmt::runtime(html.toStdString()), core.core_name,
                                            core.version, core.system_name, core.license,
                                            core.description, core.author, core.url)
                                    .c_str());
                    msg.setDefaultButton(QMessageBox::Ok);
                    msg.exec();
                });
        cores_layout->addWidget(core_list, 0, 0, 1, 0);
        add_filepicker(cores_layout, "Core directory", "core_path", "", 1, 0, true,
                       [](const std::string&) {
                           Settings::ReinitCoreInfo();
                           QMessageBox msg;
                           msg.setWindowTitle("Core directory changed!");
                           msg.setText("Please restart the settings window to\n"
                                       "see the changes.");
                           msg.addButton("What a lazy developer!", QMessageBox::YesRole);
                           msg.exec();
                       });
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
        key_picker_ = new InputPage;
        tab_show_->addTab(key_picker_, "Input");
    }

    for (size_t i = 0; i < Settings::CoreInfo.size(); i++)
    {
        const auto& core = Settings::CoreInfo[i];
        QGridLayout* core_layout = new QGridLayout;
        core_layout->setAlignment(Qt::AlignTop);
        core_layout->setColumnStretch(0, 1);
        core_layout->setColumnStretch(1, 2);
        const std::vector<std::string>& firmware_files = core.firmware_files;
        int current_row = 0;
        for (size_t j = 0; j < firmware_files.size(); j++)
        {
            add_filepicker(core_layout, firmware_files[j], core.core_name + "_" + firmware_files[j],
                           "", current_row++, 0);
            core_layout->addWidget(create_separator(), current_row++, 0, 1, 0);
        }

        std::string core_name = core.core_name;
        for (int j = 1; j <= core.max_players; j++)
        {
            core_layout->addWidget(new QLabel("Controller port " + QString::number(j) + ":"),
                                   current_row, 0);
            QCheckBox* active = new QCheckBox("Active");
            std::string is_active =
                Settings::Get(core_name + "_controller_" + std::to_string(j) + "_active");
            active->setChecked(is_active == "true");
            connect(active, &QCheckBox::stateChanged, this, [this, j, core_name](int state) {
                Settings::Set(core_name + "_controller_" + std::to_string(j) + "_active",
                              state == Qt::Checked ? "true" : "false");
            });
            core_layout->addWidget(new QComboBox, current_row, 1);
            core_layout->addWidget(active, current_row, 2);
            current_row++;
        }

        QWidget* core_tab = new QWidget;
        core_tab->setLayout(core_layout);
        tab_show_->addTab(core_tab, core_name.c_str());
    }
}

void SettingsWindow::on_open_file_click(QLineEdit* edit, const std::string& name,
                                        const std::string& setting, const std::string& extension,
                                        std::function<void(const std::string&)> callback)
{
    auto path = QFileDialog::getOpenFileName(this, name.c_str(), "", extension.c_str(), nullptr,
                                             QFileDialog::ReadOnly);
    if (!path.isEmpty())
    {
        Settings::Set(setting, path.toStdString());
        edit->setText(path);
        if (callback)
        {
            callback(path.toStdString());
        }
    }
}

void SettingsWindow::on_open_dir_click(QLineEdit* edit, const std::string& name,
                                       const std::string& setting,
                                       std::function<void(const std::string&)> callback)
{
    auto path = QFileDialog::getExistingDirectory(this, name.c_str(), "", QFileDialog::ReadOnly);

    if (!path.isEmpty())
    {
        Settings::Set(setting, path.toStdString());
        edit->setText(path);
        if (callback)
        {
            callback(path.toStdString());
        }
    }
}

void SettingsWindow::add_filepicker(QGridLayout* layout, const std::string& name,
                                    const std::string& setting, const std::string& extension,
                                    int row, int column, bool dir,
                                    std::function<void(const std::string&)> callback)
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
        connect(button, &QPushButton::clicked, this,
                std::bind(&SettingsWindow::on_open_file_click, this, edit, name, setting, extension,
                          callback));
    }
    else
    {
        connect(button, &QPushButton::clicked, this,
                std::bind(&SettingsWindow::on_open_dir_click, this, edit, name, setting, callback));
    }
    layout->addWidget(new QLabel(QString::fromStdString(name + " path: ")), row, column);
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
