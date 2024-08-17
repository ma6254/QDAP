#include "enum_dap.h"
#include "ui_enum_dap.h"

EnumDAP::EnumDAP(QWidget *parent)
    : QWidget(parent), ui(new Ui::EnumDAP)
{
    ui->setupUi(this);

    ui->dd_dev->addItem("Any CMSIS-DAP");
    ui->dd_dev->setCurrentText(0);

    connect(ui->dd_dev, SIGNAL(currentIndexChanged(int)), this, SLOT(cb_dd_dev_currentIndexChanged(int)));
}

EnumDAP::~EnumDAP()
{
    delete ui;
}

// QComboBox *EnumDAP::dd_dev()
// {
//     return ui->dd_dev;
// }

void EnumDAP::dd_dev_clear()
{
    while (ui->dd_dev->count() > 1)
    {
        ui->dd_dev->removeItem(1);
    }

    dev_list.clear();

    ui->label_manufacturer->setText("N/A");
    ui->label_product->setText("N/A");
    ui->label_serial->setText("N/A");
    ui->label_version->setText("N/A");
}

void EnumDAP::dd_dev_append(CMSIS_DAP_Base *dap)
{
    dev_list.append(dap);

    QString tmp_str;

    switch (dap->type())
    {
    case Devices::DAP_USB_HID:
    {

        // tmp_str = QString::asprintf(
        //     "%d | HID [%s] [%s]",
        //     ui->dd_dev->count(),
        //     qPrintable(dap->get_manufacturer_string()),
        //     qPrintable(dap->get_product_string()));
        tmp_str = QString::asprintf(
            "%d | %s",
            ui->dd_dev->count(),
            qPrintable(dap->get_product_string()));
    }
    break;
    case Devices::DAP_USB_Bulk:
    {
        CMSIS_DAP_V2 *tmp_dap_v2 = (CMSIS_DAP_V2 *)dap;

        tmp_str = QString::asprintf(
            "%d | %s",
            ui->dd_dev->count(),
            qPrintable(tmp_dap_v2->get_interface_string()));
    }
    break;
    default:
        tmp_str = "Unknown " + dap->type_str();
        break;
    }

    ui->dd_dev->addItem(tmp_str);

    if (ui->dd_dev->currentIndex() == 0)
    {
        set_info(0);
    }
    else
    {
        set_info(ui->dd_dev->currentIndex() - 1);
    }
}

void EnumDAP::set_info_empty()
{
    ui->label_manufacturer->setText("N/A");
    ui->label_product->setText("N/A");
    ui->label_serial->setText("N/A");
    ui->label_version->setText("N/A");
}

void EnumDAP::set_info(CMSIS_DAP_Base *dap)
{
    if (dap == NULL)
    {
        set_info_empty();
        return;
    }

    ui->label_manufacturer->setText(dap->get_manufacturer_string());
    ui->label_product->setText(dap->get_product_string());
    ui->label_serial->setText(dap->get_serial_string());
    ui->label_version->setText(dap->get_version_string());
}

void EnumDAP::set_info(int index)
{
    if (index >= dev_list.count())
    {
        set_info_empty();
        return;
    }

    set_info(dev_list[index]);
}

int EnumDAP::count()
{
    return dev_list.count();
}

// void EnumDAP::cb_devices_changed(QList<CMSIS_DAP_Base *> dev_list)
// {
// }

void EnumDAP::cb_dd_dev_currentIndexChanged(int index)
{
    if (index == 0)
    {
        set_info(0);
    }
    else
    {
        set_info(index - 1);
    }
}
