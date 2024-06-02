#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    qRegisterMetaType<uint32_t>("uint32_t");
    qRegisterMetaType<ProgramWorker::ChipOp>("ProgramWorker::ChipOp");

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
