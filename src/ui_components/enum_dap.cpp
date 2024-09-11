#include "enum_dap.h"
#include "ui_enum_dap.h"

EnumDAP::EnumDAP(QWidget *parent)
    : QWidget(parent), ui(new Ui::EnumDAP)
{
    ui->setupUi(this);

    ui->dd_dev->addItem("Any CMSIS-DAP");
    ui->dd_dev->setCurrentText(0);

    connect(ui->dd_dev, SIGNAL(currentIndexChanged(int)), this, SLOT(cb_dd_dev_currentIndexChanged(int)));

    QRegExp regexp("\\d+");
    QRegExpValidator *validator = new QRegExpValidator(regexp, this);
    ui->lineEdit_clock->setValidator(validator);
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

    device_list.clear();

    ui->label_manufacturer->setText("N/A");
    ui->label_product->setText("N/A");
    ui->label_serial->setText("N/A");
    ui->label_version->setText("N/A");
}

void EnumDAP::dd_dev_append(CMSIS_DAP_Base *dap)
{
    device_list.append(dap);

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
            "%d | V1 | %s",
            ui->dd_dev->count(),
            qPrintable(dap->get_product_string()));
    }
    break;
    case Devices::DAP_USB_Bulk:
    {
        CMSIS_DAP_V2 *tmp_dap_v2 = (CMSIS_DAP_V2 *)dap;

        tmp_str = QString::asprintf(
            "%d | V2 | %s",
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

void EnumDAP::set_config_port(CMSIS_DAP_Base::Port port, bool swj)
{
    switch (port)
    {
    case CMSIS_DAP_Base::SWD:
        ui->comboBox_port->setCurrentIndex(0);
        break;
    case CMSIS_DAP_Base::JTAG:
        ui->comboBox_port->setCurrentIndex(1);
        break;
    default:
        ui->comboBox_port->setCurrentIndex(-1);
        break;
    }

    ui->checkBox_swj->setChecked(swj);
}

void EnumDAP::set_config_clock(uint64_t clock, Devices::ClockUnit unit)
{
    ui->lineEdit_clock->setText(QString::number(clock));
    ui->comboBox_clock_unit->setCurrentIndex(unit);
}

CMSIS_DAP_Base::Port EnumDAP::get_config_port()
{
    switch (ui->comboBox_port->currentIndex())
    {
    case 0:
        return CMSIS_DAP_Base::SWD;
    default:
        return CMSIS_DAP_Base::JTAG;
    }
}

bool EnumDAP::get_config_swj()
{
    return ui->checkBox_swj->isChecked();
}

QString EnumDAP::get_config_clock_str()
{
    return ui->lineEdit_clock->text() +
           Devices::get_clock_unit_str(static_cast<Devices::ClockUnit>(ui->comboBox_clock_unit->currentIndex()));
}

uint64_t EnumDAP::get_config_clock()
{
    return ui->lineEdit_clock->text().toULongLong();
}

Devices::ClockUnit EnumDAP::get_config_clock_unit()
{
    return static_cast<Devices::ClockUnit>(ui->comboBox_clock_unit->currentIndex());
}

void EnumDAP::set_info(CMSIS_DAP_Base *dap)
{
    if (dap == NULL)
    {
        set_info_empty();
        return;
    }

    ui->label_manufacturer->setText(dap->get_manufacturer_string());

    switch (dap->type())
    {
    case Devices::DAP_USB_HID:
    {
        ui->label_product->setText(dap->get_product_string());
    }
    break;
    case Devices::DAP_USB_Bulk:
    {
        CMSIS_DAP_V2 *tmp_dap_v2 = (CMSIS_DAP_V2 *)dap;

        ui->label_product->setText(tmp_dap_v2->get_interface_string());
    }
    break;
    }

    ui->label_serial->setText(dap->get_serial_string());
    ui->label_version->setText(dap->get_version_string());
}

void EnumDAP::set_info(int index)
{
    if (index >= device_list.count())
    {
        set_info_empty();
        return;
    }

    set_info(device_list[index]);
}

int EnumDAP::count()
{
    return device_list.count();
}

int EnumDAP::current_index()
{
    return ui->dd_dev->currentIndex();
}

void EnumDAP::set_current_index(int index)
{
    ui->dd_dev->setCurrentIndex(index);
}

Devices *EnumDAP::current_device()
{
    if (count() == 0)
        return NULL;

    if (current_index() < 0)
        return NULL;

    if (current_index() == 0)
        return device_list[0];

    return device_list[current_index() - 1];
}

int EnumDAP::set_current_device(Devices device)
{
    return -1;
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
