#include "keypicker.hxx"
#include <QTableWidget>
#include <QKeyEvent>
#include <include/emulator_settings.hxx>
#include <include/emulator_factory.h>

KeyPickerPage::KeyPickerPage(QWidget *parent) : QWidget(parent) {
    emulator_picker_ = new QComboBox(this);
    tab_show_ = new QTabWidget(this);
    for (int i = 0; i < EmuTypeSize; i++) {
        emulator_picker_->addItem(QString::fromStdString(EmulatorSettings::GetEmulatorData(static_cast<TKPEmu::EmuType>(i)).Name));
        EmulatorData& data = EmulatorSettings::GetEmulatorData(static_cast<TKPEmu::EmuType>(i));
        QTableWidget* table = new QTableWidget(this);
        table->setColumnCount(2);
        table->setRowCount(data.Mappings.size());
        int j = 0;
        for (auto pair : data.Mappings) {
            table->setItem(j, 0, new QTableWidgetItem(QString::fromStdString(pair.first)));
            table->setItem(j, 1, new QTableWidgetItem(QKeySequence(std::stoi(pair.second)).toString()));
            j++;
        }
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        connect(table, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(onCellDoubleClicked(int, int)));
        tab_show_->addTab(table, QString::fromStdString(data.Name));
    }
    tab_show_->tabBar()->hide();
    layout_ = new QVBoxLayout(this);
    layout_->addWidget(emulator_picker_);
    layout_->addWidget(tab_show_);
    setLayout(layout_);

    onComboBoxChange(0);
    connect(emulator_picker_, SIGNAL(currentIndexChanged(int)), this, SLOT(onComboBoxChange(int)));
}

void KeyPickerPage::onComboBoxChange(int index) {
    tab_show_->setCurrentIndex(index);
}

void KeyPickerPage::onCellDoubleClicked(int row, int column) {
    if (column == 1 && !waiting_input_) {
        waiting_input_ = true;
        row_waiting_ = row;
        static_cast<QTableWidget*>(tab_show_->currentWidget())->setItem(row_waiting_, 1, new QTableWidgetItem("Press a key..."));
    }
}

void KeyPickerPage::keyPressEvent(QKeyEvent* event) {
    // FIXME: key doesnt trigger here it seems
    if (waiting_input_) {
        waiting_input_ = false;
        auto table = static_cast<QTableWidget*>(tab_show_->currentWidget());
        table->setItem(row_waiting_, 1, new QTableWidgetItem(QKeySequence(event->key()).toString()));
        EmulatorSettings::GetEmulatorData(static_cast<TKPEmu::EmuType>(emulator_picker_->currentIndex()))
            .Mappings[table->currentItem()->text().toStdString()] = event->key();
        saveKeySettings();
    }
}

void KeyPickerPage::saveKeySettings() {
    using emu_settings = std::map<std::string, std::map<std::string, std::string>>;
    emu_settings es;
    for (int i = 0; i < EmuTypeSize; i++) {
        es[std::to_string(i)] = EmulatorSettings::GetEmulatorData(static_cast<TKPEmu::EmuType>(i)).Mappings;
    }
    std::ofstream ofs(TKPEmu::EmulatorFactory::GetSavePath() + "mappings.json", std::ios::trunc);
    json j_map(es);
    ofs << j_map << std::endl;
}