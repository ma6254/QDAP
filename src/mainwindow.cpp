#include <QCoreApplication>
#include <QFileDialog>
#include <QTimer>
#include <QDateTime>
#include <QScrollBar>
#include <QMessageBox>
#include <QBuffer>
#include <QByteArray>
#include <yaml-cpp/yaml.h>
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "devices.h"
#include "utils.h"
#include "input_box_dialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ram_start = 0x20000000;
    current_device_index = -1;
    config = new Config();

    ui->progressBar->setHidden(true);

    // hex_viewer = new HexViewer(ui->centralwidget);

#if _WIN32
    win_hotplug_notify = new WinHotplugNotify(this);
    connect(win_hotplug_notify, SIGNAL(device_change()), this, SLOT(cb_usb_device_changed()));
#endif // _WIN32

    QFont font_hexview("Courier New", 10, QFont::Bold, false);

    ui->tabWidget->clear();
    // ui->tabWidget->addTab("芯片回读");
    // ui->tabWidget->addTab("文件");

    hexview = new QHexView();
    hexview->setFont(font_hexview);
    ui->tabWidget->addTab(hexview, "芯片回读");
    hexview_doc = NULL;

    file_hexview = new QHexView();
    file_hexview->setFont(font_hexview);
    ui->tabWidget->addTab(file_hexview, "烧录固件");
    file_hexview_doc = NULL;

    ui->tabWidget->addTab(new QWidget(), "RTT Viewer");

    // QByteArray data = QByteArray();
    // hexview_doc = QHexDocument::fromMemory<QMemoryRefBuffer>(data);
    // // QHexDocument *document = QHexDocument::fromFile("TA_TFT700_B01_V02.bin");
    // hexview->setDocument(hexview_doc);

    connect(ui->action_file_open, SIGNAL(triggered()), this, SLOT(cb_action_open_firmware_file(void)));
    connect(ui->action_file_save, SIGNAL(triggered()), this, SLOT(cb_action_save_firmware_file(void)));

    connect(ui->action_view_info, SIGNAL(triggered()), this, SLOT(cb_action_view_info(void)));
    connect(ui->action_log_clear, SIGNAL(triggered()), this, SLOT(cb_action_log_clear(void)));

    connect(ui->action_hexview_goto_addr, SIGNAL(triggered()), this, SLOT(cb_action_hexview_goto_addr(void)));
    connect(ui->action_hexview_goto_start, SIGNAL(triggered()), this, SLOT(cb_action_hexview_goto_start(void)));
    connect(ui->action_hexview_goto_end, SIGNAL(triggered()), this, SLOT(cb_action_hexview_goto_end(void)));

    connect(ui->action_hexview_line_bytes_16, SIGNAL(triggered()), this, SLOT(cb_action_hexview_line_bytes(void)));
    connect(ui->action_hexview_line_bytes_32, SIGNAL(triggered()), this, SLOT(cb_action_hexview_line_bytes(void)));
    connect(ui->action_hexview_line_bytes_48, SIGNAL(triggered()), this, SLOT(cb_action_hexview_line_bytes(void)));
    connect(ui->action_hexview_line_bytes_64, SIGNAL(triggered()), this, SLOT(cb_action_hexview_line_bytes(void)));

    connect(ui->action_hexview_group_bytes_1, SIGNAL(triggered()), this, SLOT(cb_action_hexview_group_bytes(void)));
    connect(ui->action_hexview_group_bytes_2, SIGNAL(triggered()), this, SLOT(cb_action_hexview_group_bytes(void)));
    connect(ui->action_hexview_group_bytes_4, SIGNAL(triggered()), this, SLOT(cb_action_hexview_group_bytes(void)));

    connect(ui->action_enum_device_list, SIGNAL(triggered()), this, SLOT(cb_action_enum_device_list(void)));
    connect(ui->action_refresh_enum_devices, SIGNAL(triggered()), this, SLOT(cb_action_manual_refresh_enum_devices(void)));

    connect(ui->action_chips, SIGNAL(triggered()), this, SLOT(cb_action_chips(void)));
    connect(ui->action_chip_select, SIGNAL(triggered()), this, SLOT(cb_action_chip_select(void)));
    connect(ui->action_load_flm, SIGNAL(triggered()), this, SLOT(cb_action_load_flm(void)));

    connect(ui->action_target_connect, SIGNAL(triggered()), this, SLOT(cb_action_connect(void)));
    connect(ui->action_target_read_chip, SIGNAL(triggered()), this, SLOT(cb_action_read_chip(void)));
    connect(ui->action_target_erase_chip, SIGNAL(triggered()), this, SLOT(cb_action_erase_chip(void)));
    connect(ui->action_target_check_blank, SIGNAL(triggered()), this, SLOT(cb_action_check_blank(void)));
    connect(ui->action_target_program, SIGNAL(triggered()), this, SLOT(cb_action_write(void)));
    connect(ui->action_target_verify, SIGNAL(triggered()), this, SLOT(cb_action_verify(void)));
    connect(ui->action_target_run, SIGNAL(triggered()), this, SLOT(cb_action_reset_run(void)));

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
                }
                else if (log_ver_scroll_max >= 20)
                {
                    log_layout->removeWidget(label_log_ending);
                    // n = log_layout->count();
                    // log_layout->setStretch(n - 1, 0);
                }

                // qDebug("log_ver_scroll max:%d", log_ver_scroll->maximum());
                log_ver_scroll->setValue(log_ver_scroll->maximum()); });

    device_change_delay_enum_timer = new QTimer();
    connect(device_change_delay_enum_timer, SIGNAL(timeout()), this, SLOT(cb_tick_enum_device()));
    device_change_delay_enum_timer->setInterval(1000);
    device_change_delay_enum_timer->setSingleShot(true);
    // timer_enum_device->start();

    timer_enum_device = new QTimer();
    connect(timer_enum_device, SIGNAL(timeout()), this, SLOT(cb_tick_enum_device()));
    timer_enum_device->setInterval(500);
    // timer_enum_device->start();

    label_log_ending = new QLabel();
    // label_log_ending->setText("--- 到底啦 ---");
    label_log_ending->setAlignment(Qt::AlignHCenter);

    log_info("application started");

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

    QFile file;
    file.setFileName(Config::get_default_path());
    if (file.exists() == false)
    {
        qDebug("[cfg] The config file does not exist and will be initialized %s", qPrintable(Config::get_default_path()));
        config = Config::get_default();
        config->to_file();

        // YAML::Node node;
        // node["notes"] = "Please do not edit this file manually.";
        // node["notes_zh_cn"] = "请不要手动编辑该文件";
        // node["latest"] = now_time().toLocal8Bit().constData();

        // YAML::Emitter emitter;
        // // emitter.SetIndent(4);
        // emitter << node;

        // QDir dir_config(config_dir_path);
        // if (!dir_config.exists())
        // {
        //     dir_config.mkdir(".");
        // }

        // file.open(QIODevice::ReadWrite);
        // file.write(emitter.c_str());
        // file.flush();
        // file.close();
    }
    else
    {
        int err = config->from_file();

        if (err < 0)
        {
            QMessageBox msgBox;
            msgBox.setText("配置文件加载失败");
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.exec();
            exit(1);
            return;
        }
    }

    set_hexview_group_bytes(config->hexview_group_bytes);
    set_hexview_line_bytes(config->hexview_line_bytes);
    hexview->setReadOnly(true);

    // log_info(QString("firmware_file_path: %1").arg(config->firmware_file_path));
    if (config->firmware_file_path.isEmpty() == false)
    {

        if (config->firmware_file_path.startsWith(QCoreApplication::applicationDirPath()))
        {
            QDir baseDir = QDir(QCoreApplication::applicationDirPath());
            QString firmware_file_rel_path = baseDir.relativeFilePath(config->firmware_file_path);
            log_info(QString("firmware_file_path: %1").arg(firmware_file_rel_path));
        }
        else
        {
            log_info(QString("firmware_file_path: %1").arg(config->firmware_file_path));
        }

        int err = open_firmware_file(config->firmware_file_path);
        if (err < 0)
        {
            QMessageBox msgBox;
            msgBox.setText("烧录文件打开失败：" + config->firmware_file_path);
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.exec();
        }
    }

    dialog_chip_selecter = new ChipSelecter(this);

    if (dialog_chip_selecter->error() <= 0)
    {
        QMessageBox msgBox;
        msgBox.setText("芯片器件库加载失败");
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        // exit(1);
        // return;

        while (1)
        {
            ChipsConfigDialog *chips_config_dialog = new ChipsConfigDialog(this);
            chips_config_dialog->set_chips_url(config->chips->url);
            chips_config_dialog->exec();

            // 取消了
            if (chips_config_dialog->result() == QDialog::Rejected)
            {
                delete chips_config_dialog;
                exit(1);
                return;
            }

            int err = dialog_chip_selecter->load_chips();
            if (err > 0)
            {
                delete chips_config_dialog;
                break;
            }

            delete chips_config_dialog;
        }
    }

    qDebug("[main] chip_selected: [%s] [%s] [%s]",
           qUtf8Printable(config->chip_vendor_name),
           qUtf8Printable(config->chip_series_name),
           qUtf8Printable(config->chip_name));

    bool ok = dialog_chip_selecter->switch_chip(config->chip_vendor_name, config->chip_series_name, config->chip_name);
    if (ok == false)
    {
        QMessageBox msgBox;
        msgBox.setText("芯片选择信息有误");
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        // exit(1);
        // return;

        dialog_chip_selecter->exec();

        // 取消了
        if (dialog_chip_selecter->result() == QDialog::Rejected)
        {
            exit(1);
            return;
        }

        qDebug("[main] chip_select %s %s %s",
               qUtf8Printable(dialog_chip_selecter->vendor_name()),
               qUtf8Printable(dialog_chip_selecter->series_name()),
               qUtf8Printable(dialog_chip_selecter->chip_name()));

        config->chip_vendor_name = dialog_chip_selecter->vendor_name();
        config->chip_series_name = dialog_chip_selecter->series_name();
        config->chip_name = dialog_chip_selecter->chip_name();

        // ui->label_info_chip_vendor->setText(chip_vendor_name);
        // ui->label_info_chip_series->setText(chip_series_name);
        // ui->label_info_chip_name->setText(chip_name);

        ChipInfo chip_info = dialog_chip_selecter->chip_info();
        // ui->label_info_chip_flash_size->setText(chip_info.flash_size_str);
        // ui->label_info_core_type->setText(chip_info.core);

        set_dock_chip_info();
        config->to_file();
    }

    qDebug("[main] load_flash_algo flm: %s", qPrintable(QString("chips") + QDir::separator() + dialog_chip_selecter->chip_info().algo));

    int32_t err = load_flash_algo(QString("chips") + QDir::separator() + dialog_chip_selecter->chip_info().algo);
    if (err < 0)
    {
        qDebug("[main] load_flash_algo fail");
        exit(1);
        return;
    }

    // qDebug("[main] set_dock_chip_info start");
    set_dock_chip_info();
    // qDebug("[main] set_dock_chip_info end");

    dialog_enum_devices = new enum_writer_list(this);
    connect(this, SIGNAL(device_changed(DeviceList, bool)),
            dialog_enum_devices, SLOT(cb_device_changed(DeviceList, bool)));

    connect(dialog_enum_devices, SIGNAL(refresh_enum_devides()), this, SLOT(cb_action_manual_refresh_enum_devices()));

    // if (config->auto_refresh_enum_devices)
    // {
    //     log_info("[cfg] enum devices auto refresh Enabled");
    //     timer_enum_device->start();
    //     dialog_enum_devices->set_btn_manual_refresh_enabled(false);
    //     ui->action_refresh_enum_devices->setDisabled(true);
    // }

    // 禁用轮询枚举
    ui->action_auto_refresh_enum_devices->setVisible(false);
    // ui->action_auto_refresh_enum_devices->setChecked(config->auto_refresh_enum_devices);
    // connect(ui->action_auto_refresh_enum_devices,
    //         SIGNAL(toggled(bool)),
    //         this,
    //         SLOT(cb_action_auto_refresh_enum_devices(bool)));

    EnumDAP *tmp_enum_dap;
    tmp_enum_dap = dialog_enum_devices->get_enum_dap();
    tmp_enum_dap->set_config_port(config->cmsis_dap_port, config->cmsis_dap_swj);
    tmp_enum_dap->set_config_clock(config->cmsis_dap_clock, config->cmsis_dap_clock_unit);

    cb_tick_enum_device();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete dialog_chip_selecter;
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

void MainWindow::set_dock_device_info()
{
    Devices *tmp_dev;

    if (device_list.count() == 0)
    {
        ui->label_info_dev_name->setText("N/A");
        ui->label_device_type->setText("N/A");
        return;
    }

    if (device_list.count() == 1)
    {
        tmp_dev = device_list.at(0);
    }
    else
    {
        if (current_device_index == -1)
        {
            tmp_dev = device_list.at(0);
        }
        else if (current_device_index >= device_list.count())
        {
            tmp_dev = device_list.at(device_list.count() - 1);
        }
        else
        {
            tmp_dev = device_list.at(current_device_index);
        }
    }

    QString dev_name = QString("[%1] %2").arg(tmp_dev->get_manufacturer_string()).arg(tmp_dev->get_product_string());
    QString dev_type = QVariant::fromValue(tmp_dev->type()).toString();

    ui->label_info_dev_name->setText(dev_name);
    ui->label_device_type->setText(dev_type);

    log_info(QString("current device %1 [%2] [%3] %4")
                 .arg(current_device_index)
                 .arg(dev_type)
                 .arg(tmp_dev->get_manufacturer_string())
                 .arg(tmp_dev->get_product_string()));
}

void MainWindow::set_dock_chip_info()
{
    // qDebug("[main] set_dock_chip_info chip_vendor_name");

    if (dialog_chip_selecter->vendor_homepage().isEmpty() == false)
    {
        ui->label_info_chip_vendor->setText(QString::asprintf("[%s](%s)",
                                                              qUtf8Printable(config->chip_vendor_name),
                                                              qUtf8Printable(dialog_chip_selecter->vendor_homepage())));
    }
    else
    {
        ui->label_info_chip_vendor->setText(config->chip_vendor_name);
    }

    // qDebug("[main] set_dock_chip_info chip_series_name");

    if (dialog_chip_selecter->series_homepage().isEmpty() == false)
    {
        ui->label_info_chip_series->setText(QString::asprintf("[%s](%s)",
                                                              qUtf8Printable(config->chip_series_name),
                                                              qUtf8Printable(dialog_chip_selecter->series_homepage())));
    }
    else
    {
        ui->label_info_chip_series->setText(config->chip_series_name);
    }

    // qDebug("[main] set_dock_chip_info chip_name");

    ui->label_info_chip_name->setText(config->chip_name);

    log_info(QString::asprintf("[cfg] chip_selected: [%s] [%s] [%s]",
                               qUtf8Printable(config->chip_vendor_name),
                               qUtf8Printable(config->chip_series_name),
                               qUtf8Printable(config->chip_name)));

    // qDebug("[main] set_dock_chip_info chip_info");

    ChipInfo chip_info = dialog_chip_selecter->chip_info();
    ui->label_info_chip_flash_size->setText(chip_info.flash_size_str);

    QString tmp_core_homepage = dialog_chip_selecter->core_homepage(chip_info.core);

    if (tmp_core_homepage.isEmpty() == false)
    {
        ui->label_info_core_type->setText(QString::asprintf("[%s](%s)",
                                                            qUtf8Printable(chip_info.core),
                                                            qUtf8Printable(tmp_core_homepage)));
    }
    else
    {
        ui->label_info_core_type->setText(chip_info.core);
    }

    if (config->firmware_file_path.isEmpty() == false)
    {
        ui->label_info_file_name->setText(config->firmware_file_path);
    }
    else
    {
        ui->label_info_file_name->setText("N/A");
    }
}

void MainWindow::set_hexview_line_bytes(int bytes)
{
    if ((bytes != 16) && (bytes != 32) && (bytes != 48) && (bytes != 64))
        bytes = 1;

    config->hexview_line_bytes = bytes;
    hexview->setLineLength(config->hexview_line_bytes);

    ui->action_hexview_line_bytes_16->setChecked(bytes == 16);
    ui->action_hexview_line_bytes_32->setChecked(bytes == 32);
    ui->action_hexview_line_bytes_48->setChecked(bytes == 48);
    ui->action_hexview_line_bytes_64->setChecked(bytes == 64);
}

void MainWindow::set_hexview_group_bytes(int bytes)
{
    if ((bytes != 1) && (bytes != 2) && (bytes != 4))
        bytes = 1;

    config->hexview_group_bytes = bytes;
    hexview->setGroupLength(config->hexview_group_bytes);

    ui->action_hexview_group_bytes_1->setChecked(bytes == 1);
    ui->action_hexview_group_bytes_2->setChecked(bytes == 2);
    ui->action_hexview_group_bytes_4->setChecked(bytes == 4);
}

int MainWindow::open_firmware_file(QString file_name)
{
    QFileInfo fileInfo(file_name);

    if (file_name.toLower().endsWith(".bin"))
    {
        QFile file(file_name);
        if (file.open(QIODevice::ReadOnly) == false)
            return -1;

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

        if (file_hexview_doc == NULL)
        {
            file_hexview_doc = QHexDocument::fromMemory<QMemoryRefBuffer>(firmware_buf);
            file_hexview->setDocument(file_hexview_doc);
        }
        else
        {
            QHexDocument *new_file_hexview_doc = QHexDocument::fromMemory<QMemoryRefBuffer>(firmware_buf);
            file_hexview->setDocument(new_file_hexview_doc);

            delete hexview_doc;
            file_hexview_doc = new_file_hexview_doc;
        }
    }
    else if (file_name.toLower().endsWith(".hex"))
    {
        return -1;
    }

    return 0;
}

Devices *MainWindow::current_device()
{
    int32_t err;
    Devices *tmp_dev;

    if (device_list.count() == 0)
    {
        log_info("no devices");
        return NULL;
    }

    // qDebug("[main] cb_action_connect current_device: inedx:%d len:%d", current_device_index, device_list.count());

    if (current_device_index < 0)
    {
        qDebug("[main] current_device index:%d default", current_device_index);
        tmp_dev = device_list[0];
    }
    else if (current_device_index >= device_list.count())
    {
        qDebug("[main] current_device inedx:%d len:%d", current_device_index, device_list.count());
        tmp_dev = device_list[0];
    }
    else
    {
        qDebug("[main] current_device index:%d", current_device_index);
        tmp_dev = device_list[current_device_index];
    }

    return tmp_dev;
}

int32_t MainWindow::load_flash_algo(QString file_path)
{
    int32_t err;

    err = flash_algo.load(file_path);
    if (err < 0)
        return err;

    log_info(QString("Load Algo: %1").arg(file_path));

    FlashDevice info = flash_algo.get_flash_device_info();
    QLocale locale = this->locale();
    QString flash_size_str = locale.formattedDataSize(info.szDev);
    // ui->label_info_chip_flash_size->setText(valueText);

    QString flash_addr_str = QString("%1").arg(info.DevAdr, 8, 16, QChar('0')).toUpper();

    log_info(QString("         Name: %1").arg(info.DevName));
    log_info(QString("    FlashAddr: 0x%1").arg(flash_addr_str));
    log_info(QString("    FlashSize: %1").arg(flash_size_str));

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

    if (file_name.startsWith(QCoreApplication::applicationDirPath()))
    {
        QDir baseDir = QDir(QCoreApplication::applicationDirPath());
        file_name = baseDir.relativeFilePath(file_name);
    }

    qDebug("[main] select a file %s", qPrintable(file_name));
    config->firmware_file_path = file_name;
    config->to_file();

    open_firmware_file(file_name);
}

void MainWindow::cb_action_save_firmware_file(void)
{
    QString file_name = QFileDialog::getSaveFileName(
        this, tr("请回读数据保存文件路径"),
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

        qint64 n = file.write(read_back_buf);
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

void MainWindow::cb_action_hexview_goto_addr(void)
{
    InputBoxDialog *dialog = new InputBoxDialog();

    dialog->set_description_text("跳转到地址: ");
    dialog->set_default_value("1");

    dialog->exec();

    // 取消了
    if (dialog->result() == QDialog::Rejected)
    {
        delete dialog;
        return;
    }

    qDebug("[hexview] goto_addr: %s", qPrintable(dialog->get_value()));

    QString addr_str = dialog->get_value();
    delete dialog;

    int addr = 0;
    bool is_negative = false;
    bool is_hex = false;
    bool ok = false;

    if (addr_str.startsWith("-"))
    {
        is_negative = true;
        addr_str = addr_str.mid(1);
    }

    if (addr_str.toLower().startsWith("0x"))
    {
        is_hex = true;
        addr_str = addr_str.mid(2);
    }

    if (is_hex)
    {
        addr = addr_str.toInt(&ok, 16);
    }
    else
    {
        addr = addr_str.toInt(&ok, 10);
    }

    if (ok == false)
        return;

    if (is_negative)
    {
        addr = firmware_buf.length() - addr;

        qDebug("[hexview] goto_addr 0x%X", addr);
    }

    QHexPosition pos = hexview->positionFromAddress(addr);
    qDebug("hexview_pos: %d %d", pos.line, pos.column);
    hexview->verticalScrollBar()->setValue(pos.line);
}

void MainWindow::cb_action_hexview_goto_start(void)
{
    hexview->verticalScrollBar()->setValue(0);
}

void MainWindow::cb_action_hexview_goto_end(void)
{
    hexview->verticalScrollBar()->setValue(hexview->lines());
}

void MainWindow::cb_action_hexview_line_bytes(void)
{
    QAction *sender_action = qobject_cast<QAction *>(sender());

    if (sender_action == ui->action_hexview_line_bytes_16)
    {
        set_hexview_line_bytes(16);
        config->to_file();
    }
    else if (sender_action == ui->action_hexview_line_bytes_32)
    {
        set_hexview_line_bytes(32);
        config->to_file();
    }
    else if (sender_action == ui->action_hexview_line_bytes_48)
    {
        set_hexview_line_bytes(48);
        config->to_file();
    }
    else if (sender_action == ui->action_hexview_line_bytes_64)
    {
        set_hexview_line_bytes(64);
        config->to_file();
    }
}

void MainWindow::cb_action_hexview_group_bytes(void)
{
    QAction *sender_action = qobject_cast<QAction *>(sender());

    if (sender_action == ui->action_hexview_group_bytes_1)
    {
        set_hexview_group_bytes(1);
        config->to_file();
    }
    else if (sender_action == ui->action_hexview_group_bytes_2)
    {
        set_hexview_group_bytes(2);
        config->to_file();
    }
    else if (sender_action == ui->action_hexview_group_bytes_4)
    {
        set_hexview_group_bytes(4);
        config->to_file();
    }
}

void MainWindow::cb_action_enum_device_list(void)
{
    // DAP_HID::enum_device();

    // force_update_device_list = true;

    // cb_tick_enum_device();

    // emit device_changed(device_list);
    dialog_enum_devices->setCurrentIndex(current_device_index + 1);
    dialog_enum_devices->set_auto_refresh(config->auto_refresh_enum_devices);

    dialog_enum_devices->exec();
    if (dialog_enum_devices->result() == QDialog::Rejected)
    {
        return;
    }

    Devices *tmp_dev = dialog_enum_devices->current_device();

    if (tmp_dev == NULL)
    {
        return;
    }

    QString tmp_str = QString::asprintf(
        "%s %s",
        qPrintable(tmp_dev->get_manufacturer_string()),
        qPrintable(tmp_dev->get_product_string()));
    ui->label_info_dev_name->setText(tmp_str);

    ui->label_device_type->setText(tmp_dev->type_str());

    EnumDAP *tmp_enum_dap = dialog_enum_devices->get_enum_dap();
    config->cmsis_dap_port = tmp_enum_dap->get_config_port();
    config->cmsis_dap_swj = tmp_enum_dap->get_config_swj();
    config->cmsis_dap_clock = tmp_enum_dap->get_config_clock();
    config->cmsis_dap_clock_unit = tmp_enum_dap->get_config_clock_unit();
    config->to_file();

    current_device_index = dialog_enum_devices->currentIndex();

    set_dock_device_info();
    // qDebug("[main] enum_device_list %d", current_device);

    // DAP_HID *tmp_dev = dap_hid_device_list.at(current_device - 1);

    // QString str = QString("%1 %2")
    //                   .arg(qPrintable(tmp_dev->dap_hid_get_manufacturer_string()))
    //                   .arg(qPrintable(tmp_dev->dap_hid_get_product_string()));

    // ui->label_info_dev_name->setText(str);
}

void MainWindow::cb_tick_enum_device()
{
    // qDebug("[tick_enum_device] tick");

    ui->action_refresh_enum_devices->setEnabled(true);

    bool is_changed = false;
    // for (uint32_t i = 0; i < dap_hid_device_list.count(); i++)
    // {
    //     DAP_HID *tmp_dev = dap_hid_device_list.first();
    //     dap_hid_device_list.pop_front();
    //     delete tmp_dev;
    // }

    // for (uint32_t i = 0; i < dap_hid_device_list.count(); i++)
    // {
    //     delete dap_hid_device_list[i];
    // }
    // dap_hid_device_list.clear();

    // qDebug("[main] cb_tick_enum_device DAP_HID");
    DAP_HID::enum_device(&dap_hid_device_list);
    if (Devices::device_list_compare(dap_hid_device_list, dap_hid_device_list_prev))
    {
        is_changed = true;
    }

    // dap_hid_device_list_prev.clear();
    dap_hid_device_list_prev.release_all();
    dap_hid_device_list_prev.append(dap_hid_device_list);

    // if (dap_hid_device_list.count() == 0)
    // {
    //     // log_info("All devices have been removed");

    //     ui->label_info_dev_name->setText("N/A");
    //     ui->label_info_idcode->setText("N/A");
    // }
    // else if (dap_hid_device_list.count() == 1)
    // {
    //     DAP_HID *tmp_dev = dap_hid_device_list.at(0);

    //     QString str = QString("%1 %2")
    //                       .arg(qPrintable(tmp_dev->dap_hid_get_manufacturer_string()))
    //                       .arg(qPrintable(tmp_dev->dap_hid_get_product_string()));

    //     ui->label_info_dev_name->setText(str);
    //     // log_info("Detected a dap_usb_hid device: " + str);
    // }

    // emit device_changed(dap_hid_device_list);
    // }

    // qDebug("[main] cb_tick_enum_device CMSIS_DAP_V2");
    CMSIS_DAP_V2::enum_device(&dap_v2_device_list);
    if (Devices::device_list_compare(dap_v2_device_list, dap_v2_device_list_prev))
    {
        is_changed = true;
    }

    dap_v2_device_list_prev.release_all();
    // dap_v2_device_list_prev.clear();
    dap_v2_device_list_prev.append(dap_v2_device_list);

    // dap_v2_device_list.clear();
    // CMSIS_DAP_V2::enum_device(&dap_v2_device_list);
    // if (CMSIS_DAP_V2::device_list_compare(dap_v2_device_list_prev, dap_v2_device_list) || force_update_device_list)
    // {
    //     is_changed = true;

    //     if (dap_v2_device_list.count() == 0)
    //     {
    //         log_info("[CMSIS_DAP_V2] All devices have been removed");

    //         ui->label_info_dev_name->setText("N/A");
    //         ui->label_info_idcode->setText("N/A");
    //     }
    //     else if (dap_v2_device_list.count() == 1)
    //     {
    //         CMSIS_DAP_V2 *tmp_dev = dap_v2_device_list.at(0);

    //         QString str = QString("%1 %2")
    //                           .arg(qPrintable(tmp_dev->get_manufacturer()))
    //                           .arg(qPrintable(tmp_dev->get_product()));

    //         // QString str = QString("%1 %2 %3")
    //         //                   .arg(qPrintable(tmp_dev->dap_hid_get_manufacturer_string()))
    //         //                   .arg(qPrintable(tmp_dev->dap_hid_get_product_string()))
    //         //                   .arg(qPrintable(tmp_dev->dap_get_info_cmsis_dap_protocol_version()));

    //         ui->label_info_dev_name->setText(str);
    //         log_info("Detected a dap_usb_bulk device: " + str);
    //     }
    // }
    // dap_v2_device_list_prev.clear();
    // dap_v2_device_list_prev.append(dap_v2_device_list);

    device_list.clear();
    device_list.append(dap_hid_device_list);
    device_list.append(dap_v2_device_list);

    if (is_changed)
    {
        // qDebug("device_list is changed len:%d", device_list.count());
        log_info(QString::asprintf("device_list is changed len:%d", device_list.count()));
    }

    set_dock_device_info();

    emit device_changed(device_list, is_changed);
}

void MainWindow::cb_action_manual_refresh_enum_devices()
{
    device_change_delay_enum_timer->start();
    ui->action_refresh_enum_devices->setEnabled(false);

    // cb_tick_enum_device();
}

void MainWindow::cb_action_auto_refresh_enum_devices(bool checked)
{
    if (checked)
    {
        log_info("[cfg] enum devices auto refresh Enabled");
        timer_enum_device->start();
        dialog_enum_devices->set_btn_manual_refresh_enabled(false);
        ui->action_refresh_enum_devices->setDisabled(true);
    }
    else
    {
        log_info("[cfg] enum devices auto refresh Disabled");
        timer_enum_device->stop();
        dialog_enum_devices->set_btn_manual_refresh_enabled(true);
        ui->action_refresh_enum_devices->setEnabled(true);
    }

    config->auto_refresh_enum_devices = checked;
    config->to_file();
}

// bool MainWindow::dap_hid_device_list_compare(QList<DAP_HID *> *now_list, QList<DAP_HID *> *prev_list)
// {
//     bool result = false;

//     // for (uint32_t prev_i = 0; prev_i < prev_list->count(); prev_i++)
//     // {
//     //     bool is_find = false;

//     //     for (uint32_t now_i = 0; now_i < now_list->count(); now_i++)
//     //     {
//     //         if (prev_list->at(prev_i)->get_usb_path() == now_list->at(now_i)->get_usb_path())
//     //         {
//     //             is_find = true;
//     //             break;
//     //         }
//     //     }

//     //     if (is_find == false)
//     //     {
//     //         delete prev_list->at(prev_i);
//     //     }
//     // }

//     // 比较大小
//     if (now_list->count() != prev_list->count())
//     {
//         result = true;
//     }
//     // 比较内容
//     prev_list->clear();
//     prev_list->append(*now_list);

//     return result;
// }

void MainWindow::cb_action_chips(void)
{
    ChipsConfigDialog *chips_config_dialog = new ChipsConfigDialog(this);
    chips_config_dialog->set_chips_url(config->chips->url);
    chips_config_dialog->exec();
    int result = chips_config_dialog->result();

    if (result == QDialog::Rejected)
    {
        delete chips_config_dialog;
        return;
    }

    delete chips_config_dialog;
    config->to_file();
}

void MainWindow::cb_action_chip_select(void)
{
    dialog_chip_selecter->exec();
    int result = dialog_chip_selecter->result();

    if (result == QDialog::Rejected)
    {
        return;
    }

    qDebug("[main] chip_select %s %s %s",
           qUtf8Printable(dialog_chip_selecter->vendor_name()),
           qUtf8Printable(dialog_chip_selecter->series_name()),
           qUtf8Printable(dialog_chip_selecter->chip_name()));

    config->chip_vendor_name = dialog_chip_selecter->vendor_name();
    config->chip_series_name = dialog_chip_selecter->series_name();
    config->chip_name = dialog_chip_selecter->chip_name();

    // ui->label_info_chip_vendor->setText(chip_vendor_name);
    // ui->label_info_chip_series->setText(chip_series_name);
    // ui->label_info_chip_name->setText(chip_name);

    ChipInfo chip_info = dialog_chip_selecter->chip_info();
    // ui->label_info_chip_flash_size->setText(chip_info.flash_size_str);
    // ui->label_info_core_type->setText(chip_info.core);

    load_flash_algo(QString("chips") + QDir::separator() + chip_info.algo);

    set_dock_chip_info();
    config->to_file();
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
    int err;

    err = device_connect();
    if (err < 0)
    {
        return;
    }

    // FlashDevice tmp_flash_info = flash_algo.get_flash_device_info();
    // QByteArray tmp_flash_code = flash_algo.get_flash_code();

    // // Download flash programming algorithm to target and initialise.
    // err = tmp_dev->dap_write_memory(ram_start,
    //                                 (uint8_t *)tmp_flash_code.data(),
    //                                 tmp_flash_code.length());
    // if (err < 0)
    // {
    //     qDebug("[main] dap_write_memory fail");
    //     return;
    // }

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

    // program_syscall_t sys_call_s = flash_algo.get_sys_call_s();

    // uint32_t entry = flash_algo.get_flash_func_offset(FLASH_FUNC_Init);
    // uint32_t arg1 = flash_algo.get_flash_start();
    // err = tmp_dev->swd_flash_syscall_exec(&sys_call_s, entry, arg1, 0, 1, 0);
    // if (err < 0)
    // {
    //     qDebug("[main] exec_flash_func Init arg:");
    //     qDebug("    entry: %08X", entry);
    //     qDebug("     arg1: %08X", arg1);
    //     qDebug("[main] exec_flash_func Init fail");
    //     return;
    // }
}

void MainWindow::cb_action_read_chip(void)
{
    int err;
    // uint8_t read_buf[32 * 1024];
    Devices *tmp_dev;

    err = device_connect();
    if (err < 0)
    {
        return;
    }

    tmp_dev = current_device();
    if (tmp_dev == NULL)
    {
        return;
    }

    FlashDevice tmp_flash_info = flash_algo.get_flash_device_info();
    // QByteArray tmp_flash_code = flash_algo.get_flash_code();
    uint32_t flash_size = tmp_flash_info.szDev;
    uint32_t flash_addr = tmp_flash_info.DevAdr;

    qDebug("[main] cb_action_read_chip %s 0x%08X 0x%08X", tmp_flash_info.DevName, flash_addr, flash_size);

    // FlashDevice flash_info = flash_algo.get_flash_device_info();
    // uint32_t flash_size = flash_info.szDev;
    // uint32_t flash_addr = flash_info.DevAdr;

    // QString flash_addr_str = QString("%1").arg(flash_addr, 8, 16, QChar('0')).toUpper();

    // QLocale locale = this->locale();
    // QString flash_size_str = locale.formattedDataSize(flash_size);

    // ui->label_info_data_size->setText(flash_size_str);
    // qDebug("[main] read chip size: %s", qPrintable(flash_size_str));

    // log_info(QString("flash_addr: 0x%1").arg(flash_addr_str));
    // log_info(QString("flash_size: %1").arg(flash_size_str));

    // // Download flash programming algorithm to target and initialise.
    // err = tmp_dev->dap_write_memory(ram_start,
    //                                 (uint8_t *)tmp_flash_code.data(),
    //                                 tmp_flash_code.length());
    // if (err < 0)
    // {
    //     qDebug("[main] dap_write_memory fail");
    //     log_error("初始化失败");
    //     return;
    // }

    program_worker = new ProgramWorker(tmp_dev, &flash_algo);
    connect(this, &MainWindow::program_worker_read_chip, program_worker, &ProgramWorker::read_chip);
    connect(program_worker, &ProgramWorker::finished, this, &MainWindow::cb_read_chip_finish);
    connect(program_worker, &ProgramWorker::process, this, &MainWindow::cb_read_chip_process);

    take_timer.restart();
    emit program_worker_read_chip(&read_back_buf);

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
    Devices *tmp_dev;

    log_info("start chip erase");

    err = device_connect();
    if (err < 0)
    {
        return;
    }

    tmp_dev = current_device();
    if (tmp_dev == NULL)
    {
        return;
    }

    log_info("chip connect ok");
    qDebug("[main] chip connect ok");

    FlashDevice tmp_flash_info = flash_algo.get_flash_device_info();
    QByteArray tmp_flash_code = flash_algo.get_flash_code();

    // hexdump((uint8_t *)tmp_flash_code.data(), tmp_flash_code.size());

    // Download flash programming algorithm to target and initialise.
    err = tmp_dev->chip_write_memory(ram_start,
                                     (uint8_t *)tmp_flash_code.data(),
                                     tmp_flash_code.length());
    if (err < 0)
    {
        qDebug("[main] cb_action_erase_chip load_flash_algo_code fail");
        log_error("load flash algo fail");
        return;
    }

    log_info("load flash algo code ok");
    qDebug("[main] cb_action_erase_chip load_flash_algo_code ok");

    uint32_t entry = flash_algo.get_flash_func_offset(FLASH_FUNC_Init);
    uint32_t arg1 = flash_algo.get_flash_start();
    program_syscall_t sys_call_s = flash_algo.get_sys_call_s();
    err = tmp_dev->chip_syscall_exec(&sys_call_s, entry, arg1, 0, 1, 0);
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

    // qDebug("[main] exec_flash_func EraseChip wating...");
}

void MainWindow::cb_action_check_blank(void)
{
}

void MainWindow::cb_action_write(void)
{
    int err;
    Devices *tmp_dev;

    if (firmware_buf.length() == 0)
    {
        QMessageBox msgBox;
        msgBox.setText("请导入一个数据文件");
        msgBox.exec();

        return;
    }

    err = device_connect();
    if (err < 0)
    {
        return;
    }

    tmp_dev = current_device();
    if (tmp_dev == NULL)
    {
        return;
    }

    log_info("chip connect ok");

    FlashDevice tmp_flash_info = flash_algo.get_flash_device_info();
    QByteArray tmp_flash_code = flash_algo.get_flash_code();

    FlashDevice flash_info = flash_algo.get_flash_device_info();
    uint32_t flash_size = flash_info.szDev;
    uint32_t flash_addr = flash_info.DevAdr;

    // hexdump((uint8_t *)tmp_flash_code.data(), tmp_flash_code.size());

    // Download flash programming algorithm to target and initialise.
    err = tmp_dev->chip_write_memory(ram_start,
                                     (uint8_t *)tmp_flash_code.data(),
                                     tmp_flash_code.length());
    if (err < 0)
    {
        qDebug("[main] dap_write_memory fail");
        log_error("load flash algo fail");
        return;
    }

    // log_info("load flash algo code ok");

    uint32_t entry = flash_algo.get_flash_func_offset(FLASH_FUNC_Init);
    uint32_t arg1 = flash_algo.get_flash_start();
    program_syscall_t sys_call_s = flash_algo.get_sys_call_s();
    err = tmp_dev->chip_syscall_exec(&sys_call_s, entry, arg1, 0, 1, 0);
    if (err < 0)
    {
        qDebug("[main] exec_flash_func Init fail");
        log_error("exec FlashFunc[Init] fail");
        return;
    }

    log_info("exec FlashFunc[Init] ok");

    program_worker = new ProgramWorker(tmp_dev, &flash_algo);

    connect(this, &MainWindow::program_worker_write, program_worker, &ProgramWorker::write);
    connect(program_worker, &ProgramWorker::finished, this, &MainWindow::cb_write_finish);
    connect(program_worker, &ProgramWorker::process, this, &MainWindow::cb_write_chip_process);

    take_timer.restart();
    emit program_worker_write(flash_addr, &firmware_buf);

    uint32_t w_size = (flash_size > firmware_buf.length()) ? firmware_buf.length() : flash_size;

    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(w_size);
    ui->progressBar->setVisible(true);

    log_info("write starting...");
}

void MainWindow::cb_action_verify(void)
{
    int32_t err;
    Devices *tmp_dev;

    err = device_connect();
    if (err < 0)
    {
        return;
    }

    tmp_dev = current_device();
    if (tmp_dev == NULL)
    {
        return;
    }

    log_info("chip connect ok");

    FlashDevice flash_info = flash_algo.get_flash_device_info();
    uint32_t flash_size = flash_info.szDev;
    uint32_t flash_addr = flash_info.DevAdr;

    program_worker = new ProgramWorker(tmp_dev, &flash_algo);
    connect(this, &MainWindow::program_worker_verify, program_worker, &ProgramWorker::verify);
    connect(program_worker, &ProgramWorker::finished, this, &MainWindow::cb_verify_finish);
    connect(program_worker, &ProgramWorker::process, this, &MainWindow::cb_verify_chip_process);

    take_timer.restart();
    emit program_worker_verify(flash_addr, &firmware_buf);

    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(firmware_buf.length());
    ui->progressBar->setVisible(true);

    log_info("Verify starting...");
}

void MainWindow::cb_action_reset_run(void)
{
    int32_t err;
    Devices *tmp_dev;

    if (dap_hid_device_list.count() == 0)
        return;

    err = device_connect();
    if (err < 0)
    {
        return;
    }

    tmp_dev = current_device();
    if (tmp_dev == NULL)
    {
        return;
    }

    // if (current_device == 0)
    // {
    //     tmp_dev = dap_hid_device_list.at(0);
    // }
    // else
    // {
    //     tmp_dev = dap_hid_device_list.at(current_device - 1);
    // }

    // program_syscall_t sys_call_s = flash_algo.get_sys_call_s();

    // uint32_t entry = flash_algo.get_flash_func_offset(FLASH_FUNC_UnInit);
    // err = tmp_dev->swd_flash_syscall_exec(&sys_call_s, entry, 3, 0, 0, 0);
    // if (err < 0)
    // {
    //     qDebug("[main] exec_flash_func UnInit fail");
    //     return;
    // }

    err = tmp_dev->run();
    if (err < 0)
    {
        qDebug("[main] run fail");
        return;
    }

    // log_info("Chip is reset and runnig");
}

void MainWindow::cb_erase_chip_finish(ProgramWorker::ChipOp op, bool ok)
{
    qDebug("[main] exec_flash_func EraseChip ok %d %d", op, ok);

    float take_sec = (float)(take_timer.elapsed()) / 1000;
    ui->progressBar->setHidden(true);

    if (ok)
    {
        log_info(QString("EraseChip done, take time: %1 second").arg(take_sec, 0, 'f', 2));
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

    FlashDevice tmp_flash_info = flash_algo.get_flash_device_info();
    // QByteArray tmp_flash_code = flash_algo.get_flash_code();
    uint32_t flash_size = tmp_flash_info.szDev;

    float take_sec = (float)(take_timer.elapsed()) / 1000;
    ui->progressBar->setHidden(true);

    QString bytes_speed_str = convert_bytes_speed_unit((float)(flash_size) / take_sec);

    if (ok)
    {
        log_info(QString("ReadChip done, take time: %1 second %2").arg(take_sec, 0, 'f', 2).arg(bytes_speed_str));

        // 更新固件时间
        QDateTime current_date_time = QDateTime::currentDateTime();
        QString current_date = current_date_time.toString("yyyy-MM-dd hh:mm::ss");
        ui->label_latest_load_time->setText(current_date);

        if (hexview_doc == NULL)
        {
            hexview_doc = QHexDocument::fromMemory<QMemoryRefBuffer>(read_back_buf);
            hexview->setDocument(hexview_doc);
        }
        else
        {
            QHexDocument *new_hexview_doc = QHexDocument::fromMemory<QMemoryRefBuffer>(read_back_buf);
            hexview->setDocument(new_hexview_doc);

            delete hexview_doc;
            hexview_doc = new_hexview_doc;
        }

        hexview->setBaseAddress(flash_algo.get_flash_start());
        hexview->setAddressWidth(8);

        // set_hexview_group_bytes(config->hexview_group_bytes);
        // set_hexview_line_bytes(config->hexview_line_bytes);
        // hexview->setReadOnly(true);
    }
    else
    {
        log_error("ReadChip fail");
    }

    delete program_worker;
}

void MainWindow::cb_write_finish(ProgramWorker::ChipOp op, bool ok)
{
    float take_sec = (float)(take_timer.elapsed()) / 1000;
    ui->progressBar->setHidden(true);

    FlashDevice flash_info = flash_algo.get_flash_device_info();
    uint32_t flash_size = flash_info.szDev;
    uint32_t w_size = (flash_size > firmware_buf.length()) ? firmware_buf.length() : flash_size;

    QString bytes_speed_str = convert_bytes_speed_unit((float)(w_size) / take_sec);

    if (ok)
    {
        log_info(QString("Write done, take time: %1 s %2").arg(take_sec, 0, 'f', 2).arg(bytes_speed_str));
    }
    else
    {
        log_error("Write fail");
    }

    delete program_worker;
}

void MainWindow::cb_verify_finish(ProgramWorker::ChipOp op, bool ok)
{
    float take_sec = (float)(take_timer.elapsed()) / 1000;
    ui->progressBar->setHidden(true);

    if (ok)
    {
        log_info(QString("Verify done, take time: %1 second").arg(take_sec, 0, 'f', 2));
    }
    else
    {
        log_error("Verify fail");
    }

    delete program_worker;
}

void MainWindow::cb_read_chip_process(uint32_t val, uint32_t max)
{
    ui->progressBar->setValue(val);
    // qDebug("%d/%d", val, max);
}

void MainWindow::cb_write_chip_process(uint32_t val, uint32_t max)
{
    ui->progressBar->setValue(val);
    ui->progressBar->setMaximum(max);
    // qDebug("%d/%d", val, max);
}

void MainWindow::cb_verify_chip_process(uint32_t val, uint32_t max)
{
    ui->progressBar->setValue(val);
    // qDebug("%d/%d", val, max);
}

void MainWindow::cb_usb_device_changed()
{
    qDebug("[main] cb_usb_device_changed");

    // cb_tick_enum_device();
    device_change_delay_enum_timer->start();
}

int MainWindow::device_connect()
{
    int32_t err;
    Devices *tmp_dev;

    tmp_dev = current_device();
    if (tmp_dev == NULL)
    {
        return -1;
    }

    err = tmp_dev->connect();
    if (err < 0)
    {
        qDebug("[main] connect fail");
        log_info("connect fail");
        ui->label_info_idcode->setText("N/A");
        return -1;
    }

    qDebug("[main] connect ok");
    log_info("connect ok");

    QString idcode_hex_str = QString("%1").arg(tmp_dev->get_idcode(), 8, 16, QChar('0')).toUpper();

    ui->label_info_idcode->setText(idcode_hex_str);
    log_info(QString("id_code: 0x%1").arg(idcode_hex_str));

    return 0;
}
