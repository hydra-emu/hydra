#include "cheatswindow.hxx"

#include "compatibility.hxx"
#include "core_loader.hxx"
#include "hydra/core.hxx"
#include "json.hpp"
#include "log.h"
#include "settings.hxx"

#include <qcheckbox.h>
#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QRegularExpression>
#include <QTextEdit>
#include <QVBoxLayout>

class CheatEntryWidget : public QWidget
{
public:
    CheatEntryWidget(std::shared_ptr<hydra::EmulatorWrapper> wrapper, CheatMetadata& metadata,
                     QListWidget* parent);

    CheatMetadata& GetMetadata()
    {
        return metadata_;
    }

    void Update()
    {
        lbl_name_->setText(metadata_.name.c_str());
        chk_enabled_->setChecked(metadata_.enabled);
        update();
    }

private:
    std::shared_ptr<hydra::EmulatorWrapper> wrapper_;
    CheatMetadata& metadata_;
    QLabel* lbl_name_;
    QCheckBox* chk_enabled_;
};

class CheatEditDialog : public QDialog
{
public:
    CheatEditDialog(std::shared_ptr<hydra::EmulatorWrapper> wrapper, CheatEntryWidget& entry)
        : QDialog(), wrapper_(wrapper), entry_(entry), metadata_(entry.GetMetadata())
    {
        setModal(true);
        QVBoxLayout* layout = new QVBoxLayout;

        QLineEdit* txt_name = new QLineEdit;
        txt_name->setText(metadata_.name.c_str());
        txt_name->setPlaceholderText(tr("Cheat name"));
        connect(txt_name, &QLineEdit::textChanged, this,
                [this, txt_name]() { metadata_.name = txt_name->text().toStdString(); });
        layout->addWidget(txt_name);

        QTextEdit* txt_code = new QTextEdit;
        QFont font;
        font.setFamily("Courier");
        font.setFixedPitch(true);
        font.setPointSize(10);
        txt_code->setFont(font);
        txt_code->setPlaceholderText(tr("Cheat code"));
        if (metadata_.code.size() != 0)
        {
            printf("Setting code to %s\n", metadata_.code.c_str());
            txt_code->setText(metadata_.code.c_str());
        }
        connect(txt_code, &QTextEdit::textChanged, this, [txt_code]() {
            QString new_text = txt_code->toPlainText();
            new_text.replace(QRegularExpression("[^0-9a-fA-F]"), "");
            QStringList tokens;
            for (int i = 0; i < new_text.length(); i += 8)
            {
                tokens << new_text.mid(i, 8);
            }
            txt_code->blockSignals(true);
            new_text = tokens.join(" ");
            for (int i = 17; i < new_text.length(); i += 18)
            {
                new_text[i] = '\n';
            }
            txt_code->setText(new_text);
            txt_code->moveCursor(QTextCursor::End);
            txt_code->blockSignals(false);
        });
        is_editing_ = metadata_.handle != hydra::BAD_CHEAT;
        layout->addWidget(txt_code);
        setLayout(layout);

        QDialogButtonBox* button_box =
            new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        layout->addWidget(button_box);
        connect(button_box, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(button_box, &QDialogButtonBox::rejected, this, &QDialog::reject);
        connect(this, &QDialog::accepted, this, [this, txt_code]() {
            QString code = txt_code->toPlainText();
            code.replace(QRegularExpression("[^0-9a-fA-F]"), "");
            metadata_.code = code.toStdString();
            std::vector<uint8_t> bytes = hydra::hex_to_bytes(metadata_.code);
            hydra::ICheat* cheat_interface = wrapper_->shell->asICheat();
            if (is_editing_)
            {
                cheat_interface->removeCheat(metadata_.handle);
                metadata_.handle = cheat_interface->addCheat(bytes.data(), bytes.size());
                if (metadata_.enabled)
                {
                    cheat_interface->enableCheat(metadata_.handle);
                }
                else
                {
                    cheat_interface->disableCheat(metadata_.handle);
                }
            }
            else
            {
                if (metadata_.name.empty())
                {
                    metadata_.name = tr("My cheat code").toStdString();
                }
                metadata_.handle = cheat_interface->addCheat(bytes.data(), bytes.size());
                cheat_interface->disableCheat(metadata_.handle);
            }
            entry_.Update();
        });
    }

private:
    std::shared_ptr<hydra::EmulatorWrapper> wrapper_;
    CheatEntryWidget& entry_;
    CheatMetadata& metadata_;
    bool is_editing_;
};

CheatEntryWidget::CheatEntryWidget(std::shared_ptr<hydra::EmulatorWrapper> wrapper,
                                   CheatMetadata& metadata, QListWidget* parent)
    : QWidget(), wrapper_(wrapper), metadata_(metadata)
{
    QHBoxLayout* layout = new QHBoxLayout;
    chk_enabled_ = new QCheckBox;

    chk_enabled_->setChecked(metadata_.enabled);
    connect(chk_enabled_, &QCheckBox::stateChanged, this, [this](int state) {
        metadata_.enabled = state == Qt::Checked;
        hydra::ICheat* cheat_interface = wrapper_->shell->asICheat();
        if (metadata_.handle == hydra::BAD_CHEAT)
        {
            printf("Cheat handle is bad, this shouldn't happen\n");
            return;
        }
        if (metadata_.enabled)
        {
            cheat_interface->enableCheat(metadata_.handle);
        }
        else
        {
            cheat_interface->disableCheat(metadata_.handle);
        }
    });

    lbl_name_ = new QLabel(metadata_.name.c_str());
    QPushButton* btn_edit = new QPushButton(tr("Edit"));

    connect(btn_edit, &QPushButton::clicked, this, [this]() {
        CheatEditDialog* dialog = new CheatEditDialog(wrapper_, *this);
        dialog->show();
    });

    layout->addWidget(chk_enabled_);
    layout->addWidget(lbl_name_);
    layout->addWidget(btn_edit);
    setLayout(layout);

    QListWidgetItem* list_item = new QListWidgetItem;
    list_item->setSizeHint(sizeHint());
    parent->addItem(list_item);
    parent->setItemWidget(list_item, this);
}

CheatsWindow::CheatsWindow(std::shared_ptr<hydra::EmulatorWrapper> wrapper,
                           std::shared_ptr<std::vector<CheatMetadata>> metadata,
                           const std::filesystem::path& path, QAction* action, QWidget* parent)
    : QWidget(parent, Qt::Window), wrapper_(wrapper), metadata_(metadata), cheat_path_(path),
      menu_action_(action)
{
    if (!wrapper_->shell->hasInterface(hydra::InterfaceType::ICheat))
        log_fatal(
            "Emulator does not have cheat interface, this dialog shouldn't have been opened?\n");

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(6, 6, 6, 6);
    setLayout(layout);

    cheat_list_ = new QListWidget;
    layout->addWidget(cheat_list_);

    QWidget* button_box = new QWidget;
    QHBoxLayout* button_layout = new QHBoxLayout;

    QPushButton* btn_add = new QPushButton(tr("Add"));
    connect(btn_add, &QPushButton::clicked, this, [this]() {
        metadata_->push_back(CheatMetadata{});
        CheatEntryWidget* entry = new CheatEntryWidget(wrapper_, metadata_->back(), cheat_list_);
        CheatEditDialog* dialog = new CheatEditDialog(wrapper_, *entry);
        dialog->show();
        save_cheats();
    });

    QPushButton* btn_remove = new QPushButton(tr("Remove"));
    connect(btn_remove, &QPushButton::clicked, this, [this]() {
        QListWidgetItem* item = cheat_list_->currentItem();
        if (item == nullptr)
        {
            return;
        }

        CheatEntryWidget* entry = (CheatEntryWidget*)cheat_list_->itemWidget(item);
        wrapper_->shell->asICheat()->disableCheat(entry->GetMetadata().handle);
        wrapper_->shell->asICheat()->removeCheat(entry->GetMetadata().handle);
        cheat_list_->takeItem(cheat_list_->row(item));
        save_cheats();
        entry->deleteLater();
    });

    button_layout->addWidget(btn_add);
    button_layout->addWidget(btn_remove);
    button_box->setLayout(button_layout);

    layout->addWidget(button_box);

    for (CheatMetadata& metadata : *metadata_)
    {
        new CheatEntryWidget(wrapper_, metadata, cheat_list_);
    }
    show();
}

void CheatsWindow::save_cheats()
{
    nlohmann::json cheat_json;
    for (int i = 0; i < cheat_list_->count(); i++)
    {
        CheatEntryWidget* entry = (CheatEntryWidget*)cheat_list_->itemWidget(cheat_list_->item(i));
        CheatMetadata& cheat = entry->GetMetadata();
        cheat_json.push_back({{"enabled", cheat.enabled ? "true" : "false"},
                              {"name", cheat.name},
                              {"code", cheat.code}});
    }
    std::ofstream cheat_file(cheat_path_);
    cheat_file << cheat_json.dump(4);
}
