#include "cheatswindow.hxx"

#include "core_loader.hxx"
#include "hydra/core.hxx"
#include "json.hpp"
#include "settings.hxx"

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QRegularExpression>
#include <QTextEdit>
#include <QVBoxLayout>

static std::vector<uint8_t> cheat_to_bytes(const std::string& cheat)
{
    std::vector<uint8_t> bytes;
    for (size_t i = 0; i < cheat.size(); i += 2)
    {
        std::string hex = cheat.substr(i, 2);
        bytes.push_back((uint8_t)std::stoul(hex, nullptr, 16));
    }
    return bytes;
}

// TODO: does this close gracefully when mainwindow closes?
class CheatEditDialog : public QDialog
{
public:
    CheatEditDialog(hydra::ICheat* cheat_interface, CheatMetadata& metadata)
        : QDialog(), cheat_interface_(cheat_interface), metadata_(metadata)
    {
        setModal(true);
        QVBoxLayout* layout = new QVBoxLayout;
        QTextEdit* txt_code = new QTextEdit;
        QFont font;
        font.setFamily("Courier");
        font.setFixedPitch(true);
        font.setPointSize(10);
        txt_code->setFont(font);

        if (metadata.code.size() != 0)
        {
            printf("Setting code to %s\n", metadata.code.c_str());
            txt_code->setText(metadata.code.c_str());
        }
        connect(txt_code, &QTextEdit::textChanged, this, [this, txt_code]() {
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
    }

private:
    CheatMetadata& metadata_;
    hydra::ICheat* cheat_interface_;
    bool is_editing_;
};

class CheatWidget : public QWidget
{
public:
    CheatWidget(hydra::ICheat* cheat_interface, const CheatMetadata& metadata, QListWidget* parent)
        : QWidget(), metadata_(metadata)
    {
        QHBoxLayout* layout = new QHBoxLayout;
        QCheckBox* chk_enabled = new QCheckBox;

        connect(chk_enabled, &QCheckBox::stateChanged, this, [this](int state) {
            metadata_.enabled = state == Qt::Checked;
            if (metadata_.enabled)
            {
                cheat_interface_->enableCheat(metadata_.handle);
            }
            else
            {
                cheat_interface_->disableCheat(metadata_.handle);
            }
        });

        QLabel* lbl_name = new QLabel(metadata_.name.c_str());
        QPushButton* btn_edit = new QPushButton("Edit");

        connect(btn_edit, &QPushButton::clicked, this, [this]() {

        });

        layout->addWidget(chk_enabled);
        layout->addWidget(lbl_name);
        layout->addWidget(btn_edit);
        setLayout(layout);

        QListWidgetItem* list_item = new QListWidgetItem;
        list_item->setSizeHint(sizeHint());
        parent->addItem(list_item);
        parent->setItemWidget(list_item, this);
    }

private:
    CheatMetadata metadata_;
    hydra::ICheat* cheat_interface_;
};

CheatsWindow::CheatsWindow(std::shared_ptr<hydra::EmulatorWrapper> wrapper, bool& open,
                           const std::string& hash, QWidget* parent)
    : QWidget(parent, Qt::Window), open_(open), wrapper_(wrapper)
{
    bool just_created = false;
    if (!std::filesystem::create_directories(Settings::GetSavePath() / "cheats"))
    {
        if (!std::filesystem::exists(Settings::GetSavePath() / "cheats"))
        {
            printf("Failed to create cheats directory\n");
            return;
        }
    }
    else
    {
        just_created = true;
    }

    if (!wrapper_->shell->hasInterface(hydra::InterfaceType::ICheat))
    {
        printf("Emulator does not have cheat interface, this dialog shouldn't have been opened?\n");
        return;
    }

    auto cheat_interface = wrapper_->shell->asICheat();
    cheat_path_ = Settings::GetSavePath() / "cheats" / (hash + ".json");

    if (!just_created)
    {
        // Check if this game already has saved cheats
        if (std::filesystem::exists(cheat_path_))
        {
            // Load the cheats
            std::ifstream cheat_file(cheat_path_);
            nlohmann::json cheat_json;
            cheat_file >> cheat_json;
            for (auto& cheat : cheat_json)
            {
                CheatMetadata cheat_metadata;
                cheat_metadata.enabled = cheat["enabled"] == "true";
                cheat_metadata.name = cheat["name"];
                cheat_metadata.description = cheat["description"];
                cheat_metadata.code = cheat["code"];
                std::vector<uint8_t> bytes = cheat_to_bytes(cheat_metadata.code);
                cheat_metadata.handle = cheat_interface->addCheat(bytes.data(), bytes.size());
                if (cheat_metadata.handle != hydra::BAD_CHEAT)
                {
                    cheats_.push_back(cheat_metadata);
                }
                else
                {
                    printf("Failed to add cheat %s\n", cheat_metadata.name.c_str());
                }
            }
        }
    }

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(6, 6, 6, 6);
    setLayout(layout);

    cheats_.resize(1);
    CheatMetadata& metadata = cheats_[0];
    metadata.enabled = true;
    metadata.name = "Infinite Health";
    metadata.description = "Infinite health cheat";

    QListWidget* list = new QListWidget;
    new CheatWidget(wrapper->shell->asICheat(), metadata, list);
    layout->addWidget(list);

    QPushButton* btn_add = new QPushButton("Add");
    connect(btn_add, &QPushButton::clicked, this, [this, &metadata]() {
        CheatEditDialog* dialog = new CheatEditDialog(wrapper_->shell->asICheat(), metadata);
        dialog->show();
    });
    layout->addWidget(btn_add);

    show();
    open_ = true;
}

CheatsWindow::~CheatsWindow()
{
    // Save the cheats
    nlohmann::json cheat_json;
    for (auto& cheat : cheats_)
    {
        cheat_json.push_back({{"enabled", cheat.enabled ? "true" : "false"},
                              {"name", cheat.name},
                              {"description", cheat.description},
                              {"code", cheat.code}});
    }
    std::ofstream cheat_file(cheat_path_);
    cheat_file << cheat_json.dump(4);
}
