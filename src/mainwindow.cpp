#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "dap_hid.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    DAP_HID::enum_device();
}

MainWindow::~MainWindow()
{
    delete ui;
}
