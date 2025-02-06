#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QElapsedTimer>
#include <QHexView/qhexview.h>
#include <QHexView/model/buffer/qmemoryrefbuffer.h>
#include "config.h"
#include "devices.h"
#include "flash_algo.h"
// #include "hex_viewer.h"
#include "program_worker.h"
#include "chip_selecter.h"

#if _WIN32
#include "win_hotplug_notify.h"
#endif // _WIN32

// views
#include "chips_config_dialog.h"
#include "enum_writer_list.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QString now_time(void);
    void log_append(QString str);
    void log_debug(QString str);
    void log_info(QString str);
    void log_warn(QString str);
    void log_error(QString str);

    // void config_save(void);
    // int config_load(void);

    void set_dock_chip_info();

    bool detect_git();

    void set_hexview_line_bytes(int bytes);
    void set_hexview_group_bytes(int bytes);
    int open_firmware_file(QString file_path);

    Devices *current_device();

private slots:
    void cb_action_open_firmware_file(void);
    void cb_action_save_firmware_file(void);

    void cb_action_view_info(void);
    void cb_action_log_clear(void);

    void cb_action_hexview_goto_addr(void);
    void cb_action_hexview_goto_start(void);
    void cb_action_hexview_goto_end(void);
    void cb_action_hexview_line_bytes(void);
    void cb_action_hexview_group_bytes(void);

    void cb_action_chips(void);
    void cb_action_chip_select(void);
    void cb_action_load_flm(void);

    void cb_action_connect(void);
    void cb_action_read_chip(void);
    void cb_action_erase_chip(void);
    void cb_action_check_blank(void);
    void cb_action_write(void);
    void cb_action_verify(void);
    void cb_action_reset_run(void);

    void cb_action_enum_device_list(void);
    void cb_tick_enum_device(void);
    void cb_action_manual_refresh_enum_devices();
    void cb_action_auto_refresh_enum_devices(bool checked);

    void cb_erase_chip_finish(ProgramWorker::ChipOp op, bool ok);
    void cb_read_chip_finish(ProgramWorker::ChipOp op, bool ok);
    void cb_write_finish(ProgramWorker::ChipOp op, bool ok);
    void cb_verify_finish(ProgramWorker::ChipOp op, bool ok);

    void cb_read_chip_process(uint32_t val, uint32_t max);
    void cb_write_chip_process(uint32_t val, uint32_t max);
    void cb_verify_chip_process(uint32_t val, uint32_t max);

    void cb_usb_device_changed();

signals:
    void device_changed(DeviceList dev_list, bool changed);

    void program_worker_erase_chip(void);
    void program_worker_read_chip(QByteArray *data);
    void program_worker_write(uint32_t addr, QByteArray *data);
    void program_worker_verify(uint32_t addr, QByteArray *data);

private:
    Ui::MainWindow *ui;
    QTimer *timer_enum_device;
    QTimer *device_change_delay_enum_timer;
    QElapsedTimer take_timer;
    Config *config;

#if _WIN32
    WinHotplugNotify *win_hotplug_notify;
#endif // _WIN32

    DeviceList dap_hid_device_list_prev;
    DeviceList dap_hid_device_list;

    DeviceList dap_v2_device_list_prev;
    DeviceList dap_v2_device_list;

    DeviceList device_list;

    QLabel *label_log_ending;

    int current_device_index;

    QByteArray firmware_buf;
    QByteArray read_back_buf;

    bool force_update_device_list;

    FlashAlgo flash_algo;
    uint32_t ram_start;

    // HexViewer *hex_viewer;
    QHexView *hexview;
    QHexDocument *hexview_doc;
    
    QHexView *file_hexview;
    QHexDocument *file_hexview_doc;

    ProgramWorker *program_worker;

    ChipSelecter *dialog_chip_selecter;
    enum_writer_list *dialog_enum_devices;

    // bool dap_hid_device_list_compare(QList<DAP_HID *> *now_list, QList<DAP_HID *> *prev_list);
    int32_t load_flash_algo(QString file_path);

    int device_connect();
};
#endif // MAINWINDOW_H
