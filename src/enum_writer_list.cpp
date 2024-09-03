#include "enum_writer_list.h"
#include "ui_enum_writer_list.h"

enum_writer_list::enum_writer_list(QWidget *parent) : QDialog(parent),
                                                      ui(new Ui::enum_writer_list)
{
    ui->setupUi(this);

    connect(ui->collapse, SIGNAL(currentChanged(int)), this, SLOT(cb_collapse_currentChanged(int)));
    connect(ui->btn_ok, SIGNAL(clicked()), this, SLOT(cb_btn_ok()));

    tmp_current_device = NULL;

    icon_arrow_right = QIcon("assert/chevron_right_24dp_FFFFFF_FILL0_wght400_GRAD0_opsz24.png");
    icon_arrow_down = QIcon("assert/keyboard_arrow_down_24dp_FFFFFF_FILL0_wght400_GRAD0_opsz24.png");

    // widget_dap = new QWidget(this);
    // vbox_dap = new QVBoxLayout(widget_dap);
    // widget_dap->setLayout(vbox_dap);
    // widget_dap->setContentsMargins(0, 0, 0, 0);
    // vbox_dap->setMargin(0);

    enum_dap = new EnumDAP(this);

    // dd_dap = new QComboBox(this);
    // vbox_dap->addWidget(dd_dap);
    // dd_dap->addItem("Any CMSIS-DAP");
    // vbox_dap->addWidget(new QLabel(""));
    // vbox_dap->setStretch(0, 0);
    // vbox_dap->setStretch(1, 1);

    while (ui->collapse->count())
        ui->collapse->removeItem(0);

    ui->collapse->addItem(enum_dap, "CMSIS-DAP");
    ui->collapse->addItem(new QWidget(), "Segger J-Link");
    ui->collapse->addItem(new QWidget(), "WCH CH347");
    ui->collapse->addItem(new QWidget(), "FTDI FT2232");

    for (uint32_t i = 0; i < ui->collapse->count(); i++)
    {
        collapse_title_bak_list.append(ui->collapse->itemText(i));
    }

    ui->collapse->setCurrentIndex(0);
    set_collapse_icon(0);

    ui->collapse->setStyleSheet(R""(
QToolBox::tab {
    background-color: rgba(50, 50, 50, 1);
    color: rgba(255, 255, 255, 1);
    border-radius: 5px;
}
)"");

    connect(ui->btn_refresh_enum_devices, SIGNAL(clicked()), this, SLOT(cb_refresh_enum_devices()));
}

enum_writer_list::~enum_writer_list()
{
    delete ui;
}

void enum_writer_list::set_auto_refresh(bool auto_refresh)
{
    is_auto_refresh_enum_devices = auto_refresh;
    ui->btn_refresh_enum_devices->setEnabled(!auto_refresh);
}

void enum_writer_list::cb_device_changed(DeviceList dev_list, bool changed)
{
    if (is_auto_refresh_enum_devices == false)
    {
        ui->btn_refresh_enum_devices->setEnabled(true);
    }

    if (changed == false)
    {
        return;
    }

    device_list.clear();
    device_list.append(dev_list);

    qDebug("[enum_writer_list] dev_changed count:%d", dev_list.count());

    // dap_hid_list_clear();
    enum_dap->dd_dev_clear();

    for (int i = 0; i < dev_list.count(); i++)
    {
        Devices *tmp_dev = dev_list.at(i);

        switch (tmp_dev->type())
        {
        case Devices::DAP_USB_HID:
        {
            // qDebug("    %d [%s] [%s] [%s]",
            //        i,
            //        qPrintable(tmp_dev->type_str()),
            //        qPrintable(tmp_dev->get_manufacturer_string()),
            //        qPrintable(tmp_dev->get_product_string()));

            enum_dap->dd_dev_append((CMSIS_DAP_Base *)tmp_dev);
        }
        break;
        case Devices::DAP_USB_Bulk:
        {
            // qDebug("    %d [%s] [%s] [%s]",
            //        i,
            //        qPrintable(tmp_dev->type_str()),
            //        qPrintable(tmp_dev->get_manufacturer_string()),
            //        qPrintable(tmp_dev->get_product_string()));

            enum_dap->dd_dev_append((CMSIS_DAP_Base *)tmp_dev);
        }
        break;
        case Devices::DAP_USB_JLink:
        {
        }
        break;
        case Devices::DAP_ETH_JLink:
        {
        }
        break;
        case Devices::DAP_CH347:
        {
        }
        break;
        case Devices::DAP_FT2232:
        {
        }
        break;
        }

        // QPushButton *tmp_btn = new QPushButton(this);
        // tmp_btn->setText(str);
        // ui->gbox_cmsis_dap->layout()->addWidget(tmp_btn);
    }

    // ui->gbox_cmsis_dap->setTitle(QString::asprintf("CMSIS-DAP [%d]", ui->comboBox_daplink->count() - 1));
    ui->collapse->setItemText(0, QString::asprintf(
                                     "%s [%d]",
                                     qPrintable(collapse_title_bak_list[0]),
                                     enum_dap->count()));
    // enum_dap->dd_dev()->setCurrentIndex(current_device_index);
}

void enum_writer_list::cb_collapse_currentChanged(int index)
{
    set_collapse_icon(index);
}

void enum_writer_list::cb_btn_ok(void)
{
    // emit finished(ui->comboBox_daplink->currentIndex());

    accept();
}

void enum_writer_list::cb_refresh_enum_devices()
{
    if (is_auto_refresh_enum_devices)
        return;

    ui->btn_refresh_enum_devices->setEnabled(false);

    emit refresh_enum_devides();
}

void enum_writer_list::dap_hid_list_clear(void)
{
    // while (ui->comboBox_daplink->count() > 1)
    // {
    //     ui->comboBox_daplink->removeItem(1);
    // }

    // while (enum_dap->dd_dev()->count() > 1)
    // {
    //     enum_dap->dd_dev()->removeItem(1);
    // }
}

int enum_writer_list::currentIndex(void)
{
    // return ui->comboBox_daplink->currentIndex();

    if (device_list.count() == 0)
        return -1;

    switch (ui->collapse->currentIndex())
    {
    case 0:
    {
        return enum_dap->current_index();
    }
    break;
    }

    return -1;
}

void enum_writer_list::setCurrentIndex(int n)
{
    // current_device_index = n;
    // ui->comboBox_daplink->setCurrentIndex(n);
}

Devices *enum_writer_list::current_device()
{
    // return tmp_current_device;

    if (device_list.count() == 0)
        return NULL;

    switch (ui->collapse->currentIndex())
    {
    case 0:
    {
        return enum_dap->current_device();
    }
    break;
    }

    return NULL;
}

void enum_writer_list::set_collapse_icon(uint32_t index)
{
    ui->collapse->setItemIcon(0, icon_arrow_right);
    ui->collapse->setItemIcon(1, icon_arrow_right);
    ui->collapse->setItemIcon(2, icon_arrow_right);
    ui->collapse->setItemIcon(3, icon_arrow_right);

    ui->collapse->setItemIcon(index, icon_arrow_down);
}

void enum_writer_list::set_btn_manual_refresh_enabled(bool enabled)
{
    ui->btn_refresh_enum_devices->setEnabled(enabled);
}
