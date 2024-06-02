#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QElapsedTimer>
#include "dap_hid.h"
#include "flash_algo.h"
#include "hex_viewer.h"
#include "program_worker.h"

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

private slots:
    void cb_action_open_firmware_file(void);
    void cb_action_save_firmware_file(void);

    void cb_action_view_info(void);
    void cb_action_log_clear(void);

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

    void cb_erase_chip_finish(ProgramWorker::ChipOp op, bool ok);
    void cb_read_chip_finish(ProgramWorker::ChipOp op, bool ok);
    void cb_write_finish(ProgramWorker::ChipOp op, bool ok);

    void cb_read_chip_process(uint32_t val, uint32_t max);
    void cb_write_chip_process(uint32_t val, uint32_t max);

signals:
    void device_changed(QList<DAP_HID *> dev_list);

    void program_worker_erase_chip(void);
    void program_worker_read_chip(QByteArray *data);
    void program_worker_write(uint32_t addr, QByteArray *data);

private:
    Ui::MainWindow *ui;
    QTimer *timer_enum_device;
    QElapsedTimer take_timer;

    QList<DAP_HID *> dap_hid_device_list_prev;
    QList<DAP_HID *> dap_hid_device_list;

    QLabel *label_log_ending;

    int current_device;

    QByteArray firmware_buf;
    QByteArray read_back_buf;

    QString firmware_file_path;

    bool force_update_device_list;

    FlashAlgo flash_algo;
    uint32_t ram_start;

    HexViewer *hex_viewer;

    ProgramWorker *program_worker;

    bool dap_hid_device_list_compare(QList<DAP_HID *> a_list, QList<DAP_HID *> b_list);
    int32_t load_flash_algo(QString file_path);
};
#endif // MAINWINDOW_H
