#include "mainwindow.h"
#include "devices.h"
#include "device_list.h"

#include <QApplication>

int main(int argc, char *argv[])
{

    qRegisterMetaType<uint32_t>("uint32_t");
    qRegisterMetaType<ProgramWorker::ChipOp>("ProgramWorker::ChipOp");
    qRegisterMetaType<Devices>("Devices");
    qRegisterMetaType<Devices *>("Devices *");
    qRegisterMetaType<DeviceList>("DeviceList");

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
