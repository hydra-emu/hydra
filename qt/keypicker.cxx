#include <iostream>
#include <QHeaderView>
#include <QKeyEvent>
#include <qt/keypicker.hxx>
#include <QTableWidget>

KeyPickerPage::KeyPickerPage(QWidget* parent) : QWidget(parent)
{
    emulator_picker_ = new QComboBox;
    tab_show_ = new QTabWidget;
    for (int i = 0; i < EmuTypeSize; i++)
    {
        emulator_picker_->addItem(
            QString::fromStdString(hydra::get_emu_type_name(static_cast<hydra::EmuType>(i))));
        QTableWidget* table = new QTableWidget;
        table->setColumnCount(2);
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        table->setHorizontalHeaderLabels(QStringList() << "Key"
                                                       << "Binding");
        QHeaderView* header = new QHeaderView(Qt::Vertical);
        header->hide();
        table->verticalHeader()->deleteLater();
        table->setVerticalHeader(header);
        // table->setRowCount(emulator_data.Mappings.size());
        // int j = 0;
        // for (auto pair : emulator_data.Mappings)
        // {
        //     table->setItem(j, 0, new QTableWidgetItem(QString::fromStdString(pair.first)));
        //     if (!pair.second.empty())
        //     {
        //         table->setItem(
        //             j, 1,
        //             new
        //             QTableWidgetItem(QKeySequence(std::atoi(pair.second.c_str())).toString()));
        //     }
        //     j++;
        // }
        table->setRowCount(1);
        QTableWidgetItem* item = new QTableWidgetItem("Key");
        item->setFlags(Qt::NoItemFlags);
        item->setForeground(Qt::white);
        table->setItem(0, 0, item);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->setItem(0, 1, new QTableWidgetItem("Some binding"));
        table->setFocusPolicy(Qt::NoFocus);
        connect(table, SIGNAL(cellDoubleClicked(int, int)), this,
                SLOT(onCellDoubleClicked(int, int)));
        tab_show_->addTab(table, QString::fromStdString(
                                     hydra::get_emu_type_name(static_cast<hydra::EmuType>(i))));
    }
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