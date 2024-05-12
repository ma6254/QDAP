#include <QFileDialog>
#include <QTimer>
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "enum_writer_list.h"
#include "chip_selecter.h"
#include "dap_hid.h"
#include "utils.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    hex_viewer = new HexViewer(ui->centralwidget);

    ui->centralwidget->layout()->addWidget(hex_viewer);

    connect(ui->action_file_open, SIGNAL(triggered()), this, SLOT(cb_action_open_firmware_file(void)));
    connect(ui->action_file_save, SIGNAL(triggered()), this, SLOT(cb_action_save_firmware_file(void)));
    connect(ui->action_chip_select, SIGNAL(triggered()), this, SLOT(cb_action_chip_select(void)));
    connect(ui->action_target_connect, SIGNAL(triggered()), this, SLOT(cb_action_connect(void)));
    connect(ui->action_target_read_chip, SIGNAL(triggered()), this, SLOT(cb_action_read_chip(void)));
    connect(ui->action_target_erase_chip, SIGNAL(triggered()), this, SLOT(cb_action_erase_chip(void)));
    connect(ui->action_target_program, SIGNAL(triggered()), this, SLOT(cb_action_write(void)));
    connect(ui->action_target_run, SIGNAL(triggered()), this, SLOT(cb_action_reset_run(void)));
    connect(ui->action_enum_device_list, SIGNAL(triggered()), this, SLOT(cb_action_enum_device_list(void)));

    timer_enum_device = new QTimer();
    connect(timer_enum_device, SIGNAL(timeout()), this, SLOT(cb_tick_enum_device()));
    timer_enum_device->setInterval(500);
    timer_enum_device->start();

    ram_start = 0x20000000;

    // load_flash_algo("devices/AIR001/Air001.FLM");
    // load_flash_algo("devices/AIR001/AIR001xx_32.FLM");
    // load_flash_algo("devices/STM/STM32F4xx/STM32F4xx_256.FLM");
    load_flash_algo("C:\\Keil_v5\\ARM\\PACK\\Keil\\STM32F4xx_DFP\\2.17.1\\CMSIS\\Flash\\STM32F4xx\\STM32F4xx_128.FLM");
}

MainWindow::~MainWindow()
{
    delete ui;
}

int32_t MainWindow::load_flash_algo(QString file_path)
{
    int32_t err;

    err = flash_algo.load(file_path);
    if (err < 0)
        return err;

    return 0;
}

void MainWindow::cb_action_open_firmware_file(void)
{
    // QFileDialog dialog(this);
    // dialog.setAcceptMode(QFileDialog::AcceptOpen);
    // dialog.setWindowTitle("请选择一个烧录文件");
    // dialog.setNameFilter(tr("Firmware (*.hex *.bin)"));

    // if (!dialog.exec())
    //     return;

    QString file_name = QFileDialog::getOpenFileName(this, tr("请选择一个烧录文件"),
                                                     NULL,
                                                     tr("Firmware (*.hex *.bin)"));

    if (file_name.count() == 0)
        return;

    qDebug("[main] select a file %s", qPrintable(file_name));

    if (file_name.toLower().endsWith(".bin"))
    {
        QFile file(file_name);
        if (!file.open(QIODevice::ReadOnly))
            return;

        firmware_buf = file.readAll();
        file.close();

        QLocale locale = this->locale();
        QString valueText = locale.formattedDataSize(firmware_buf.count());

        // hex_viewer->load();
        hex_viewer->load(firmware_buf);

        qDebug("[main] file_load %s", qPrintable(valueText));
    }
    else if (file_name.toLower().endsWith(".hex"))
    {
    }
}

void MainWindow::cb_action_save_firmware_file(void)
{
    QString file_name = QFileDialog::getSaveFileName(
        this, tr("请选择烧录文件保存路径"),
        NULL,
        tr("Binary file (*.bin);;Intel HEX file (*.hex)"));

    if (file_name.count() == 0)
        return;

    qDebug("[main] save a file %s", qPrintable(file_name));

    if (file_name.toLower().endsWith(".bin"))
    {
        QFile file(file_name);

        file.remove();

        if (!file.open(QIODevice::WriteOnly))
            return;

        qint64 n = file.write(firmware_buf);
        file.close();

        QLocale locale = this->locale();
        QString valueText = locale.formattedDataSize(n);

        qDebug("[main] file_save %s", qPrintable(valueText));
    }
    else if (file_name.toLower().endsWith(".hex"))
    {
    }
}

void MainWindow::cb_action_enum_device_list(void)
{
    // DAP_HID::enum_device();

    enum_writer_list *d = new enum_writer_list(this);

    connect(this, SIGNAL(device_changed(QList<DAP_HID *>)),
            d, SLOT(cb_device_changed(QList<DAP_HID *>)));

    // force_update_device_list = true;

    d->setCurrentIndex(current_device);
    emit device_changed(dap_hid_device_list);

    d->exec();
    current_device = d->currentIndex();

    delete d;

    qDebug("[main] enum_device_list %d", current_device);
}

void MainWindow::cb_tick_enum_device(void)
{
    // qDebug("[tick_enum_device] tick");

    dap_hid_device_list.clear();
    // for (uint32_t i = 0; i < dap_hid_device_list.count(); i++)
    // {
    //     DAP_HID *tmp_dev = dap_hid_device_list.first();
    //     dap_hid_device_list.pop_front();
    //     delete tmp_dev;
    // }

    DAP_HID::enum_device(&dap_hid_device_list);

    if (dap_hid_device_list_compare(dap_hid_device_list_prev, dap_hid_device_list) || force_update_device_list)
    {
        emit device_changed(dap_hid_device_list);
    }

    // dap_hid_device_list_prev.clear();
    // for (uint32_t i = 0; i < dap_hid_device_list_prev.count(); i++)
    // {
    //     DAP_HID *tmp_dev = dap_hid_device_list_prev.first();
    //     dap_hid_device_list_prev.pop_front();
    //     delete tmp_dev;
    // }

    dap_hid_device_list_prev.append(dap_hid_device_list);

    force_update_device_list = false;
}

bool MainWindow::dap_hid_device_list_compare(QList<DAP_HID *> a_list, QList<DAP_HID *> b_list)
{
    // 比较大小
    if (a_list.count() != b_list.count())
        return 1;

    // 比较内容

    return 0;
}

void MainWindow::cb_action_chip_select(void)
{
    ChipSelecter *d = new ChipSelecter(this);

    d->exec();

    delete d;
}

void MainWindow::cb_action_connect(void)
{
    int32_t err;
    DAP_HID *tmp_dev;

    if (dap_hid_device_list.count() == 0)
        return;

    if (current_device == 0)
    {
        tmp_dev = dap_hid_device_list.at(0);
    }
    else
    {
        tmp_dev = dap_hid_device_list.at(current_device - 1);
    }

    err = tmp_dev->connect();
    if (err < 0)
    {
        qDebug("[main] connect fail");
    }

    FlashDevice tmp_flash_info = flash_algo.get_flash_device_info();
    QByteArray tmp_flash_code = flash_algo.get_flash_code();

    // Download flash programming algorithm to target and initialise.
    err = tmp_dev->dap_write_memory(ram_start,
                                    (uint8_t *)tmp_flash_code.data(),
                                    tmp_flash_code.length());
    if (err < 0)
    {
        qDebug("[main] dap_write_memory fail");
        return;
    }

    // QByteArray read_buf;
    // err = tmp_dev->dap_read_memory(ram_start,
    //                                (uint8_t *)read_buf.data(),
    //                                tmp_flash_code.length());
    // if (err < 0)
    // {
    //     qDebug("[main] dap_read_memory fail");
    //     return;
    // }

    // if (tmp_flash_code != read_buf)
    // {
    //     qDebug("[main] read back verify fail");
    //     return;
    // }

    program_syscall_t sys_call_s = flash_algo.get_sys_call_s();

    uint32_t entry = flash_algo.get_flash_func_offset(FLASH_FUNC_Init);
    uint32_t arg1 = flash_algo.get_flash_start();
    err = tmp_dev->swd_flash_syscall_exec(&sys_call_s, entry, arg1, 0, 1, 0);
    if (err < 0)
    {
        qDebug("[main] exec_flash_func Init arg:");
        qDebug("    entry: %08X", entry);
        qDebug("     arg1: %08X", arg1);
        qDebug("[main] exec_flash_func Init fail");
        return;
    }
}

void MainWindow::cb_action_read_chip(void)
{
    int err;
    // uint8_t read_buf[32 * 1024];

    firmware_buf.clear();
    firmware_buf.fill(0x00, 32 * 1024);

    qDebug("[main] read chip");

    DAP_HID *tmp_dev;

    if (dap_hid_device_list.count() == 0)
        return;

    if (current_device == 0)
    {
        tmp_dev = dap_hid_device_list.at(0);
    }
    else
    {
        tmp_dev = dap_hid_device_list.at(current_device - 1);
    }
    err = tmp_dev->dap_read_memory(0x8000000, (uint8_t *)firmware_buf.data(), firmware_buf.count());
    if (err < 0)
    {
        qDebug("[main] read chip fail");
        return;
    }

    qDebug("[main] read chip ok");
}

void MainWindow::cb_action_erase_chip(void)
{
    int err;
    DAP_HID *tmp_dev;

    if (dap_hid_device_list.count() == 0)
        return;

    if (current_device == 0)
    {
        tmp_dev = dap_hid_device_list.at(0);
    }
    else
    {
        tmp_dev = dap_hid_device_list.at(current_device - 1);
    }

    program_syscall_t sys_call_s = flash_algo.get_sys_call_s();

    uint32_t entry = flash_algo.get_flash_func_offset(FLASH_FUNC_EraseChip);
    err = tmp_dev->swd_flash_syscall_exec(&sys_call_s, entry, 0, 0, 0, 0);
    if (err < 0)
    {
        qDebug("[main] exec_flash_func EraseChip fail");
        return;
    }

    qDebug("[main] exec_flash_func EraseChip ok");
}

void MainWindow::cb_action_write(void)
{
    int err;
    DAP_HID *tmp_dev;
    uint8_t w_buf[16];
    uint8_t r_buf[sizeof(w_buf)];

    memset(r_buf, 0, sizeof(r_buf));
    for (uint8_t i = 0; i < sizeof(w_buf); i++)
    {
        if (i % 2)
        {
            w_buf[i] = 0xA0 + i;
        }
        else
        {
            w_buf[i] = 0x50 + i;
        }
    }

    if (dap_hid_device_list.count() == 0)
        return;

    if (current_device == 0)
    {
        tmp_dev = dap_hid_device_list.at(0);
    }
    else
    {
        tmp_dev = dap_hid_device_list.at(current_device - 1);
    }

    qDebug("[main] write");

    err = tmp_dev->dap_write_memory(0x20000000, w_buf, sizeof(w_buf));
    if (err < 0)
    {
        qDebug("[main] dap_write_memory fail");
        return;
    }

    err = tmp_dev->dap_read_memory(0x20000000, r_buf, sizeof(w_buf));
    // err = tmp_dev->dap_read_memory(0x8001000, r_buf, 16);
    if (err < 0)
    {
        qDebug("[main] dap_read_memory fail");
        return;
    }

    qDebug("[main] r_buf:");
    hexdump(r_buf, sizeof(r_buf));

    if (memcmp(w_buf, r_buf, sizeof(w_buf)) == 0)
    {
        qDebug("[main] ramcheck ok");
    }
    else
    {
        qDebug("[main] ramcheck fail");
    }
}

void MainWindow::cb_action_reset_run(void)
{
    int32_t err;
    DAP_HID *tmp_dev;

    if (dap_hid_device_list.count() == 0)
        return;

    if (current_device == 0)
    {
        tmp_dev = dap_hid_device_list.at(0);
    }
    else
    {
        tmp_dev = dap_hid_device_list.at(current_device - 1);
    }

    program_syscall_t sys_call_s = flash_algo.get_sys_call_s();

    uint32_t entry = flash_algo.get_flash_func_offset(FLASH_FUNC_UnInit);
    err = tmp_dev->swd_flash_syscall_exec(&sys_call_s, entry, 3, 0, 1, 0);
    if (err < 0)
    {
        qDebug("[main] exec_flash_func UnInit fail");
        return;
    }

    tmp_dev->run();
}
