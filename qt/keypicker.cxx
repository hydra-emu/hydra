#include <filesystem>
#include <hydra/core.hxx>
#include <input.hxx>
#include <iostream>
#include <json.hpp>
#include <QCheckBox>
#include <QFile>
#include <QFileInfo>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMessageBox>
#include <qt/keypicker.hxx>
#include <QTableWidget>
#include <settings.hxx>
#include <str_hash.hxx>

constexpr const char* serialize(hydra::ButtonType input)
{
    switch (input)
    {
        case hydra::ButtonType::Keypad1Up:
            return "Left Keypad - Up button";
        case hydra::ButtonType::Keypad1Down:
            return "Left Keypad - Down button";
        case hydra::ButtonType::Keypad1Left:
            return "Left Keypad - Left button";
        case hydra::ButtonType::Keypad1Right:
            return "Left Keypad - Right button";
        case hydra::ButtonType::Keypad2Up:
            return "Right Keypad - Up button";
        case hydra::ButtonType::Keypad2Down:
            return "Right Keypad - Down button";
        case hydra::ButtonType::Keypad2Left:
            return "Right Keypad - Left button";
        case hydra::ButtonType::Keypad2Right:
            return "Right Keypad - Right button";
        case hydra::ButtonType::A:
            return "A button";
        case hydra::ButtonType::B:
            return "B button";
        case hydra::ButtonType::X:
            return "X button";
        case hydra::ButtonType::Y:
            return "Y button";
        case hydra::ButtonType::Z:
            return "Z button";
        case hydra::ButtonType::L1:
            return "L1 button";
        case hydra::ButtonType::R1:
            return "R1 button";
        case hydra::ButtonType::L2:
            return "L2 button";
        case hydra::ButtonType::R2:
            return "R2 button";
        case hydra::ButtonType::L3:
            return "L3 button";
        case hydra::ButtonType::R3:
            return "R3 button";
        case hydra::ButtonType::Start:
            return "Start button";
        case hydra::ButtonType::Select:
            return "Select button";
        case hydra::ButtonType::Touch:
            return "Touch";
        case hydra::ButtonType::Analog1Up:
            return "Left Analog Stick - Up";
        case hydra::ButtonType::Analog1Down:
            return "Left Analog Stick - Down";
        case hydra::ButtonType::Analog1Left:
            return "Left Analog Stick - Left";
        case hydra::ButtonType::Analog1Right:
            return "Left Analog Stick - Right";
        case hydra::ButtonType::Analog2Up:
            return "Right Analog Stick - Up";
        case hydra::ButtonType::Analog2Down:
            return "Right Analog Stick - Down";
        case hydra::ButtonType::Analog2Left:
            return "Right Analog Stick - Left";
        case hydra::ButtonType::Analog2Right:
            return "Right Analog Stick - Right";
        default:
            return "Unknown";
    }
}

InputPage::InputPage(const std::vector<std::tuple<QComboBox*, int, QString>>& listener_combos,
                     QWidget* parent)
    : listener_combos_(listener_combos), QWidget(parent)
{
    tab_show_ = new QTabWidget;
    emulator_picker_ = new QComboBox;
    emulator_picker_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    {
        QFile file(":/default_mappings.json");
        file.open(QIODevice::ReadOnly);
        add_tab_from_file(file);
    }

    std::filesystem::path dirpath = Settings::GetSavePath() / "mappings";
    std::vector<std::filesystem::path> files = hydra::Input::Scan(dirpath, ".json");
    for (const auto& path : files)
    {
        QFile file(path.c_str());
        file.open(QIODevice::ReadOnly);
        add_tab_from_file(file);
    }

    for (const auto& [combo, player, name] : listener_combos_)
    {
        int found = combo->findText(
            Settings::Get((name.toStdString() + std::string("_") + std::to_string(player)) +
                          "_mapping")
                .c_str());
        if (found == -1)
        {
            combo->setCurrentIndex(0);
        }
        else
        {
            combo->setCurrentIndex(found);
        }
    }

    QHBoxLayout* hlayout = new QHBoxLayout;
    hlayout->addWidget(emulator_picker_);

    mapping_name_ = new QLineEdit;
    mapping_name_->hide();
    hlayout->addWidget(mapping_name_);

    add_button_ = new QPushButton("Add");
    hlayout->addWidget(add_button_);
    connect(add_button_, SIGNAL(clicked()), this, SLOT(onAddButtonClicked()));

    copy_button_ = new QPushButton("Copy");
    hlayout->addWidget(copy_button_);
    connect(copy_button_, SIGNAL(clicked()), this, SLOT(onCopyButtonClicked()));

    remove_button_ = new QPushButton("Remove");
    hlayout->addWidget(remove_button_);
    connect(remove_button_, SIGNAL(clicked()), this, SLOT(onRemoveButtonClicked()));

    save_button_ = new QPushButton("Save");
    hlayout->addWidget(save_button_);
    connect(save_button_, SIGNAL(clicked()), this, SLOT(onSaveButtonClicked()));
    save_button_->hide();

    cancel_button_ = new QPushButton("Cancel");
    hlayout->addWidget(cancel_button_);
    connect(cancel_button_, SIGNAL(clicked()), this, SLOT(onCancelButtonClicked()));
    cancel_button_->hide();

    cancel_waiting_button_ = new QPushButton("Nevermind");
    hlayout->addWidget(cancel_waiting_button_);
    connect(cancel_waiting_button_, SIGNAL(clicked()), this, SLOT(onCancelWaitingButtonClicked()));
    cancel_waiting_button_->hide();

    tab_show_->tabBar()->hide();
    tab_show_->setFocusPolicy(Qt::NoFocus);
    layout_ = new QVBoxLayout;
    QWidget* holder = new QWidget;
    holder->setLayout(hlayout);
    layout_->addWidget(holder);
    layout_->addWidget(tab_show_);
    setLayout(layout_);

    set_tab(0);
    connect(emulator_picker_, SIGNAL(currentIndexChanged(int)), this, SLOT(set_tab(int)));
}

void InputPage::add_tab_from_file(QFile& file)
{
    std::string file_data = file.readAll().toStdString();
    hydra::KeyMappings mappings = hydra::Input::Load(file_data);
    QTableWidget* table = copy_page(nullptr);
    for (int i = 0; i < (int)hydra::ButtonType::InputCount; ++i)
    {
        table->item(i, 0)->setText(serialize(static_cast<hydra::ButtonType>(i)));
        table->item(i, 1)->setText(mappings[i].toString());
    }
    QString name = QFileInfo(file).baseName();
    if (file.fileName() == ":/default_mappings.json")
        name = "Default mappings";
    add_tab(name, table);
}

void InputPage::onCellDoubleClicked(int row, int column)
{
    if (column != 1)
        return;

    ensure_same();
    if (!adding_mapping_)
    {
        if (tab_show_->currentIndex() == 0)
        {
            QMessageBox msg;
            msg.setIcon(QMessageBox::Warning);
            msg.setText("You can't edit the default mappings.\nCreate a new mapping?");
            msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msg.setDefaultButton(QMessageBox::Yes);
            int ret = msg.exec();
            if (ret == QMessageBox::Yes)
            {
                onAddButtonClicked();
            }
            return;
        }

        QTableWidgetItem* item =
            static_cast<QTableWidget*>(tab_show_->currentWidget())->item(row, 1);
        if (!waiting_input_)
        {
            row_waiting_ = row;
            old_key_ = item->text();

            item->setText("Press a key");
            set_buttons(ButtonPage::WaitingInput);
        }
    }
}

void InputPage::onAddButtonClicked()
{
    ensure_same();
    if (!waiting_input_)
    {
        set_buttons(ButtonPage::AddingMapping);
    }
}

void InputPage::onRemoveButtonClicked()
{
    ensure_same();
    if (!waiting_input_)
    {
        if (emulator_picker_->currentIndex() == 0)
        {
            QMessageBox::warning(this, "Warning", "Cannot remove default mappings");
            return;
        }

        QMessageBox msg;
        msg.setIcon(QMessageBox::Warning);
        msg.setText("Are you sure you want to permanently remove this mapping?");
        msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msg.setDefaultButton(QMessageBox::No);
        int ret = msg.exec();
        if (ret == QMessageBox::Yes)
        {
            std::string current_name = emulator_picker_->currentText().toStdString();
            int index = emulator_picker_->currentIndex();
            tab_show_->removeTab(index);
            emulator_picker_->removeItem(index);
            set_tab(index - 1);

            for (auto& [combo, n, s] : listener_combos_)
            {
                combo->removeItem(index);
            }

            auto path = Settings::GetSavePath() / "mappings" / (current_name + ".json");
            std::error_code ec;
            std::filesystem::remove(path, ec);
            if (ec)
            {
                QMessageBox::warning(this, "Warning",
                                     QString("Failed to remove mapping file: ") + path.c_str());
            }
        }
    }
}

void InputPage::onSaveButtonClicked()
{
    ensure_same();
    if (!waiting_input_)
    {
        if (check_exists(mapping_name_->text()))
        {
            QMessageBox::warning(this, "Warning", "Mapping name already exists");
            return;
        }

        auto copy =
            is_copying_page_ ? static_cast<QTableWidget*>(tab_show_->currentWidget()) : nullptr;
        add_tab(mapping_name_->text(), copy_page(copy));
        set_buttons(ButtonPage::Normal);
        set_tab(emulator_picker_->count() - 1);

        save_page((QTableWidget*)tab_show_->currentWidget());
    }
}

hydra::KeyMappings InputPage::table_to_mappings(QTableWidget* table)
{
    hydra::KeyMappings mappings;
    for (int i = 0; i < table->rowCount(); i++)
    {
        QTableWidgetItem* item = table->item(i, 1);
        mappings[i] = hydra::Input::StringToKey(item->text().toStdString());
    }
    return mappings;
}

bool InputPage::check_exists(const QString& name)
{
    std::filesystem::path path =
        Settings::GetSavePath() / "mappings" / (name.toStdString() + ".json");
    if (std::filesystem::exists(path))
        return true;

    for (int i = 0; i < emulator_picker_->count(); i++)
    {
        if (emulator_picker_->itemText(i) == name)
        {
            printf("Weird, name exists in QComboBox but file doesn't. Was the mapping file "
                   "removed?\n");
            return true;
        }
    }
    return false;
}

void InputPage::onCopyButtonClicked()
{
    onAddButtonClicked();
    is_copying_page_ = true;
}

void InputPage::onCancelButtonClicked()
{
    ensure_same();
    if (!waiting_input_)
    {
        set_buttons(ButtonPage::Normal);
    }
}

void InputPage::KeyPressed(QKeyEvent* event)
{
    if (waiting_input_)
    {
        // Check if the key is already used
        QTableWidget* table = static_cast<QTableWidget*>(tab_show_->currentWidget());
        int count = table->rowCount();
        for (int i = 0; i < count; i++)
        {
            QTableWidgetItem* item = table->item(i, 1);
            if (item->text() == QKeySequence(event->key()).toString())
            {
                std::string act = Settings::Get("mappings_overwrite");
                if (act.empty())
                {
                    QString key_name = table->item(i, 0)->text();
                    QMessageBox msg;
                    msg.setIcon(QMessageBox::Warning);
                    msg.setText("Key already used by " + key_name + "\nReplace?");
                    msg.addButton(QMessageBox::Yes);
                    msg.addButton(QMessageBox::No);
                    QAbstractButton* use_for_both = new QPushButton("Use mapping for both");
                    msg.addButton(use_for_both, QMessageBox::YesRole);
                    QCheckBox* remember = new QCheckBox("Remember my choice");
                    msg.setCheckBox(remember);
                    msg.setDefaultButton(QMessageBox::No);
                    int ret = msg.exec();
                    if (msg.clickedButton() == use_for_both)
                    {
                        act = "both";
                    }
                    else if (ret == QMessageBox::Yes)
                    {
                        act = "yes";
                    }
                    else
                    {
                        act = "no";
                    }
                    if (remember->isChecked())
                    {
                        Settings::Set("mappings_overwrite", act);
                    }
                }
                if (act == "yes")
                {
                    item->setText("");
                    break;
                }
                else if (act == "no")
                {
                    onCancelWaitingButtonClicked();
                    return;
                }
                else if (act == "both")
                {
                    break;
                }
                else
                {
                    Settings::Set("mappings_overwrite", "");
                }
            }
        }

        auto item = static_cast<QTableWidget*>(tab_show_->currentWidget())->item(row_waiting_, 1);
        item->setText(QKeySequence(event->key()).toString());
        save_page((QTableWidget*)tab_show_->currentWidget());
        cancel_waiting();
    }
}

// Just a sanity check function, in case I messed up the code or I mess it up in the future
void InputPage::ensure_same()
{
    if (emulator_picker_->count() != tab_show_->count())
    {
        QMessageBox::critical(this, "Error", "Emulator picker and tab show have different counts");
        exit(1);
    }

    if (emulator_picker_->currentIndex() != tab_show_->currentIndex())
    {
        QMessageBox::critical(this, "Error",
                              "Emulator picker and tab show have different current indexes");
        exit(1);
    }

    for (int i = 0; i < emulator_picker_->count(); i++)
    {
        if (emulator_picker_->itemText(i) != tab_show_->tabText(i))
        {
            QMessageBox::critical(this, "Error",
                                  "Emulator picker and tab show have different texts");
            exit(1);
        }
    }
}

QTableWidget* InputPage::copy_page(QTableWidget* copy)
{
    QHeaderView* header = new QHeaderView(Qt::Vertical);
    header->hide();
    QTableWidget* table = new QTableWidget;
    table->setColumnCount(2);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setHorizontalHeaderLabels(QStringList() << "Key"
                                                   << "Binding");
    table->verticalHeader()->deleteLater();
    table->setVerticalHeader(header);
    table->setRowCount((int)hydra::ButtonType::InputCount);
    table->setFocusPolicy(Qt::NoFocus);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    for (int i = 0; i < (int)hydra::ButtonType::InputCount; i++)
    {
        table->setItem(i, 0, new QTableWidgetItem(serialize((hydra::ButtonType)i)));
        table->setItem(i, 1, new QTableWidgetItem(copy ? copy->item(i, 1)->text() : ""));
    }

    connect(table, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(onCellDoubleClicked(int, int)));
    return table;
}

QTableWidget* InputPage::make_page(const std::filesystem::path& path)
{
    QTableWidget* page = copy_page(nullptr);

    hydra::KeyMappings mappings = hydra::Input::Open(path);
    for (int i = 0; i < (int)hydra::ButtonType::InputCount; i++)
    {
        page->item(i, 1)->setText(mappings[i].toString());
    }

    return page;
}

void InputPage::save_page(QTableWidget* page)
{
    std::filesystem::path path = Settings::GetSavePath() / "mappings" /
                                 (emulator_picker_->currentText().toStdString() + ".json");
    hydra::KeyMappings mappings;

    for (int i = 0; i < (int)hydra::ButtonType::InputCount; i++)
    {
        mappings[i] = QKeySequence(page->item(i, 1)->text());
    }

    hydra::Input::Save(path, mappings);
}

void InputPage::set_tab(int index)
{
    tab_show_->setCurrentIndex(index);
    emulator_picker_->setCurrentIndex(index);
    ensure_same();
}

void InputPage::add_tab(const QString& name, QTableWidget* table)
{
    ensure_same();
    tab_show_->addTab(table, name);
    emulator_picker_->addItem(name);

    for (auto& [combo, player, _] : listener_combos_)
    {
        combo->addItem(name);
    }
}

void InputPage::onCancelWaitingButtonClicked()
{
    ensure_same();
    if (waiting_input_)
    {
        auto item = static_cast<QTableWidget*>(tab_show_->currentWidget())->item(row_waiting_, 1);
        item->setText(old_key_);
        cancel_waiting();
    }
}

void InputPage::cancel_waiting()
{
    set_buttons(ButtonPage::Normal);
}

void InputPage::set_buttons(ButtonPage page)
{
    switch (page)
    {
        case ButtonPage::Normal:
        {
            add_button_->show();
            copy_button_->show();
            remove_button_->show();
            cancel_button_->hide();
            save_button_->hide();
            cancel_waiting_button_->hide();
            emulator_picker_->setEnabled(true);
            emulator_picker_->show();
            mapping_name_->hide();
            tab_show_->setEnabled(true);
            adding_mapping_ = false;
            waiting_input_ = false;
            break;
        }
        case ButtonPage::AddingMapping:
        {
            add_button_->hide();
            copy_button_->hide();
            remove_button_->hide();
            cancel_button_->show();
            save_button_->show();
            cancel_waiting_button_->hide();
            emulator_picker_->setEnabled(false);
            emulator_picker_->hide();
            mapping_name_->show();
            int i = 0;
            if (check_exists("New mapping"))
            {
                i++;
                while (check_exists("New mapping " + QString::number(i)))
                    i++;
            }
            mapping_name_->setText("New mapping" + (i ? " " + QString::number(i) : ""));
            tab_show_->setEnabled(false);
            adding_mapping_ = true;
            waiting_input_ = false;
            break;
        }
        case ButtonPage::WaitingInput:
        {
            add_button_->hide();
            copy_button_->hide();
            remove_button_->hide();
            cancel_button_->hide();
            save_button_->hide();
            cancel_waiting_button_->show();
            emulator_picker_->setEnabled(false);
            emulator_picker_->show();
            mapping_name_->hide();
            tab_show_->setEnabled(true);
            adding_mapping_ = false;
            waiting_input_ = true;
            break;
        }
    };
}
