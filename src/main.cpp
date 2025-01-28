#include "mainwindow.h"
#include "devices.h"
#include "device_list.h"

#include <QApplication>
#include <libusb.h>

libusb_context *g_libusb_context = NULL;

int main(int argc, char *argv[])
{
    int rc = 0;

    rc = libusb_init(&g_libusb_context);
    if (rc != 0)
    {
        qDebug("[amin] libusb_init fail %d", rc);
        libusb_exit(g_libusb_context);
        return -1;
    }

    qRegisterMetaType<uint32_t>("uint32_t");
    qRegisterMetaType<ProgramWorker::ChipOp>("ProgramWorker::ChipOp");
    // qRegisterMetaType<Devices>("Devices");
    qRegisterMetaType<Devices *>("Devices *");
    qRegisterMetaType<DeviceList>("DeviceList");

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    a.exec();

    libusb_exit(g_libusb_context);
    return 0;
}
