#ifndef N64_DEBUGGER
#define N64_DEBUGGER
#include <QWidget>
#include <QListWidget>
#include <QGroupBox>
#include <QTabWidget>
#include <memory>
#include <lib/messagequeue.hxx>
#include <include/emulator_types.hxx>
#include <N64TKP/n64_tkpwrapper.hxx>

class QLabel;
class QTextEdit;

class N64Debugger : public QWidget {
    Q_OBJECT;
public:
    N64Debugger(bool& open, QWidget* parent = nullptr);
    ~N64Debugger();

    void SetEmulator(TKPEmu::N64::N64_TKPWrapper* emulator);
private slots:
    void pi_dma();
    void si_dma();
    void vi_dma();
private:
    bool open_;
    TKPEmu::EmuType emulator_type_;
    TKPEmu::N64::N64_TKPWrapper* emulator_;

    QListWidget* tab_list_;
    QTabWidget* tab_show_;
    QGroupBox* right_group_box_, *left_group_box_;

    void create_tabs();
    void customEvent(QEvent* event) override;
};
#endif