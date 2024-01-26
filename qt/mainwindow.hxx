#pragma once

#include "ringbuffer.hxx"
#include "screenwidget.hxx"
#include "settings.hxx"
#include "update.hxx"
#include <array>
#include <deque>
#include <hydra/core.hxx>
#include <memory>
#define MA_NO_DECODING
#define MA_NO_ENCODING
#include "cheatswindow.hxx"
#include <corewrapper.hxx>
#include <miniaudio.h>
#include <QFileDialog>
#include <QFutureWatcher>
#include <QLabel>
#include <QMainWindow>
#include <QMenuBar>
#include <QStatusBar>
#include <QVBoxLayout>
#include <thread>
#include <unordered_map>

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

    // Initialization functions
    void create_actions();
    void create_menus();

    // Menu bar actions
    void open_file();
    void open_file_impl(const std::string& file);
    void action_settings();
    void action_download_cores();
    void action_about();
    void action_scripts();
    void action_terminal();
    void action_cheats();
    void run_script(const std::string& script, bool safe_mode);
    void screenshot();
    void add_recent(const std::string& path);

    // Emulation functions
    void pause_emulator();
    void reset_emulator();
    void init_emulator();
    void stop_emulator();
    void enable_emulation_actions(bool should);
    void init_audio(hydra::SampleType sample_type = hydra::SampleType::Int16,
                    hydra::ChannelType channel_type = hydra::ChannelType::Stereo);
    void set_volume(int volume);
    void resample(void* output, const void* input, size_t frames);
    void update_recent_files();
    void update_fbo(unsigned fbo);
    void reset_emulator_windows();

    // Hydra callbacks
    static void video_callback(void* data, hydra::Size size);
    static void audio_callback(void* data, size_t frames);
    static int32_t read_input_callback(uint32_t player, hydra::ButtonType button);

private slots:
    void on_mouse_move(QMouseEvent* event);
    void on_mouse_click(QMouseEvent* event);
    void on_mouse_release(QMouseEvent* event);
    void emulator_frame();
    void update_check_finished();
    void toggle_mute();

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
    void OpenFile(const std::string& file);

private:
    // GUI
    enum WindowIndex
    {
        Cheats = 0,
        Terminal,
        Script,
        Settings,
        About,
        Downloader,
        WindowCount
    };

    std::array<std::unique_ptr<QWidget>, WindowIndex::WindowCount> windows_;
    QFutureWatcher<hydra::Updater::UpdateStatus>* update_watcher_;
    QMenu* file_menu_;
    QMenu* emulation_menu_;
    QMenu* tools_menu_;
    QMenu* help_menu_;
    QAction* open_act_;
    QAction* pause_act_;
    QAction* reset_act_;
    QAction* mute_act_;
    QAction* about_act_;
    QAction* stop_act_;
    QAction* close_act_;
    QAction* download_cores_act_;
    QAction* settings_act_;
    QAction* open_settings_file_act_;
    QAction* open_settings_folder_act_;
    QAction* screenshot_act_;
    QAction* scripts_act_;
    QAction* cheats_act_;
    QAction* terminal_act_;
    QAction* recent_act_;
    QTimer* emulator_timer_;
    ScreenWidget* screen_;

    // Common
    std::mutex emulator_mutex_;
    std::mutex audio_mutex_;
    int frame_count_ = 0;
    int sleep_time_ = 0;

    // Emulator
    std::shared_ptr<hydra::EmulatorWrapper> emulator_;
    std::unique_ptr<hydra::CoreInfo> info_;
    std::string game_hash_;
    bool paused_ = false;

    // Video
    std::vector<uint8_t> video_buffer_;
    uint32_t video_width_ = 0;
    uint32_t video_height_ = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> last_emulation_second_time_;

    // Audio
    // TODO: reduce size once done debugging
    hydra::ringbuffer<65536 * sizeof(float)> audio_buffer_;
    std::unique_ptr<ma_device, void (*)(ma_device*)> audio_device_;
    std::unique_ptr<ma_resampler, void (*)(ma_resampler*)> resampler_;
    uint8_t audio_frame_size_ = 0;

    // Input
    // Maps key -> pair<player, button>
    std::unordered_map<int, std::pair<int, hydra::ButtonType>> backwards_mappings_{};
    // Holds the current state of the input
    // vector[player][button]
    std::vector<std::array<int32_t, (int)hydra::ButtonType::InputCount>> input_state_{};
    std::deque<std::string> recent_files_;
    uint32_t mouse_state_ = hydra::TOUCH_RELEASED;

    friend void emulator_signal_handler(int);
    friend void hungry_for_more(ma_device*, void*, const void*, ma_uint32);
};
