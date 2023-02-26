#ifndef N64_DEBUGGER
#define N64_DEBUGGER
#include <QWidget>
#include <memory>
#include <lib/messagequeue.hxx>
#include <include/emulator_types.hxx>
#include <N64TKP/n64_tkpwrapper.hxx>

class QLabel;
class QTextEdit;

class N64Debugger : public QWidget {
    Q_OBJECT;
public:
    N64Debugger(bool& open, std::shared_ptr<TKPEmu::Tools::MQBase> mq, QWidget* parent = nullptr);
    ~N64Debugger();

    void SetEmulator(TKPEmu::N64::N64_TKPWrapper* emulator);
private slots:
    void test();
    void test2();
    void pi_dma();
    void si_dma();
    void vi_dma();
private:
    bool open_;
    std::shared_ptr<TKPEmu::Tools::MQBase> message_queue_;
    TKPEmu::EmuType emulator_type_;
    TKPEmu::N64::N64_TKPWrapper* emulator_;

    QLabel* pc_label_;
    QTextEdit* text_edit_;
};
#endif