#include <QTimer>
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "enum_writer_list.h"
#include "dap_hid.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
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

void MainWindow::cb_action_enum_device_list(void)
{
    // DAP_HID::enum_device();

    enum_writer_list *d = new enum_writer_list(this);

    connect(this, SIGNAL(device_changed(QList<DAP_HID *>)),
            d, SLOT(cb_device_changed(QList<DAP_HID *>)));

    force_update_device_list = true;

    d->exec();

    delete d;
}

void MainWindow::cb_tick_enum_device(void)
{
    // qDebug("[tick_enum_device] tick");

    dap_hid_device_list.clear();
    DAP_HID::enum_device(&dap_hid_device_list);

    if (dap_hid_device_list_compare(dap_hid_device_list_prev, dap_hid_device_list) || force_update_device_list)
    {
        emit device_changed(dap_hid_device_list);
    }

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
