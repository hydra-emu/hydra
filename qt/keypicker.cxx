#include <filesystem>
#include <hydra/core.hxx>
#include <iostream>
#include <json.hpp>
#include <QFile>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMessageBox>
#include <qt/keypicker.hxx>
#include <QTableWidget>
#include <str_hash.hxx>

// Keypad1Up,
// Keypad1Down,
// Keypad1Left,
// Keypad1Right,
// Keypad2Up,
// Keypad2Down,
// Keypad2Left,
// Keypad2Right,
// A,
// B,
// X,
// Y,
// Z,
// L1,
// R1,
// L2,
// R2,
// L3,
// R3,
// Start,
// Select,
// Touch,
// Analog1Up,
// Analog1Down,
// Analog1Left,
// Analog1Right,
// Analog2Up,
// Analog2Down,
// Analog2Left,
// Analog2Right,

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

std::map<std::string, std::string> load_mappings(const std::string& file_data)
{
    nlohmann::json json;
    try
    {
        json = nlohmann::json::parse(file_data);
        std::map<std::string, std::string> out;
        for (auto& pair : json.items())
        {
            out[pair.key()] = pair.value();
        }
        return out;
    } catch (const std::exception& e)
    {
        std::cerr << "Failed to parse json: " << e.what() << std::endl;
        return {};
    }
}

std::string save_mappings(const std::map<std::string, std::string>& mappings)
{
    nlohmann::json json;
    for (auto& pair : mappings)
    {
        json[pair.first] = pair.second;
    }
    return json.dump(4);
}

KeyPickerPage::KeyPickerPage(QWidget* parent) : QWidget(parent)
{
    QFile file(":/default_mappings.json");
    file.open(QIODevice::ReadOnly);
    std::string file_data = file.readAll().toStdString();
    std::map<std::string, std::string> default_mappings = load_mappings(file_data);

    tab_show_ = new QTabWidget;
    emulator_picker_ = new QComboBox;
    emulator_picker_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QTableWidget* default_table = copy_page(nullptr);
    add_tab("Default mappings", default_table);

    QHBoxLayout* hlayout = new QHBoxLayout;
    hlayout->addWidget(emulator_picker_);

    add_button_ = new QPushButton("Add");
    hlayout->addWidget(add_button_);
    connect(add_button_, SIGNAL(clicked()), this, SLOT(onAddButtonClicked()));

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

    onComboBoxChange(0);
    connect(emulator_picker_, SIGNAL(currentIndexChanged(int)), this, SLOT(onComboBoxChange(int)));
}

void KeyPickerPage::onComboBoxChange(int index)
{
    tab_show_->setCurrentIndex(index);
}

void KeyPickerPage::onCellDoubleClicked(int row, int column)
{
    if (column != 1)
        return;

    ensure_same();
    if (!adding_mapping_)
    {
        QTableWidgetItem* item =
            static_cast<QTableWidget*>(tab_show_->currentWidget())->item(row, 1);
        if (!waiting_input_)
        {
            waiting_input_ = true;
            row_waiting_ = row;
            old_key_ = item->text();

            item->setText("Press a key");
            add_button_->hide();
        }
        else
        {
            if (row == row_waiting_)
            {
                waiting_input_ = false;
            }
        }
    }
}

void KeyPickerPage::onAddButtonClicked()
{
    ensure_same();
    // Shouldn't happen but w/e
    if (!waiting_input_)
    {
        emulator_picker_->setCurrentIndex(emulator_picker_->count() - 1);
        emulator_picker_->setEditable(true);
        emulator_picker_->setEditText("New mapping");
        emulator_picker_->hidePopup();
        tab_show_->setEnabled(false);
        save_button_->show();
        cancel_button_->show();
        add_button_->hide();
        remove_button_->hide();
    }
}

void KeyPickerPage::onRemoveButtonClicked()
{
    ensure_same();
    if (!waiting_input_)
    {
        if (emulator_picker_->currentIndex() == 0)
        {
            QMessageBox::warning(this, "Warning", "Cannot remove default mappings");
            return;
        }

        tab_show_->removeTab(tab_show_->currentIndex());
        emulator_picker_->removeItem(tab_show_->currentIndex());
        emulator_picker_->setCurrentIndex(emulator_picker_->count() - 1);
    }
}

void KeyPickerPage::onSaveButtonClicked()
{
    ensure_same();
    if (!waiting_input_)
    {
        add_tab(emulator_picker_->currentText(),
                copy_page(static_cast<QTableWidget*>(tab_show_->currentWidget())));
        emulator_picker_->clearEditText();
        emulator_picker_->setEditable(false);
        emulator_picker_->setCurrentIndex(emulator_picker_->count() - 1);
        tab_show_->setEnabled(true);
        save_button_->hide();
        cancel_button_->hide();
        add_button_->show();
        remove_button_->show();
    }
}

void KeyPickerPage::onCancelButtonClicked()
{
    ensure_same();
    if (!waiting_input_)
    {
        emulator_picker_->setEditable(false);
        tab_show_->setEnabled(true);
        save_button_->hide();
        cancel_button_->hide();
        add_button_->show();
        remove_button_->show();
    }
}

void KeyPickerPage::KeyPressed(QKeyEvent* event)
{
    if (waiting_input_)
    {
        waiting_input_ = false;
        auto item = static_cast<QTableWidget*>(tab_show_->currentWidget())->item(row_waiting_, 1);
        item->setText(QKeySequence(event->key()).toString());
    }
}

void KeyPickerPage::ensure_same()
{
    // Just a sanity check function, in case I messed up the code or I mess it up in the future
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

QTableWidget* KeyPickerPage::copy_page(QTableWidget* copy)
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

void KeyPickerPage::add_tab(const QString& name, QTableWidget* table)
{
    ensure_same();
    tab_show_->addTab(table, name);
    emulator_picker_->addItem(name);
}

void KeyPickerPage::onCancelWaitingButtonClicked()
{
    ensure_same();
    if (waiting_input_)
    {
        waiting_input_ = false;
        auto item = static_cast<QTableWidget*>(tab_show_->currentWidget())->item(row_waiting_, 1);
        item->setText(old_key_);
    }
}