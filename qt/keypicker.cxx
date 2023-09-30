#include <common/str_hash.hxx>
#include <core/core.h>
#include <filesystem>
#include <iostream>
#include <json.hpp>
#include <QFile>
#include <QHeaderView>
#include <QKeyEvent>
#include <qt/keypicker.hxx>
#include <QTableWidget>

inline std::string serialize_pretty(hc_input_e input)
{
    std::string out;
    switch (input)
    {
#define X(key) out = #key;
        HC_INPUTS
#undef X
        default:
            return "";
    }

    std::string sub = out.substr(strlen("HC_INPUT_"));
    bool first_letter = true;
    for (auto& c : sub)
    {
        if (c == '_')
        {
            c = ' ';
            first_letter = true;
        }
        else
        {
            if (first_letter)
            {
                c = toupper(c);
                first_letter = false;
            }
            else
            {
                c = tolower(c);
            }
        }
    }
    return sub.c_str();
}

constexpr const char* serialize(hc_input_e input)
{
    switch (input)
    {
#define X(key) \
    case key:  \
        return #key;
        HC_INPUTS
#undef X
        default:
            return "";
    }
}

constexpr hc_input_e deserialize(const char* input)
{
    switch (str_hash(input))
    {
#define X(key)           \
    case str_hash(#key): \
        return key;
        HC_INPUTS
#undef X
        default:
            return HC_INPUT_SIZE;
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
    emulator_picker_ = new QComboBox;
    tab_show_ = new QTabWidget;
    QFile file(":/default_mappings.json");
    file.open(QIODevice::ReadOnly);
    std::string file_data = file.readAll().toStdString();
    std::map<std::string, std::string> mappings = load_mappings(file_data);
    emulator_picker_->addItem("Default mappings");
    QTableWidget* table = new QTableWidget;
    table->setColumnCount(2);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setHorizontalHeaderLabels(QStringList() << "Key"
                                                   << "Binding");
    QHeaderView* header = new QHeaderView(Qt::Vertical);
    header->hide();
    table->verticalHeader()->deleteLater();
    table->setVerticalHeader(header);
    table->setRowCount(mappings.size());
    table->setFocusPolicy(Qt::NoFocus);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    int j = 0;
    for (auto pair : mappings)
    {
        table->setItem(j, 0, new QTableWidgetItem(pair.first.c_str()));
        if (!pair.second.empty())
        {
            table->setItem(j, 1, new QTableWidgetItem(pair.second.c_str()));
        }
        j++;
    }
    connect(table, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(onCellDoubleClicked(int, int)));
    tab_show_->addTab(table, QString::fromStdString("Default mappings"));
    tab_show_->tabBar()->hide();
    tab_show_->setFocusPolicy(Qt::NoFocus);
    layout_ = new QVBoxLayout;
    layout_->addWidget(emulator_picker_);
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

    QTableWidgetItem* item = static_cast<QTableWidget*>(tab_show_->currentWidget())->item(row, 1);
    if (!waiting_input_)
    {
        waiting_input_ = true;
        row_waiting_ = row;

        item->setText("Press a key");
    }
    else
    {
        if (row == row_waiting_)
        {
            waiting_input_ = false;
            item->setText("Some binding");
        }
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

void KeyPickerPage::saveKeySettings()
{
    // using emu_settings = std::map<std::string, std::map<std::string, std::string>>;
    // emu_settings es;
    // for (int i = 0; i < EmuTypeSize; i++)
    // {
    //     es[std::to_string(i)] =
    //         EmulatorSettings::GetEmulatorData(static_cast<hydra::EmuType>(i)).Mappings;
    // }
    // std::ofstream ofs(hydra::EmulatorFactory::GetSavePath() + "mappings.json", std::ios::trunc);
    // json j_map(es);
    // ofs << j_map << std::endl;
}