#include <QFileDialog>
#include <QTimer>
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "enum_writer_list.h"
#include "dap_hid.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->action_file_open, SIGNAL(triggered()), this, SLOT(cb_action_open_firmware_file(void)));
    connect(ui->action_file_save, SIGNAL(triggered()), this, SLOT(cb_action_save_firmware_file(void)));
    connect(ui->action_target_connect, SIGNAL(triggered()), this, SLOT(cb_action_connect(void)));
    connect(ui->action_target_run, SIGNAL(triggered()), this, SLOT(cb_action_reset_run(void)));
    connect(ui->action_enum_device_list, SIGNAL(triggered()), this, SLOT(cb_action_enum_device_list(void)));

    timer_enum_device = new QTimer();
    connect(timer_enum_device, SIGNAL(timeout()), this, SLOT(cb_tick_enum_device()));
    timer_enum_device->setInterval(500);
    timer_enum_device->start();
}

MainWindow::~MainWindow()
{
    delete ui;
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

void MainWindow::cb_action_connect(void)
{
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

    tmp_dev->connect();
}

void MainWindow::cb_action_reset_run(void)
{
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

    tmp_dev->run();
}
