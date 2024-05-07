#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "dap_hid.h"
#include "flash_algo.h"

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

private slots:
    void cb_action_open_firmware_file(void);
    void cb_action_save_firmware_file(void);

    void cb_action_chip_select(void);
    void cb_action_connect(void);
    void cb_action_read_chip(void);
    void cb_action_erase_chip(void);
    void cb_action_write(void);
    void cb_action_reset_run(void);

    void cb_action_enum_device_list(void);
    void cb_tick_enum_device(void);

signals:
    void device_changed(QList<DAP_HID *> dev_list);

private:
    Ui::MainWindow *ui;
    QTimer *timer_enum_device;
    QList<DAP_HID *> dap_hid_device_list_prev;
    QList<DAP_HID *> dap_hid_device_list;

    int current_device;

    QByteArray firmware_buf;
    QString firmware_file_path;

    bool force_update_device_list;

    FlashAlgo flash_algo;
    uint32_t ram_start;

    bool dap_hid_device_list_compare(QList<DAP_HID *> a_list, QList<DAP_HID *> b_list);
    int32_t load_flash_algo(QString file_path);
};
#endif // MAINWINDOW_H
