#include <QFileDialog>
#include <QTimer>
#include <QDateTime>
#include <QScrollBar>
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
    ram_start = 0x20000000;
    current_device = 0;
    ui->progressBar->setHidden(true);

    hex_viewer = new HexViewer(ui->centralwidget);
    ui->centralwidget->layout()->addWidget(hex_viewer);

    connect(ui->action_file_open, SIGNAL(triggered()), this, SLOT(cb_action_open_firmware_file(void)));
    connect(ui->action_file_save, SIGNAL(triggered()), this, SLOT(cb_action_save_firmware_file(void)));

    connect(ui->action_view_info, SIGNAL(triggered()), this, SLOT(cb_action_view_info(void)));
    connect(ui->action_log_clear, SIGNAL(triggered()), this, SLOT(cb_action_log_clear(void)));

    connect(ui->action_chip_select, SIGNAL(triggered()), this, SLOT(cb_action_chip_select(void)));
    connect(ui->action_load_flm, SIGNAL(triggered()), this, SLOT(cb_action_load_flm(void)));

    connect(ui->action_target_connect, SIGNAL(triggered()), this, SLOT(cb_action_connect(void)));
    connect(ui->action_target_read_chip, SIGNAL(triggered()), this, SLOT(cb_action_read_chip(void)));
    connect(ui->action_target_erase_chip, SIGNAL(triggered()), this, SLOT(cb_action_erase_chip(void)));
    connect(ui->action_target_check_blank, SIGNAL(triggered()), this, SLOT(cb_action_check_blank(void)));
    connect(ui->action_target_program, SIGNAL(triggered()), this, SLOT(cb_action_write(void)));
    connect(ui->action_target_run, SIGNAL(triggered()), this, SLOT(cb_action_reset_run(void)));
    connect(ui->action_enum_device_list, SIGNAL(triggered()), this, SLOT(cb_action_enum_device_list(void)));

    QVBoxLayout *log_layout = ((QVBoxLayout *)ui->scrollAreaWidgetContents->layout());
    QScrollBar *log_ver_scroll = ui->scrollArea->verticalScrollBar();
    connect(log_ver_scroll, &QScrollBar::rangeChanged, [=]()
            {
                QVBoxLayout *log_layout = ((QVBoxLayout *)ui->scrollAreaWidgetContents->layout());
                uint32_t log_ver_scroll_max = log_ver_scroll->maximum();
                int n;

                if (log_ver_scroll_max == 0)
                {
                    log_layout->addWidget(label_log_ending);
                    n = log_layout->count();
                    log_layout->setStretch(n - 1, 1);
                }else  if (log_ver_scroll_max >= 20){
                    log_layout->removeWidget(label_log_ending);
                    // n = log_layout->count();
                    // log_layout->setStretch(n - 1, 0);
                }
                
                // qDebug("log_ver_scroll max:%d", log_ver_scroll->maximum());
                log_ver_scroll->setValue(log_ver_scroll->maximum()); });

    timer_enum_device = new QTimer();
    connect(timer_enum_device, SIGNAL(timeout()), this, SLOT(cb_tick_enum_device()));
    timer_enum_device->setInterval(500);
    timer_enum_device->start();

    label_log_ending = new QLabel();
    // label_log_ending->setText("--- 到底啦 ---");
    label_log_ending->setAlignment(Qt::AlignHCenter);

    // DAP_HID::enum_device(&dap_hid_device_list);
    // if (dap_hid_device_list.count() > 0)
    // {
    //     DAP_HID *tmp_dev = dap_hid_device_list.at(0);

    //     QString str = QString("%1 %2")
    //                       .arg(qPrintable(tmp_dev->dap_hid_get_manufacturer_string()))
    //                       .arg(qPrintable(tmp_dev->dap_hid_get_product_string()));

    //     ui->label_info_dev_name->setText(str);
    // }

    // load_flash_algo("devices/AIR001/Air001.FLM");
    // load_flash_algo("devices/AIR001/AIR001xx_32.FLM");
    // load_flash_algo("devices/STM/STM32F4xx/STM32F4xx_256.FLM");
    load_flash_algo("C:\\Keil_v5\\ARM\\PACK\\Keil\\STM32F4xx_DFP\\2.17.1\\CMSIS\\Flash\\STM32F4xx\\STM32F4xx_128.FLM");
}

MainWindow::~MainWindow()
{
    delete ui;
}

QString MainWindow::now_time(void)
{
    QDateTime current_date_time = QDateTime::currentDateTime();
    QString current_date = current_date_time.toString("yyyy/MM/dd hh:mm:ss");

    return current_date;
}

void MainWindow::log_append(QString str)
{
    // QString temp_str = ui->label_log->text();
    // temp_str.append(str);
    // ui->label_log->setText(temp_str);

    QVBoxLayout *log_layout = ((QVBoxLayout *)ui->scrollAreaWidgetContents->layout());
    int n;

    QLabel *tmp_label = new QLabel();
    tmp_label->setText(str);
    tmp_label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    // label->setWordWrap(true);

    n = log_layout->count();
    if (n != 0)
        log_layout->removeWidget(label_log_ending);
    log_layout->addWidget(tmp_label);
    log_layout->addWidget(label_log_ending);

    n = log_layout->count();
    log_layout->setStretch(n - 2, 0);
    log_layout->setStretch(n - 1, 1);

    QScrollBar *tmp_scroll;
    // QScrollBar *tmp_scroll = ui->scrollArea->verticalScrollBar();
    // tmp_scroll->setValue(tmp_scroll->maximum());

    tmp_scroll = ui->scrollArea->horizontalScrollBar();
    tmp_scroll->setValue(tmp_scroll->minimum());
}

void MainWindow::log_debug(QString str)
{
    log_append(QString("%1 [DEBUG] %2").arg(now_time()).arg(str));
}

void MainWindow::log_info(QString str)
{
    log_append(QString("%1 [INFO] %2").arg(now_time()).arg(str));
}

void MainWindow::log_warn(QString str)
{
    log_append(QString("<font color=\"orange\">%1 [WARN] %2</font>").arg(now_time()).arg(str));
}

void MainWindow::log_error(QString str)
{
    log_append(QString("<font color=\"red\">%1 [ERROR] %2</font>").arg(now_time()).arg(str));
}

int32_t MainWindow::load_flash_algo(QString file_path)
{
    int32_t err;

    err = flash_algo.load(file_path);
    if (err < 0)
        return err;

    log_info(QString("Load Algo: %1").arg(file_path));

    ui->label_info_chip_mfrs->setText("STM(意法半导体)");
    ui->label_info_chip_series->setText("STM32F4XX");
    ui->label_info_chip_name->setText("STM32F401CCU6");
    ui->label_info_core_type->setText("ARM M4");

    FlashDevice info = flash_algo.get_flash_device_info();
    QLocale locale = this->locale();
    QString valueText = locale.formattedDataSize(info.szDev);
    ui->label_info_chip_flash_size->setText(valueText);

    QString flash_addr_str = QString("%1").arg(info.DevAdr, 8, 16, QChar('0')).toUpper();

    log_info(QString("         Name: %1").arg(info.DevName));
    log_info(QString("    FlashAddr: 0x%1").arg(flash_addr_str));
    log_info(QString("    FlashSize: %1").arg(valueText));

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

    QFileInfo fileInfo(file_name);

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
        // hex_viewer->load(firmware_buf);

        ui->label_info_file_name->setText(fileInfo.fileName());

        ui->label_info_data_size->setText(valueText);
        qDebug("[main] file_load %s", qPrintable(valueText));

        QDateTime current_date_time = QDateTime::currentDateTime();
        QString current_date = current_date_time.toString("yyyy-MM-dd hh:mm::ss");
        ui->label_latest_load_time->setText(current_date);
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

void MainWindow::cb_action_view_info(void)
{
}

void MainWindow::cb_action_log_clear(void)
{
    // ui->label_log->setText("");
    // ui->label_log->clear();

    QVBoxLayout *log_layout = ((QVBoxLayout *)ui->scrollAreaWidgetContents->layout());

    while (1)
    {
        QLayoutItem *tmp_item = log_layout->takeAt(0);
        if (tmp_item == NULL)
            break;
        delete tmp_item->widget();
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

    if (d->exec() == QDialog::Rejected)
    {
        delete d;
        return;
    }

    current_device = d->currentIndex();

    delete d;

    qDebug("[main] enum_device_list %d", current_device);

    DAP_HID *tmp_dev = dap_hid_device_list.at(current_device - 1);

    QString str = QString("%1 %2")
                      .arg(qPrintable(tmp_dev->dap_hid_get_manufacturer_string()))
                      .arg(qPrintable(tmp_dev->dap_hid_get_product_string()));

    ui->label_info_dev_name->setText(str);
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

        if (dap_hid_device_list.count() == 0)
        {
            log_info("All devices have been removed");

            ui->label_info_dev_name->setText("N/A");
        }
        else if (dap_hid_device_list.count() == 1)
        {
            DAP_HID *tmp_dev = dap_hid_device_list.at(0);

            QString str = QString("%1 %2")
                              .arg(qPrintable(tmp_dev->dap_hid_get_manufacturer_string()))
                              .arg(qPrintable(tmp_dev->dap_hid_get_product_string()));

            ui->label_info_dev_name->setText(str);
            log_info("Detected a device: " + str);
        }

        emit device_changed(dap_hid_device_list);
    }

    // dap_hid_device_list_prev.clear();
    // for (uint32_t i = 0; i < dap_hid_device_list_prev.count(); i++)
    // {
    //     DAP_HID *tmp_dev = dap_hid_device_list_prev.first();
    //     dap_hid_device_list_prev.pop_front();
    //     delete tmp_dev;
    // }

    dap_hid_device_list_prev.clear();
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

void MainWindow::cb_action_load_flm(void)
{
    QString file_name = QFileDialog::getOpenFileName(this, tr("请选择一个算法"),
                                                     NULL,
                                                     tr("flash algo (*.flm)"));

    if (file_name.count() == 0)
        return;

    qDebug("[main] select a file %s", qPrintable(file_name));
    log_info(QString("[main] select a file %1").arg(qPrintable(file_name)));

    QFileInfo fileInfo(file_name);
}

void MainWindow::cb_action_connect(void)
{
    int32_t err;
    DAP_HID *tmp_dev;

    if (dap_hid_device_list.count() == 0)
        return;

    qDebug("[main] cb_action_connect current_device: %d", current_device);

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

    ui->label_info_idcode->setText(QString("%1").arg(tmp_dev->idcode(), 8, 16, QChar('0')).toUpper());

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
        log_error("初始化失败");
        qDebug("[main] connect fail");
    }

    QString idcode_str = QString("%1").arg(tmp_dev->idcode(), 8, 16, QChar('0')).toUpper();
    ui->label_info_idcode->setText(idcode_str);
    log_info(QString("IDCODE: 0x%1").arg(idcode_str));

    FlashDevice tmp_flash_info = flash_algo.get_flash_device_info();
    QByteArray tmp_flash_code = flash_algo.get_flash_code();

    FlashDevice flash_info = flash_algo.get_flash_device_info();
    uint32_t flash_size = flash_info.szDev;
    uint32_t flash_addr = flash_info.DevAdr;

    QString flash_addr_str = QString("%1").arg(flash_addr, 8, 16, QChar('0')).toUpper();

    QLocale locale = this->locale();
    QString flash_size_str = locale.formattedDataSize(flash_size);

    ui->label_info_data_size->setText(flash_size_str);
    qDebug("[main] read chip size: %s", qPrintable(flash_size_str));

    log_info(QString("flash_addr: 0x%1").arg(flash_addr_str));
    log_info(QString("flash_size: %1").arg(flash_size_str));

    // Download flash programming algorithm to target and initialise.
    err = tmp_dev->dap_write_memory(ram_start,
                                    (uint8_t *)tmp_flash_code.data(),
                                    tmp_flash_code.length());
    if (err < 0)
    {
        qDebug("[main] dap_write_memory fail");
        log_error("初始化失败");
        return;
    }

    program_worker = new ProgramWorker(tmp_dev, &flash_algo);

    connect(this, &MainWindow::program_worker_read_chip, program_worker, &ProgramWorker::read_chip);
    connect(program_worker, &ProgramWorker::finished, this, &MainWindow::cb_read_chip_finish);
    connect(program_worker, &ProgramWorker::process, this, &MainWindow::cb_read_chip_process);

    take_timer.restart();
    emit program_worker_read_chip();

    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(flash_size);
    ui->progressBar->setVisible(true);

    log_info("ReadChip starting...");

    // err = tmp_dev->dap_read_memory(0x8000000, (uint8_t *)firmware_buf.data(), firmware_buf.count());
    // if (err < 0)
    // {
    //     qDebug("[main] read chip fail");
    //     return;
    // }

    // QList<uint64_t> process_bar;
    // process_bar.append(0);
    // progress_dialog = new ProgressDialog(process_bar, this);
    // progress_dialog->exec();
    // delete progress_dialog;
    // progress_dialog = NULL;
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

    err = tmp_dev->connect();
    if (err < 0)
    {
        log_error("Chip connection failed");
        qDebug("[main] connect fail");
        return;
    }

    log_info("chip connect ok");

    QString idcode_str = QString("%1").arg(tmp_dev->idcode(), 8, 16, QChar('0')).toUpper();
    ui->label_info_idcode->setText(idcode_str);
    log_info(QString("IDCODE: 0x%1").arg(idcode_str));

    FlashDevice tmp_flash_info = flash_algo.get_flash_device_info();
    QByteArray tmp_flash_code = flash_algo.get_flash_code();

    // hexdump((uint8_t *)tmp_flash_code.data(), tmp_flash_code.size());

    // Download flash programming algorithm to target and initialise.
    err = tmp_dev->dap_write_memory(ram_start,
                                    (uint8_t *)tmp_flash_code.data(),
                                    tmp_flash_code.length());
    if (err < 0)
    {
        qDebug("[main] dap_write_memory fail");
        log_error("load flash algo fail");
        return;
    }

    log_info("load flash algo code ok");

    uint32_t entry = flash_algo.get_flash_func_offset(FLASH_FUNC_Init);
    uint32_t arg1 = flash_algo.get_flash_start();
    program_syscall_t sys_call_s = flash_algo.get_sys_call_s();
    err = tmp_dev->swd_flash_syscall_exec(&sys_call_s, entry, arg1, 0, 1, 0);
    if (err < 0)
    {
        qDebug("[main] exec_flash_func Init fail");
        log_error("exec FlashFunc[Init] fail");
        return;
    }

    log_info("exec FlashFunc[Init] ok");

    program_worker = new ProgramWorker(tmp_dev, &flash_algo);

    connect(this, &MainWindow::program_worker_erase_chip, program_worker, &ProgramWorker::erase_chip);
    connect(program_worker, &ProgramWorker::finished, this, &MainWindow::cb_erase_chip_finish);

    take_timer.restart();
    emit program_worker_erase_chip();

    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(0);
    ui->progressBar->setVisible(true);

    log_info("EraseChip starting...");

    // QList<uint64_t> process_bar;
    // process_bar.append(0);
    // progress_dialog = new ProgressDialog("全片擦除", process_bar, this);
    // progress_dialog->exec();
    // delete progress_dialog;
    // progress_dialog = NULL;

    // program_syscall_t sys_call_s = flash_algo.get_sys_call_s();

    // uint32_t entry = flash_algo.get_flash_func_offset(FLASH_FUNC_EraseChip);
    // if (entry == UINT32_MAX)
    // {
    //     qDebug("[main] exec_flash_func unsuport func");
    //     return;
    // }

    // err = tmp_dev->swd_flash_syscall_exec(&sys_call_s, entry, 0, 0, 0, 0);
    // if (err < 0)
    // {
    //     qDebug("[main] exec_flash_func EraseChip fail");
    //     return;
    // }

    qDebug("[main] exec_flash_func EraseChip wating...");
}

void MainWindow::cb_action_check_blank(void)
{
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
    err = tmp_dev->swd_flash_syscall_exec(&sys_call_s, entry, 3, 0, 0, 0);
    if (err < 0)
    {
        qDebug("[main] exec_flash_func UnInit fail");
        return;
    }

    tmp_dev->run();
}

void MainWindow::cb_erase_chip_finish(ProgramWorker::ChipOp op, bool ok)
{
    qDebug("[main] exec_flash_func EraseChip ok %d %d", op, ok);

    float take_sec = (float)(take_timer.elapsed()) / 1000;
    ui->progressBar->setHidden(true);

    if (ok)
    {
        log_info(QString("EraseChip done, take time: %1 secoud").arg(take_sec, 0, 'f', 2));
    }
    else
    {
        log_error("EraseChip fail");
    }

    delete program_worker;
}

void MainWindow::cb_read_chip_finish(ProgramWorker::ChipOp op, bool ok)
{
    qDebug("[main] ReadChip ok %d %d", op, ok);

    float take_sec = (float)(take_timer.elapsed()) / 1000;
    ui->progressBar->setHidden(true);

    if (ok)
    {
        log_info(QString("ReadChip done, take time: %1 secoud").arg(take_sec, 0, 'f', 2));

        // 更新固件时间
        QDateTime current_date_time = QDateTime::currentDateTime();
        QString current_date = current_date_time.toString("yyyy-MM-dd hh:mm::ss");
        ui->label_latest_load_time->setText(current_date);
    }
    else
    {
        log_error("ReadChip fail");
    }

    delete program_worker;
}

void MainWindow::cb_read_chip_process(uint32_t val, uint32_t max)
{
    ui->progressBar->setValue(val);
    // qDebug("%d/%d", val, max);
}
