#ifndef N64_DEBUGGER
#define N64_DEBUGGER
#include <QWidget>
#include <QListWidget>
#include <QGroupBox>
#include <QGridLayout>
#include <QTabWidget>
#include <memory>
#include <lib/messagequeue.hxx>
#include <include/emulator_types.hxx>
#include <N64TKP/n64_tkpwrapper.hxx>
#include <QFontDatabase>

class QLabel;
class QTextEdit;

#define N64_DEBUGGER_TABS \
    X(Registers) \
    X(Settings)

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
    void on_tab_change();
    void update_debugger_tab();
private:
    bool open_;
    bool was_paused_ = false;
    TKPEmu::EmuType emulator_type_;
    TKPEmu::N64::N64_TKPWrapper* emulator_ { nullptr };

    QListWidget* tab_list_;
    QTabWidget* tab_show_;
    QGroupBox* right_group_box_, *left_group_box_;
    const QFont fixedfont = QFontDatabase::systemFont(QFontDatabase::FixedFont);

    bool register_names_ = false;
    std::array<QLineEdit*, 32> gpr_edit_;

    void create_tabs();
    void create_registers_tab();
    void create_settings_tab();
    std::string get_gpr_name(int n);
    std::string get_gpr_value(int n);
    void register_changed(const QString&, int reg);

    #define X(name) QWidget* name##_tab; QGridLayout* name##_layout;
    N64_DEBUGGER_TABS
    #undef X
};
#endif