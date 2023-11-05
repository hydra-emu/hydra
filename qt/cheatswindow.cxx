#include "cheatswindow.hxx"

#include "compatibility.hxx"
#include "corewrapper.hxx"
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
    CheatEntryWidget(std::shared_ptr<hydra::EmulatorWrapper> wrapper, uint32_t handle,
                     QListWidget* parent);

    void Update()
    {
        const hydra::CheatMetadata& metadata = wrapper_->GetCheat(handle_);
        lbl_name_->setText(metadata.name.c_str());
        chk_enabled_->setChecked(metadata.enabled);
        update();
    }

    uint32_t GetHandle()
    {
        return handle_;
    }

    void SetHandle(uint32_t handle)
    {
        handle_ = handle;
    }

private:
    std::shared_ptr<hydra::EmulatorWrapper> wrapper_;
    uint32_t handle_;
    QLabel* lbl_name_;
    QCheckBox* chk_enabled_;
};

class CheatEditDialog : public QDialog
{
public:
    CheatEditDialog(std::shared_ptr<hydra::EmulatorWrapper> wrapper, uint32_t handle,
                    CheatEntryWidget& entry)
        : QDialog(), wrapper_(wrapper), entry_(entry), handle_(handle)
    {
        setModal(true);
        QVBoxLayout* layout = new QVBoxLayout;

        hydra::CheatMetadata metadata = wrapper_->GetCheat(handle_);

        QLineEdit* txt_name = new QLineEdit;
        txt_name->setText(metadata.name.c_str());
        txt_name->setPlaceholderText(tr("Cheat name"));
        layout->addWidget(txt_name);

        QTextEdit* txt_code = new QTextEdit;
        QFont font;
        font.setFamily("Courier");
        font.setFixedPitch(true);
        font.setPointSize(10);
        txt_code->setFont(font);
        txt_code->setPlaceholderText(tr("Cheat code"));
        if (metadata.code.size() != 0)
        {
            txt_code->setText(metadata.code.c_str());
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
        is_editing_ = handle != hydra::BAD_CHEAT;
        layout->addWidget(txt_code);
        setLayout(layout);

        QDialogButtonBox* button_box =
            new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        layout->addWidget(button_box);
        connect(button_box, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(button_box, &QDialogButtonBox::rejected, this, &QDialog::reject);
        connect(this, &QDialog::accepted, this, [this, txt_name, txt_code]() {
            QString code = txt_code->toPlainText();
            code.replace(QRegularExpression("[^0-9a-fA-F]"), "");

            hydra::CheatMetadata metadata = wrapper_->GetCheat(handle_);
            metadata.name = txt_name->text().toStdString();
            metadata.code = code.toStdString();

            if (is_editing_)
            {
                handle_ = wrapper_->EditCheat(metadata, handle_);
            }
            else
            {
                if (metadata.name.empty())
                {
                    metadata.name = tr("Cheat code").toStdString();
                }
                handle_ = wrapper_->EditCheat(metadata);
            }

            entry_.SetHandle(handle_);
            entry_.Update();
        });
    }

private:
    std::shared_ptr<hydra::EmulatorWrapper> wrapper_;
    CheatEntryWidget& entry_;
    uint32_t handle_;
    bool is_editing_;
};

CheatEntryWidget::CheatEntryWidget(std::shared_ptr<hydra::EmulatorWrapper> wrapper, uint32_t handle,
                                   QListWidget* parent)
    : QWidget(), wrapper_(wrapper), handle_(handle)
{
    QHBoxLayout* layout = new QHBoxLayout;
    chk_enabled_ = new QCheckBox;

    const hydra::CheatMetadata& metadata = wrapper_->GetCheat(handle);
    chk_enabled_->setChecked(metadata.enabled);
    connect(chk_enabled_, &QCheckBox::stateChanged, this, [this](int state) {
        bool enabled = state == Qt::Checked;
        if (handle_ == hydra::BAD_CHEAT)
        {
            printf("Cheat handle is bad, this shouldn't happen\n");
            return;
        }
        if (enabled)
        {
            wrapper_->EnableCheat(handle_);
        }
        else
        {
            wrapper_->DisableCheat(handle_);
        }
    });

    lbl_name_ = new QLabel(metadata.name.c_str());
    QPushButton* btn_edit = new QPushButton(tr("Edit"));

    connect(btn_edit, &QPushButton::clicked, this, [this]() {
        CheatEditDialog* dialog = new CheatEditDialog(wrapper_, handle_, *this);
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
                           const std::filesystem::path& path, QAction* action, QWidget* parent)
    : QWidget(parent, Qt::Window), wrapper_(wrapper), cheat_path_(path), menu_action_(action)
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
        CheatEntryWidget* entry = new CheatEntryWidget(wrapper_, hydra::BAD_CHEAT, cheat_list_);
        CheatEditDialog* dialog = new CheatEditDialog(wrapper_, hydra::BAD_CHEAT, *entry);
        dialog->show();
    });

    QPushButton* btn_remove = new QPushButton(tr("Remove"));
    connect(btn_remove, &QPushButton::clicked, this, [this]() {
        QListWidgetItem* item = cheat_list_->currentItem();
        if (item == nullptr)
        {
            return;
        }

        CheatEntryWidget* entry = (CheatEntryWidget*)cheat_list_->itemWidget(item);
        wrapper_->RemoveCheat(entry->GetHandle());
        cheat_list_->takeItem(cheat_list_->row(item));
        entry->deleteLater();
    });

    button_layout->addWidget(btn_add);
    button_layout->addWidget(btn_remove);
    button_box->setLayout(button_layout);

    layout->addWidget(button_box);

    for (const hydra::CheatMetadata& metadata : wrapper_->GetCheats())
    {
        new CheatEntryWidget(wrapper_, metadata.handle, cheat_list_);
    }
    show();
}
