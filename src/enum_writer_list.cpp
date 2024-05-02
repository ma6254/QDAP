#include "enum_writer_list.h"
#include "ui_enum_writer_list.h"

enum_writer_list::enum_writer_list(QWidget *parent) : QDialog(parent),
                                                      ui(new Ui::enum_writer_list)
{
    ui->setupUi(this);

    connect(ui->btn_ok, SIGNAL(clicked()), this, SLOT(cb_btn_ok()));
}

enum_writer_list::~enum_writer_list()
{
    delete ui;
}

void enum_writer_list::cb_device_changed(QList<DAP_HID *> dev_list)
{
    QLayout *gbox_layout = ui->gbox_cmsis_dap->layout();
    QComboBox *combobox = (QComboBox *)gbox_layout->itemAt(0)->widget();

    qDebug("[enum_writer_list] hid dev_changed count:%d", dev_list.count());

    dap_hid_list_clear();

    for (int i = 0; i < dev_list.count(); i++)
    {
        DAP_HID *tmp_dev = dev_list.at(i);

        QString str = QString("%1 %2")
                          .arg(qPrintable(tmp_dev->dap_hid_get_manufacturer_string()))
                          .arg(qPrintable(tmp_dev->dap_hid_get_product_string()));

        combobox->addItem(str);

        // QPushButton *tmp_btn = new QPushButton(this);
        // tmp_btn->setText(str);
        // ui->gbox_cmsis_dap->layout()->addWidget(tmp_btn);
    }

    ui->comboBox_daplink->setCurrentIndex(current_device);
}

void enum_writer_list::cb_btn_ok(void)
{
    emit finished(ui->comboBox_daplink->currentIndex());

    accept();
}

void enum_writer_list::dap_hid_list_clear(void)
{
    QLayout *gbox_layout = ui->gbox_cmsis_dap->layout();
    QComboBox *combobox = (QComboBox *)gbox_layout->itemAt(0)->widget();

    for (int i = 0; gbox_layout->count() > 1; i++)
    {
        QLayoutItem *tmp_item = gbox_layout->itemAt(1);
        gbox_layout->removeItem(tmp_item);

        delete tmp_item->widget();
    }

    while (combobox->count() > 1)
    {
        combobox->removeItem(1);
    }
}

int enum_writer_list::currentIndex(void)
{
    return ui->comboBox_daplink->currentIndex();
}

void enum_writer_list::setCurrentIndex(int n)
{
    current_device = n;
    ui->comboBox_daplink->setCurrentIndex(n);
}
