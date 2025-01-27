#include "dap_usb_hid.h"
#include "debug_cm.h"
#include "utils.h"
#include <QtGlobal>

uint32_t Flash_Page_Size = 4096;

DAP_HID::DAP_HID(QString usb_path)
{
    this->usb_path = usb_path;
    int32_t err;
    err_code = 0;
    // uint32_t idcode;

    err = open_device();
    if (err < 0)
    {
        qDebug("[DAP_HID] open fail");
        err_code = -1;
        close_device();
        return;
    }

    // qDebug("[DAP_HID] init");

    // dap_hid_get_info();
    // close_device();

    err = dap_hid_get_info();
    if (err < 0)
    {
        qDebug("[DAP_HID] dap_hid_get_info fail");
        err_code = -2;
        close_device();
        return;
    }

    // qDebug("    vendor_name: %s", qPrintable(dap_get_info_vendor_name()));
    // qDebug("    product_name: %s", qPrintable(dap_get_info_product_name()));
    // qDebug("    serial_number: %s", qPrintable(dap_get_info_serial_number()));
    // qDebug("    protocol_version: %s", qPrintable(dap_get_info_cmsis_dap_protocol_version()));

    // qDebug("[enum_device] protocol_version %s", qPrintable(tmp_str));

    hid_version = dap_get_info_cmsis_dap_protocol_version();
    if (hid_version.isEmpty())
    {
        // qDebug("[enum_device] dap_hid_get_info fail");
        err_code = -3;
        close_device();
        return;
    }

    close_device();
    // if (hid_serial.isEmpty())
    // {
    //     hid_serial = dap_get_info_serial_number();
    // }

    // tmp_str = dap_get_info_product_name();
    // qDebug("[enum_device] product_name %s", qPrintable(tmp_str));

    // if (tmp_str.isEmpty())
    // {
    //     // qDebug("[enum_device] dap_hid_get_info fail");
    //     err_code = -1;
    //     return;
    // }

    // err = dap_get_info_caps();
    // if (err < 0)
    // {
    //     qDebug("    caps: Unspupport");
    // }
    // else
    // {
    //     int16_t caps = err;
    //     qDebug("    caps: 0x%X %s", caps, qPrintable(parse_dap_hid_info_resp(caps)));
    // }

    // qDebug("    freq: %d", dap_get_info_freq());

    // err = dap_connect(1);
    // if (err < 0)
    // {
    //     qDebug("[enum_device] dap_connect fail");
    //     return;
    // }

    // err = dap_reset_target();
    // if (err < 0)
    // {
    //     qDebug("[enum_device] dap_reset_target fail");
    //     return;
    // }

    // err = dap_swd_config(0x02);
    // if (err < 0)
    // {
    //     qDebug("[enum_device] dap_swd_config fail");
    //     return;
    // }

    // err = dap_transfer_config(10, 10, 0);
    // if (err < 0)
    // {
    //     qDebug("[enum_device] dap_swd_config fail");
    //     return;
    // }

    // err = dap_swd_reset();
    // if (err < 0)
    // {
    //     qDebug("[enum_device] dap_swd_reset fail");
    //     return;
    // }

    // err = dap_swd_switch(0xE79E);
    // if (err < 0)
    // {
    //     qDebug("[enum_device] dap_swd_reset fail");
    //     return;
    // }

    // err = dap_swd_reset();
    // if (err < 0)
    // {
    //     qDebug("[enum_device] dap_swd_reset fail");
    //     return;
    // }

    // err = dap_swd_read_idcode(&idcode);
    // if (err < 0)
    // {
    //     qDebug("[enum_device] dap_swd_read_idcode fail");
    // }
    // else
    // {
    //     qDebug("[enum_device] dap_swd_read_idcode 0x%08X", idcode);
    // }

    // err = swd_init_debug();
    // if (err < 0)
    // {
    //     qDebug("[enum_device] swd_init_debug fail");
    //     return;
    // }

    // err = dap_set_target_state_hw(DAP_TARGET_RESET_PROGRAM);
    // if (err < 0)
    // {
    //     qDebug("[enum_device] dap_set_target_state_hw RESET_PROGRAM fail");
    //     return;
    // }

    // dap_disconnect();
    // close_device();
}

DAP_HID::DAP_HID(Devices *devices)
{
    if (devices->type() != type())
    {
        return;
    }
}

DAP_HID::~DAP_HID()
{

    if (dev)
    {
        qDebug("[DAP_HID] destroy");
        dap_disconnect();
        close_device();
    }
}

int32_t DAP_HID::connect()
{
    int32_t err;

    err = open_device();
    if (err < 0)
    {
        qDebug("[DAP_HID] connect open fail");
        return err;
    }

    err = dap_connect(1);
    if (err < 0)
    {
        qDebug("[DAP_HID] connect dap_connect fail");
        return err;
    }

    err = dap_reset_target();
    if (err < 0)
    {
        qDebug("[DAP_HID] connect dap_reset_target fail");
        return err;
    }

    // err = dap_swd_config(0x02);
    // if (err < 0)
    // {
    //     qDebug("[enum_device] dap_swd_config fail");
    //     return err;
    // }

    // err = dap_transfer_config(10, 10, 0);
    // if (err < 0)
    // {
    //     qDebug("[enum_device] dap_swd_config fail");
    //     return err;
    // }

    err = dap_swd_reset();
    if (err < 0)
    {
        qDebug("[DAP_HID] connect dap_swd_reset fail");
        return err;
    }

    // JTAG-to-SWD
    err = dap_swd_switch(0xE79E);
    if (err < 0)
    {
        qDebug("[DAP_HID] connect dap_swd_switch fail");
        return err;
    }

    err = dap_swd_reset();
    if (err < 0)
    {
        qDebug("[DAP_HID] connect dap_swd_reset fail");
        return err;
    }

    err = dap_swd_read_idcode(&tmp_idcode);
    if (err < 0)
    {
        qDebug("[DAP_HID] connect dap_swd_read_idcode fail idcode:0x%08X", tmp_idcode);
    }
    else
    {
        qDebug("[DAP_HID] connect dap_swd_read_idcode 0x%08X", tmp_idcode);
    }

    err = swd_init_debug();
    if (err < 0)
    {
        qDebug("[DAP_HID] connect swd_init_debug fail");
        return err;
    }

    qDebug("[DAP_HID] connect swd_init_debug ok");

    err = dap_set_target_state_hw(DAP_TARGET_RESET_PROGRAM);
    if (err < 0)
    {
        qDebug("[DAP_HID] connect dap_set_target_state_hw RESET_PROGRAM fail");
        return err;
    }
    qDebug("[DAP_HID] connect dap_set_target_state_hw RESET_PROGRAM ok");

    // dap_disconnect();
    // close_device();
    return 0;
}

int32_t DAP_HID::run()
{

    int32_t err;
    uint32_t idcode;

    err = open_device();
    if (err < 0)
    {
        qDebug("[DAP_HID] run open fail");
        return err;
    }

    err = dap_connect(1);
    if (err < 0)
    {
        qDebug("[DAP_HID] run dap_connect fail");
        return err;
    }

    // err = dap_set_target_state_hw(DAP_TARGET_RESET_RUN);
    err = dap_set_target_state_hw(DAP_TARGET_RESET_RUN);
    if (err < 0)
    {
        qDebug("[DAP_HID] run dap_set_target_state_hw DAP_TARGET_RESET_RUN fail");
        return err;
    }

    qDebug("[DAP_HID] run");

    dap_disconnect();
    close_device();
    return 0;
}

/*******************************************************************************
 * @brief 枚举扫描设备
 * @param None
 * @return None
 ******************************************************************************/
int32_t DAP_HID::enum_device_id(DeviceList *dev_list, uint16_t vid, uint16_t pid)
{
    uint32_t i = 0;

    if (dev_list == NULL)
        return -1;

    dev_list->clear();

    hid_device_info *device_info = hid_enumerate(vid, pid);
    if (device_info == NULL)
    {
        // qDebug("[enum_device] is empty");

        hid_free_enumeration(device_info);
        return 0;
    }

    // for (i = 0; i < dev_list->count(); i++)
    // {
    //     DAP_HID *tmp_dev = dev_list->first();
    //     dev_list->pop_front();
    //     delete tmp_dev;
    // }

    i = 0;
    while (1)
    {

        DAP_HID *tmp_dev = new DAP_HID(device_info->path);

        if (tmp_dev->error() < 0)
        {
            // qDebug("[DAP_HID] enum_devices index:%d init failed err:%d", i, tmp_dev->error());

            delete tmp_dev;

            if (device_info->next == NULL)
                break;

            device_info = device_info->next;
            continue;
        }

        // qDebug("[enum_device] #%d. ser:%d %ls %ls", i,
        //        device_info->interface_number,
        //        device_info->manufacturer_string,
        //        device_info->product_string);

        dev_list->push_back(tmp_dev);

        i++;
        if (device_info->next == NULL)
            break;

        device_info = device_info->next;
    }

    // qDebug("[enum_device] count %d", dev_list->count());

    hid_free_enumeration(device_info);
    return i;
}

int32_t DAP_HID::enum_device(DeviceList *dev_list)
{
    DeviceList tmp_total_list;
    DeviceList tmp_list;
    int err;

    if (dev_list == NULL)
        return -1;
    dev_list->clear();

    tmp_list.clear();
    err = enum_device_id(&tmp_list);
    if (err > 0)
        dev_list->append(tmp_list);

    tmp_list.clear();
    err = enum_device_id(&tmp_list, 0xC251, 0xF001);
    if (err > 0)
        dev_list->append(tmp_list);

    tmp_list.clear();
    err = enum_device_id(&tmp_list, 0xC251, 0xF002);
    if (err > 0)
        dev_list->append(tmp_list);

    // H7 Tool
    tmp_list.clear();
    err = enum_device_id(&tmp_list, 0xC251, 0xF00A);
    if (err > 0)
        dev_list->append(tmp_list);

    // enum_device_id(&tmp_list, 0xC251, 0x2722);
    // dev_list->append(tmp_list);

    // enum_device_id(&tmp_list, 0xC251, 0x2750);
    // dev_list->append(tmp_list);

    return dev_list->count();
}

bool DAP_HID::equal(const Devices &device)
{
    DAP_HID *dap = (DAP_HID *)&device;
    bool result = usb_path == dap->usb_path;

    // qDebug("[DAP_HID] equal resilt:%d this->[%s] another->[%s]", result, qPrintable(hid_product), qPrintable(dap->hid_product));

    // if (CMSIS_DAP_Base::equal(device) == false)
    //     return false;

    return result;
}

int32_t DAP_HID::open_device()
{
    // qDebug("[DAP_HID] open device %s", qPrintable(usb_path));

    hid_device *hid_dev = hid_open_path(usb_path.toUtf8().constData());
    if (hid_dev == NULL)
    {
        QString err_info = QString::fromWCharArray(hid_error(NULL));
        qDebug("[DAP_HID] hid open fail: %s", qUtf8Printable(err_info));
        return -1;
    }
    this->dev = hid_dev;

    return 0;
}

int32_t DAP_HID::close_device()
{
    // qDebug("[DAP_HID] close device %s", qPrintable(usb_path));
    hid_close(dev);
    dev = NULL;
    return 0;
}

int32_t DAP_HID::dap_request(uint8_t *tx_data, uint8_t *rx_data)
{
    int32_t err;
    uint8_t tx_buf[65] = {0};

    tx_buf[0] = 0x00;
    memcpy(tx_buf + 1, tx_data, 64);

    if (dev == NULL)
    {
        qDebug("[DAP_HID] dev is NULL");
        return -1;
    }

    err = hid_write(dev, tx_buf, 65);
    if (err < 0)
    {
        QString err_info = QString::fromWCharArray(hid_error(dev));
        qDebug("[DAP_HID] dap_hid_request fail: %s", qUtf8Printable(err_info));
        return err;
    }

    err = hid_read_timeout(dev, rx_data, 64, 100);
    if (err < 0)
    {
        QString err_info = QString::fromWCharArray(hid_error(dev));
        qDebug("[DAP_HID] dap_hid_request fail: %s", qUtf8Printable(err_info));
        return err;
    }

    // 首字节一定与发送相等
    if (rx_data[0] != tx_data[0])
    {
        qDebug("[DAP_HID] dap_hid_request fail cmd not requls tx:0x%02X rx:0x%02X", tx_data[0], rx_data[0]);
        hexdump(rx_data, 64);
        return -1;
    }

    return 0;
}

int32_t DAP_HID::dap_hid_request(uint8_t *tx_data, uint8_t *rx_data)
{
    int32_t err;
    uint8_t tx_buf[65] = {0};

    tx_buf[0] = 0x00;
    memcpy(tx_buf + 1, tx_data, 64);

    if (dev == NULL)
    {
        qDebug("[DAP_HID] dev is NULL");
        return -1;
    }

    err = hid_write(dev, tx_buf, 65);
    if (err < 0)
    {
        QString err_info = QString::fromWCharArray(hid_error(dev));
        qDebug("[DAP_HID] dap_hid_request fail: %s", qUtf8Printable(err_info));
        return err;
    }

    err = hid_read_timeout(dev, rx_data, 64, 100);
    if (err < 0)
    {
        QString err_info = QString::fromWCharArray(hid_error(dev));
        qDebug("[DAP_HID] dap_hid_request fail: %s", qUtf8Printable(err_info));
        return err;
    }

    // 首字节一定与发送相等
    if (rx_data[0] != tx_data[0])
    {
        // qDebug("[DAP_HID] dap_hid_request fail cmd not requls tx:0x%02X rx:0x%02X", tx_data[0], rx_data[0]);
        // hexdump(rx_data, 64);
        return -1;
    }

    return 0;
}

int32_t DAP_HID::dap_hid_resp_status_return(uint8_t *rx_data)
{
    uint8_t status = rx_data[1];

    if (status == DAP_OK)
        return 1;
    else if (status == DAP_ERROR)
        return 0;
    else
        return -1;
}

QString DAP_HID::dap_hid_get_manufacturer_string()
{
    return hid_manufacturer;
}

QString DAP_HID::dap_hid_get_product_string()
{
    return hid_product;
}

int32_t DAP_HID::dap_hid_get_info()
{
    wchar_t tmp_serial_wchar_buf[1024];
    int err;

    if (dev == NULL)
    {
        return -1;
    }

    hid_device_info *info = hid_get_device_info(dev);
    if (info == NULL)
    {
        qDebug("[dap_hid] hid_get_device_info fail");
        return -1;
    }

    hid_manufacturer = QString::fromWCharArray(info->manufacturer_string);
    hid_product = QString::fromWCharArray(info->product_string);
    // hid_serial = QString::fromWCharArray(info->serial_number);

    err = hid_get_serial_number_string(dev, tmp_serial_wchar_buf, 33);
    if (err == 0)
    {
        hid_serial = QString::fromWCharArray(tmp_serial_wchar_buf, 33);
    }

    // qDebug("[dap_hid] hid_get_device_info ok");
    // qDebug("[dap_hid] manufacturer_string: %s", qPrintable(manufacturer));
    // qDebug("[dap_hid] product_string: %s", qPrintable(product));

    return 0;
}

/*******************************************************************************
 * @brief 0x01 = Get the Vendor Name (string).
 * @ref https://arm-software.github.io/CMSIS_5/DAP/html/group__DAP__Info.html
 * @param None
 * @return None
 ******************************************************************************/
QString DAP_HID::dap_get_info_vendor_name()
{
    uint8_t tx_buf[64] = {0};
    uint8_t rx_buf[64] = {0};
    int32_t err;

    // qDebug("[dap_hid] dap_get_info_vendor_name");

    tx_buf[0] = 0x00;
    tx_buf[1] = 0x01;
    err = dap_hid_request(tx_buf, rx_buf);
    if (err < 0)
    {
        return "";
    }

    uint8_t len = rx_buf[1];

    if (len == 0)
        return "";

    if (len > 62)
        len = 62;

    // qDebug("[DAP_HID] dap_get_info_vendor_name resp:");
    // hexdump(rx_buf, 64);

    return QString(QLatin1String((char *)rx_buf + 2, len));
}

/*******************************************************************************
 * @brief 0x02 = Get the Product Name (string).
 * @param None
 * @return None
 ******************************************************************************/
QString DAP_HID::dap_get_info_product_name()
{
    uint8_t tx_buf[64] = {0};
    uint8_t rx_buf[64] = {0};
    int32_t err;

    // qDebug("[dap_hid] dap_get_info_product_name");

    tx_buf[0] = 0x00;
    tx_buf[1] = 0x02;
    err = dap_hid_request(tx_buf, rx_buf);
    if (err < 0)
    {
        return "";
    }

    uint8_t len = rx_buf[1];

    if (len == 0)
        return "";

    if (len > 62)
        len = 62;

    // hexdump(rx_buf, 64);
    // qDebug("[dap_get_info_vendor_name] ok");
    return QString(QLatin1String((char *)rx_buf + 2, len));
}

/*******************************************************************************
 * @brief Get the Serial Number (string).
 * @param None
 * @return None
 ******************************************************************************/
QString DAP_HID::dap_get_info_serial_number()
{
    uint8_t tx_buf[64] = {0};
    uint8_t rx_buf[64] = {0};
    int32_t err;

    // qDebug("[dap_hid] dap_get_info_serial_number");

    tx_buf[0] = 0x00;
    tx_buf[1] = 0x03;
    err = dap_hid_request(tx_buf, rx_buf);
    if (err < 0)
    {
        return "";
    }

    uint8_t len = rx_buf[1];

    if (len == 0)
        return "";

    if (len > 62)
        len = 62;

    // hexdump(rx_buf, 64);
    // qDebug("[dap_get_info_serial_number] ok");
    return QString(QLatin1String((char *)rx_buf + 2, len));
}

/*******************************************************************************
 * @brief Get the CMSIS-DAP Protocol Version (string).
 * @param None
 * @return None
 ******************************************************************************/
QString DAP_HID::dap_get_info_cmsis_dap_protocol_version()
{
    uint8_t tx_buf[64] = {0};
    uint8_t rx_buf[64] = {0};
    int32_t err;

    // qDebug("[dap_hid] dap_get_info_cmsis_dap_protocol_version");

    tx_buf[0] = 0x00;
    tx_buf[1] = 0x04;
    err = dap_hid_request(tx_buf, rx_buf);
    if (err < 0)
        return "";

    uint8_t len = rx_buf[1];

    if (len == 0)
        return "";

    if (len > 62)
        len = 62;

    // hexdump(rx_buf, 64);
    // qDebug("[dap_get_info_serial_number] ok");
    return QString(QLatin1String((char *)rx_buf + 2, len));
}

/*******************************************************************************
 * @brief 0xF0 = Get information about the Capabilities (BYTE) of the Debug Unit (see below for details).
 * @param None
 * @return None
 ******************************************************************************/
int32_t DAP_HID::dap_get_info_caps()
{
    uint8_t tx_buf[64] = {0};
    uint8_t rx_buf[64] = {0};
    int32_t err;

    qDebug("[dap_hid] dap_get_info_caps");

    tx_buf[0] = 0x00;
    tx_buf[1] = 0xF0;
    err = dap_hid_request(tx_buf, rx_buf);
    if (err < 0)
        return err;

    // qDebug("[DAP_HID] dap_get_info_caps resp:");
    // hexdump(rx_buf, 64);

    uint8_t len = rx_buf[1];

    if (len == 1)
        return rx_buf[2];
    else if (len == 2)
        return ((uint16_t)rx_buf[2]) << 8 | ((uint16_t)rx_buf[3]);
    else
        return -1;
}

/*******************************************************************************
 * @brief Get the Test Domain Timer parameter information (see below for details).
 * @param None
 * @return None
 ******************************************************************************/
int32_t DAP_HID::dap_get_info_freq()
{
    uint8_t tx_buf[64] = {0};
    uint8_t rx_buf[64] = {0};
    int32_t err;

    // qDebug("[dap_hid] dap_get_info_freq");

    tx_buf[0] = 0x00;
    tx_buf[1] = 0xF1;
    err = dap_hid_request(tx_buf, rx_buf);
    if (err < 0)
        return err;

    if (rx_buf[1] != 0x08)
        return -1;

    uint16_t freq = ((uint16_t)rx_buf[2]) << 8 | ((uint16_t)rx_buf[3]);

    // hexdump(rx_buf, 64);
    // qDebug("[get_info_serial_number] ok");
    return freq;
}

/*******************************************************************************
 * @brief Connect to Device and selected DAP mode.
 * @ref https://arm-software.github.io/CMSIS_5/DAP/html/group__DAP__Connect.html
 * @param None
 * @return None
 ******************************************************************************/
int32_t DAP_HID::dap_connect(uint8_t port)
{
    uint8_t tx_buf[64] = {0};
    uint8_t rx_buf[64] = {0};
    uint8_t rx_port;
    int32_t err;

    // qDebug("[dap_hid] dap_connect");

    tx_buf[0] = DAP_CMD_CONNECT;
    tx_buf[1] = port;
    err = dap_hid_request(tx_buf, rx_buf);
    if (err < 0)
    {
        return err;
    }

    rx_port = rx_buf[1];

    if (rx_port == 0)
        return -1;

    if (rx_port != port)
        return -1;

    return 0;
}

/*******************************************************************************
 * @brief Get the CMSIS-DAP Protocol Version (string).
 * @ref https://arm-software.github.io/CMSIS_5/DAP/html/group__DAP__Disconnect.html
 * @param None
 * @return None
 ******************************************************************************/
int32_t DAP_HID::dap_disconnect()
{
    uint8_t tx_buf[64] = {0};
    uint8_t rx_buf[64] = {0};
    int32_t err;

    // qDebug("[dap_hid] dap_disconnect");

    tx_buf[0] = DAP_CMD_DISCONNECT;
    err = dap_hid_request(tx_buf, rx_buf);
    if (err < 0)
        return err;

    return dap_hid_resp_status_return(rx_buf);
}

/*******************************************************************************
 * @brief The DAP_ResetTarget Command requests a target reset with a device
 *        specific command sequence. This command calls the user configurable
 *        function RESET_TARGET.
 * @ref https://arm-software.github.io/CMSIS_5/DAP/html/group__DAP__ResetTarget.html
 * @param None
 * @return None
 ******************************************************************************/
int32_t DAP_HID::dap_reset_target()
{
    uint8_t tx_buf[64] = {0};
    uint8_t rx_buf[64] = {0};
    int32_t err;

    // qDebug("[dap_hid] dap_reset_target");

    // 报告编号
    tx_buf[0] = DAP_CMD_RESET_TARGET;
    err = dap_hid_request(tx_buf, rx_buf);
    if (err < 0)
        return err;

    // qDebug("[DAP_HID] dap_reset_target resp:");
    // hexdump(rx_buf, 64);

    uint8_t status = rx_buf[1];
    // uint8_t execute = rx_buf[2];

    if (status == DAP_OK)
        return 1;
    else if (status == DAP_ERROR)
        return 0;
    else
        return -1;
}

/*******************************************************************************
 * @brief The DAP_SWJ_Sequence Command can be used to generate required SWJ
 *        sequences for SWD/JTAG Reset, SWD<->JTAG switch and Dormant operation.
 * @ref https://arm-software.github.io/CMSIS_5/DAP/html/group__DAP__SWJ__Sequence.html
 * @param None
 * @return None
 ******************************************************************************/
int32_t DAP_HID::dap_swj_sequence(uint8_t bit_count, uint8_t *data)
{
    uint8_t tx_buf[64] = {0};
    uint8_t rx_buf[64] = {0};
    int32_t err;
    uint8_t count_byte;

    // qDebug("[dap_hid] dap_swj_sequence");

    tx_buf[0] = 0x12;
    tx_buf[1] = bit_count; // Sequence Count
    // tx_buf[3] = tx_data; // SWDIO Data

    count_byte = bit_count / 8;
    if (bit_count % 8)
        count_byte += 1;
    memcpy(tx_buf + 2, data, count_byte);

    err = dap_hid_request(tx_buf, rx_buf);
    if (err < 0)
        return err;

    uint8_t status = rx_buf[1];

    err = (status == DAP_OK) ? 0 : -1;

    if (err < 0)
    {
        qDebug("[DAP_HID] dap_swj_sequence_write resp:");
        hexdump(rx_buf, 64);
    }

    return err;
}

int32_t DAP_HID::dap_swd_config(uint8_t cfg)
{
    uint8_t tx_buf[65] = {0};
    uint8_t rx_buf[64] = {0};
    int32_t err;

    // qDebug("[dap_hid] dap_swd_config");

    tx_buf[0] = 0x00; // 报告编号
    tx_buf[1] = 0x13;
    tx_buf[2] = cfg; // Configuration
    err = hid_write(dev, tx_buf, 3);
    if (err < 0)
    {
        QString err_info = QString::fromWCharArray(hid_error(dev));
        qDebug("[DAP_HID] dap_swd_sequence fail: %s", qPrintable(err_info));
        return -1;
    }

    err = hid_read_timeout(dev, rx_buf, 64, 100);
    if (err < 0)
    {
        QString err_info = QString::fromWCharArray(hid_error(dev));
        qDebug("[DAP_HID] dap_swd_sequence fail: %s", qPrintable(err_info));
        return -1;
    }

    // 首字节一定与发送相等
    if (rx_buf[0] != 0x13)
        return -1;

    uint8_t status = rx_buf[1];

    if (status != DAP_OK)
        return -1;

    return 0;
}

int32_t DAP_HID::dap_swd_sequence_write(uint8_t count, uint8_t *tx_data)
{
    uint8_t tx_buf[65] = {0};
    uint8_t rx_buf[64] = {0};
    int32_t err;
    uint8_t count_byte;

    // qDebug("[dap_hid] dap_swd_sequence_write");

    tx_buf[0] = 0x00; // 报告编号
    tx_buf[1] = 0x1D;
    tx_buf[2] = count; // Sequence Count
    tx_buf[3] = 0;     // Sequence Info
    // tx_buf[4] = tx_data; // SWDIO Data

    count_byte = count / 8;
    if (count % 8)
        count += 1;

    memcpy(tx_buf + 4, tx_data, count_byte);

    err = hid_write(dev, tx_buf, count_byte + 4);
    if (err < 0)
    {
        QString err_info = QString::fromWCharArray(hid_error(dev));
        qDebug("[DAP_HID] dap_swd_sequence fail: %s", qPrintable(err_info));
        return -1;
    }

    err = hid_read_timeout(dev, rx_buf, 64, 100);
    if (err < 0)
    {
        QString err_info = QString::fromWCharArray(hid_error(dev));
        qDebug("[DAP_HID] dap_swd_sequence fail: %s", qPrintable(err_info));
        return -1;
    }

    qDebug("[DAP_HID] dap_swd_sequence_write resp:");
    hexdump(rx_buf, 64);

    // 首字节一定与发送相等
    if (rx_buf[0] != 0x1D)
        return -1;

    uint8_t status = rx_buf[1];
    // uint8_t rx_data = rx_buf[2];

    if (status == DAP_OK)
        return 0;
    else
        return -1;
}

int32_t DAP_HID::dap_transfer_config(uint8_t idle_cyless, uint16_t wait_retry, uint16_t match_retry)
{
    uint8_t tx_buf[65] = {0};
    uint8_t rx_buf[64] = {0};
    int32_t err;

    // qDebug("[dap_hid] dap_transfer_config");

    tx_buf[0] = 0x00; // 报告编号
    tx_buf[1] = 0x04;
    tx_buf[2] = idle_cyless;
    tx_buf[3] = wait_retry >> 8;
    tx_buf[4] = wait_retry & 0xFF;
    tx_buf[5] = match_retry >> 8;
    tx_buf[6] = match_retry & 0xFF;

    err = hid_write(dev, tx_buf, 7);
    if (err < 0)
    {
        QString err_info = QString::fromWCharArray(hid_error(dev));
        qDebug("[DAP_HID] dap_transfer_config fail: %s", qPrintable(err_info));
        return -1;
    }

    err = hid_read_timeout(dev, rx_buf, 64, 100);
    if (err < 0)
    {
        QString err_info = QString::fromWCharArray(hid_error(dev));
        qDebug("[DAP_HID] dap_transfer_config fail: %s", qPrintable(err_info));
        return -1;
    }

    // 首字节一定与发送相等
    if (rx_buf[0] != 0x04)
        return -1;

    uint8_t status = rx_buf[1];

    if (status == DAP_OK)
        return 0;
    else
        return -1;
}

/*******************************************************************************
 * @brief Read/write single and multiple registers.
 * @ref https://arm-software.github.io/CMSIS_5/DAP/html/group__DAP__Transfer.html
 * @param None
 * @return None
 ******************************************************************************/
int32_t DAP_HID::dap_swd_transfer(uint8_t req, uint32_t tx_data, uint8_t *resp, uint32_t *rx_data)
{
    uint8_t tx_buf[64] = {0};
    uint8_t rx_buf[64] = {0};
    int32_t err;

    // qDebug("[dap_hid] dap_swd_transfer");

    tx_buf[0] = 0x05;                // cmd
    tx_buf[1] = 0;                   // DAP_Index
    tx_buf[2] = 1;                   // Transfer Count
    tx_buf[3] = req;                 // Transfer Request
    memcpy(tx_buf + 4, &tx_data, 4); // Transfer Data

    err = dap_hid_request(tx_buf, rx_buf);
    if (err < 0)
        return err;

    // qDebug("[DAP_HID] dap_swd_transfer resp:");
    // hexdump(rx_buf, 64);

    *(resp) = rx_buf[2];

    if (rx_data)
        memcpy(rx_data, rx_buf + 3, 4);

    return 0;
}

int32_t DAP_HID::dap_swd_transfer_retry(uint8_t req, uint32_t tx_data, uint8_t *resp, uint32_t *rx_data)
{
    int32_t err;

    for (uint8_t retry_i = 0; retry_i < 10; retry_i++)
    {
        err = dap_swd_transfer(req, tx_data, resp, rx_data);
        if (err < 0)
            return -1;

        if (rx_data && retry_i)
        {
            qDebug("[DAP_HID] dap_swd_transfer_retry i:%d resp: 0x%08X", retry_i, *rx_data);
        }

        if (*resp != DAP_TRANSFER_WAIT)
            break;
    }

    if (*resp != DAP_TRANSFER_OK)
        return -1;

    return 0;
}

int32_t DAP_HID::dap_swd_transfer_block(uint8_t req, uint32_t tx_data, uint8_t *resp, uint32_t *timestamp, uint32_t *rx_data)
{
    uint8_t tx_buf[65] = {0};
    uint8_t rx_buf[64] = {0};
    int32_t err;

    // 报告编号
    tx_buf[0] = 0x00;
    tx_buf[1] = 0x06;
    tx_buf[2] = 0;                   // DAP_Index
    tx_buf[3] = 1;                   // Transfer Count
    tx_buf[4] = req;                 // Transfer Request
    memcpy(tx_buf + 5, &tx_data, 4); // Transfer Data

    err = hid_write(dev, tx_buf, 9);
    if (err < 0)
    {
        QString err_info = QString::fromWCharArray(hid_error(dev));
        qDebug("[DAP_HID] dap_swd_transfer fail: %s", qPrintable(err_info));
        return -1;
    }

    err = hid_read_timeout(dev, rx_buf, 64, 100);
    if (err < 0)
    {
        QString err_info = QString::fromWCharArray(hid_error(dev));
        qDebug("[DAP_HID] dap_swd_transfer fail: %s", qPrintable(err_info));
        return -1;
    }

    // 首字节一定与发送相等
    if (rx_buf[0] != 0x06)
        return -1;

    qDebug("[DAP_HID] dap_swd_transfer_block resp:");
    hexdump(rx_buf, 64);

    *(resp) = rx_buf[2];
    memcpy(timestamp, rx_buf + 3, 4);
    memcpy(rx_data, rx_buf + 7, 4);

    return 0;
}

int32_t DAP_HID::dap_swd_reset()
{
    uint8_t tmp_in[8];
    uint8_t i = 0;

    for (i = 0; i < 8; i++)
    {
        tmp_in[i] = 0xff;
    }

    return dap_swj_sequence(51, tmp_in);
}

int32_t DAP_HID::dap_swd_switch(uint16_t val)
{

    uint8_t tmp_in[2];
    tmp_in[0] = val & 0xff;
    tmp_in[1] = (val >> 8) & 0xff;

    return dap_swj_sequence(16, tmp_in);
}

int32_t DAP_HID::dap_swd_read_dp(uint8_t addr, uint32_t *val)
{
    int32_t err;
    uint8_t req;
    uint8_t resp = 0;

    req = DAP_TRANS_DP | DAP_TRANS_READ_REG | DAP_TRANS_REG_ADDR(addr);

    for (uint8_t i = 0; i < 10; i++)
    {
        err = dap_swd_transfer(req, 0, &resp, val);
        if (err < 0)
        {
            return -1;
        }

        if ((resp != DAP_TRANSFER_WAIT))
            break;
    }

    if (resp != DAP_TRANSFER_OK)
        return -1;

    return 0;
}

int32_t DAP_HID::dap_swd_write_dp(uint8_t addr, uint32_t val)
{
    uint8_t req;
    uint8_t data[4];
    int32_t err;

    uint8_t resp = 0;
    uint32_t rx_data;

    switch (addr)
    {
    case DP_SELECT:
        if (dap_state.select == val)
        {
            return 1;
        }

        dap_state.select = val;
        break;

    default:
        break;
    }

    req = DAP_TRANS_DP | DAP_TRANS_WRITE_REG | DAP_TRANS_REG_ADDR(addr);

    for (uint8_t i = 0; i < 10; i++)
    {
        err = dap_swd_transfer(req, val, &resp, &rx_data);
        if (err < 0)
            return -1;

        if ((resp != DAP_TRANSFER_WAIT))
            break;
    }

    if (resp != DAP_TRANSFER_OK)
        return -1;

    return 0;
}

int32_t DAP_HID::dap_swd_read_ap(uint8_t addr, uint32_t *val)
{
    int32_t err;
    uint8_t req;
    uint8_t resp = 0;

    uint32_t apsel = addr & 0xff000000;
    uint32_t bank_sel = addr & APBANKSEL;

    err = dap_swd_write_dp(DP_SELECT, apsel | bank_sel);
    if (err < 0)
        return 0;

    req = DAP_TRANS_AP | DAP_TRANS_READ_REG | DAP_TRANS_REG_ADDR(addr);

    for (uint8_t i = 0; i < 10; i++)
    {
        err = dap_swd_transfer(req, 0, &resp, val);
        if (err < 0)
            return -1;

        if ((resp != DAP_TRANSFER_WAIT))
            break;
    }

    if (resp != DAP_TRANSFER_OK)
        return -1;

    return 0;
}

int32_t DAP_HID::dap_swd_write_ap(uint8_t addr, uint32_t val)
{
    uint32_t apsel = addr & 0xff000000;
    uint32_t bank_sel = addr & APBANKSEL;
    int32_t err;
    uint8_t req;
    uint8_t resp;
    uint32_t rx_data;

    err = dap_swd_write_dp(DP_SELECT, apsel | bank_sel);
    if (err < 0)
        return 0;

    switch (addr)
    {
    case AP_CSW:
        if (dap_state.csw == val)
        {
            return 1;
        }

        dap_state.csw = val;
        break;

    default:
        break;
    }

    req = DAP_TRANS_AP | DAP_TRANS_WRITE_REG | DAP_TRANS_REG_ADDR(addr);
    for (uint8_t i = 0; i < 10; i++)
    {
        err = dap_swd_transfer(req, val, &resp, &rx_data);
        if (err < 0)
            return -1;

        if ((resp != DAP_TRANSFER_WAIT))
            break;
    }

    if (resp != DAP_TRANSFER_OK)
        return -1;

    req = DAP_TRANS_DP | DAP_TRANS_READ_REG | DAP_TRANS_REG_ADDR(DP_RDBUFF);
    for (uint8_t i = 0; i < 10; i++)
    {
        err = dap_swd_transfer(req, val, &resp, &rx_data);
        if (err < 0)
            return -1;

        if ((resp != DAP_TRANSFER_WAIT))
            break;
    }

    if (resp != DAP_TRANSFER_OK)
        return -1;

    return 0;
}

int32_t DAP_HID::dap_swd_read_idcode(uint32_t *idcode)
{
    int32_t err;

    uint8_t tmp_in[1];
    tmp_in[0] = 0x00;

    err = dap_swj_sequence(8, tmp_in);
    if (err < 0)
        return err;

    return dap_swd_read_dp(DP_IDCODE, idcode);
}

int32_t DAP_HID::dap_read_data(uint32_t addr, uint32_t *data)
{
    int32_t err;
    uint8_t req;
    uint8_t resp;
    uint32_t rx_data;

    //  put addr in TAR register
    req = DAP_TRANS_AP | DAP_TRANS_WRITE_REG | (1 << 2);
    for (uint8_t i = 0; i < 10; i++)
    {
        err = dap_swd_transfer(req, addr, &resp, &rx_data);
        if (err < 0)
            return -1;

        if ((resp != DAP_TRANSFER_WAIT))
            break;
    }

    if (resp != DAP_TRANSFER_OK)
        return -1;

    // read data
    req = DAP_TRANS_AP | DAP_TRANS_READ_REG | (3 << 2);
    for (uint8_t i = 0; i < 10; i++)
    {
        err = dap_swd_transfer(req, 0, &resp, data);
        if (err < 0)
            return -1;

        if ((resp != DAP_TRANSFER_WAIT))
            break;
    }

    if (resp != DAP_TRANSFER_OK)
        return -1;

    // qDebug("[swd_init_debug] read_data 0x%08X", rx_data);

    // dummy read
    req = DAP_TRANS_DP | DAP_TRANS_READ_REG | DAP_TRANS_REG_ADDR(DP_RDBUFF);
    for (uint8_t i = 0; i < 10; i++)
    {
        err = dap_swd_transfer(req, 0, &resp, &rx_data);
        if (err < 0)
            return -1;

        if ((resp != DAP_TRANSFER_WAIT))
            break;
    }

    if (resp != DAP_TRANSFER_OK)
        return -1;

    // qDebug("[swd_init_debug] read_data 0x%08X", data);

    return 0;
}

int32_t DAP_HID::dap_write_data(uint32_t addr, uint32_t data)
{
    int32_t err;
    uint8_t req;
    uint8_t resp;
    uint32_t rx_data;

    //  put addr in TAR register
    req = DAP_TRANS_AP | DAP_TRANS_WRITE_REG | (1 << 2);
    for (uint8_t i = 0; i < 10; i++)
    {
        err = dap_swd_transfer(req, addr, &resp, &rx_data);
        if (err < 0)
            return -1;

        if ((resp != DAP_TRANSFER_WAIT))
            break;
    }

    if (resp != DAP_TRANSFER_OK)
        return -1;

    //  write data
    req = DAP_TRANS_AP | DAP_TRANS_WRITE_REG | (3 << 2);
    for (uint8_t i = 0; i < 10; i++)
    {
        err = dap_swd_transfer(req, data, &resp, &rx_data);
        if (err < 0)
            return -1;

        if ((resp != DAP_TRANSFER_WAIT))
            break;
    }

    if (resp != DAP_TRANSFER_OK)
        return -1;

    // dummy read
    req = DAP_TRANS_DP | DAP_TRANS_READ_REG | DAP_TRANS_REG_ADDR(DP_RDBUFF);
    for (uint8_t i = 0; i < 10; i++)
    {
        err = dap_swd_transfer(req, 0, &resp, &rx_data);
        if (err < 0)
            return -1;

        if ((resp != DAP_TRANSFER_WAIT))
            break;
    }

    if (resp != DAP_TRANSFER_OK)
        return -1;

    return 0;
}

int32_t DAP_HID::dap_read_word(uint32_t addr, uint32_t *val)
{
    int32_t err;

    err = dap_swd_write_ap(AP_CSW, CSW_VALUE | CSW_SIZE32);
    if (err < 0)
        return err;

    err = dap_read_data(addr, val);
    if (err < 0)
        return err;

    return 0;
}

int32_t DAP_HID::dap_write_word(uint32_t addr, uint32_t val)
{
    int32_t err;

    err = dap_swd_write_ap(AP_CSW, CSW_VALUE | CSW_SIZE32);
    if (err < 0)
        return err;

    err = dap_write_data(addr, val);
    if (err < 0)
        return err;

    return 0;
}

int32_t DAP_HID::dap_jtag_2_swd()
{
    int32_t err;
    uint32_t idcode;

    err = dap_swd_reset();
    if (err < 0)
        return err;

    err = dap_swd_switch(0xE79E);
    if (err < 0)
        return err;

    err = dap_swd_reset();
    if (err < 0)
        return err;

    err = dap_swd_read_idcode(&idcode);
    if (err < 0)
        return err;

    return 0;
}

int32_t DAP_HID::swd_init_debug()
{
    int32_t err;
    uint32_t tmp = 0;
    int timeout = 100;
    uint8_t i = 0;

    err = dap_jtag_2_swd();
    if (err < 0)
        return err;

    err = dap_swd_write_dp(DP_ABORT, STKCMPCLR | STKERRCLR | WDERRCLR | ORUNERRCLR);
    if (err < 0)
    {
        qDebug("[swd_init_debug] dap_swd_write_dp DP_ABORT fail");
        return err;
    }

    // Ensure CTRL/STAT register selected in DPBANKSEL
    err = dap_swd_write_dp(DP_SELECT, 0);
    if (err < 0)
    {
        qDebug("[swd_init_debug] dap_swd_write_dp DP_SELECT fail");
        return err;
    }

    // Power up
    err = dap_swd_write_dp(DP_CTRL_STAT, CSYSPWRUPREQ | CDBGPWRUPREQ);
    if (err < 0)
    {
        qDebug("[swd_init_debug] Power up fail");
        return err;
    }

    for (i = 0; i < timeout; i++)
    {
        err = dap_swd_read_dp(DP_CTRL_STAT, &tmp);
        if (err < 0)
            return err;

        if ((tmp & (CDBGPWRUPACK | CSYSPWRUPACK)) == (CDBGPWRUPACK | CSYSPWRUPACK))
        {
            // Break from loop if powerup is complete
            break;
        }
    }

    if (i == timeout)
    {
        return -1;
    }

    err = dap_swd_write_dp(DP_CTRL_STAT, CSYSPWRUPREQ | CDBGPWRUPREQ | TRNNORMAL | MASKLANE);
    if (err < 0)
        return err;

    // call a target dependant function:
    // some target can enter in a lock state, this function can unlock these targets
    // target_unlock_sequence();

    err = dap_swd_write_dp(DP_SELECT, 0);
    if (err < 0)
        return err;

    return 0;
}

#define SCS_BASE (0xE000E000UL)
#define SCB_BASE (SCS_BASE + 0x0D00UL)

#define SCB_AIRCR_VECTKEY_Pos 16U                                    /*!< SCB AIRCR: VECTKEY Position */
#define SCB_AIRCR_VECTKEY_Msk (0xFFFFUL << SCB_AIRCR_VECTKEY_Pos)    /*!< SCB AIRCR: VECTKEY Mask */
#define SCB_AIRCR_PRIGROUP_Pos 8U                                    /*!< SCB AIRCR: PRIGROUP Position */
#define SCB_AIRCR_PRIGROUP_Msk (7UL << SCB_AIRCR_PRIGROUP_Pos)       /*!< SCB AIRCR: PRIGROUP Mask */
#define SCB_AIRCR_SYSRESETREQ_Pos 2U                                 /*!< SCB AIRCR: SYSRESETREQ Position */
#define SCB_AIRCR_SYSRESETREQ_Msk (1UL << SCB_AIRCR_SYSRESETREQ_Pos) /*!< SCB AIRCR: SYSRESETREQ Mask */

#define SCB_AIRCR 0xE000ED0C

int32_t DAP_HID::dap_set_target_reset(uint8_t asserted)
{
    int32_t err;
    uint32_t val_scb_aircr;

    if (asserted == 0)
    {
        err = dap_read_word((uint32_t)SCB_AIRCR, &val_scb_aircr);
        if (err < 0)
            return err;

        // qDebug("[swd_init_debug] set_target_reset SCB_AIRCR: 0x%08X", val_scb_aircr);

        err = dap_write_word(
            (uint32_t)SCB_AIRCR,
            ((0x5FA << SCB_AIRCR_VECTKEY_Pos) | (0 & SCB_AIRCR_PRIGROUP_Msk) | SCB_AIRCR_SYSRESETREQ_Msk));
        if (err < 0)
            return err;
    }

    return 0;
}

int32_t DAP_HID::dap_set_target_state_hw(dap_target_reset_state_t state)
{
    uint32_t err;
    uint32_t val;
    int8_t ap_retries = 2;
    uint16_t retry = 32;

    switch (state)
    {
    case DAP_TARGET_RESET_HOLD:
        err = dap_set_target_reset(1);
        if (err < 0)
            return err;
        return 0;

    case DAP_TARGET_RESET_RUN:
        err = dap_set_target_reset(1);
        if (err < 0)
            return err;
        QThread::msleep(20);
        err = dap_set_target_reset(0);
        if (err < 0)
            return err;
        QThread::msleep(20);
        return 0;

    case DAP_TARGET_RESET_PROGRAM:

        err = swd_init_debug();
        if (err < 0)
            return err;

        // Enable debug
        while (1)
        {
            err = dap_write_word(DBG_HCSR, DBGKEY | C_DEBUGEN);
            if (err < 0)
            {
                ap_retries--;
                if (ap_retries == 0)
                    return -1;
            }
            else
            {
                break;
            }

            err = dap_set_target_reset(1);
            if (err < 0)
                return err;
            QThread::msleep(20);
            err = dap_set_target_reset(0);
            if (err < 0)
                return err;
            QThread::msleep(20);
        }

        // Enable halt on reset
        err = dap_write_word(DBG_EMCR, VC_CORERESET);
        if (err < 0)
            return err;

        // Reset again
        err = dap_set_target_reset(1);
        if (err < 0)
            return err;
        QThread::msleep(20);
        err = dap_set_target_reset(0);
        if (err < 0)
            return err;
        QThread::msleep(20);

        do
        {
            if (retry == 0)
                return -1;

            err = dap_read_word(DBG_HCSR, &val);
            if (err < 0)
                return err;

            if (retry)
                retry--;

        } while ((val & S_HALT) == 0);

        // Disable halt on reset
        err = dap_write_word(DBG_EMCR, 0);
        if (err < 0)
            return err;

        return 0;

    case DAP_TARGET_RUN:
    {
        int32_t err;
        uint32_t idcode;

        err = dap_swd_reset();
        if (err < 0)
        {
            qDebug("[DAP_HID] connect dap_swd_reset fail");
            return err;
        }

        // JTAG-to-SWD
        err = dap_swd_switch(0xE79E);
        if (err < 0)
        {
            qDebug("[DAP_HID] connect dap_swd_switch fail");
            return err;
        }

        err = dap_swd_reset();
        if (err < 0)
        {
            qDebug("[DAP_HID] connect dap_swd_reset fail");
            return err;
        }

        err = dap_swd_read_idcode(&idcode);
        if (err < 0)
        {
            qDebug("[DAP_HID] connect dap_swd_read_idcode fail idcode:0x%08X", idcode);
        }
        else
        {
            qDebug("[DAP_HID] connect dap_swd_read_idcode 0x%08X", idcode);
        }

        err = swd_init_debug();
        if (err < 0)
        {
            qDebug("[DAP_HID] connect swd_init_debug fail");
            return err;
        }

        err = dap_write_word(DBG_HCSR, DBGKEY);
        if (err < 0)
        {
            return err;
        }
        break;
    }

    default:
        return -1;
    }

    return 0;
}

int32_t DAP_HID::dap_read_byte(uint32_t addr, uint8_t *val)
{
    int32_t err;
    uint32_t tmp;

    err = dap_swd_write_ap(AP_CSW, CSW_VALUE | CSW_SIZE8);
    if (err < 0)
        return err;

    err = dap_read_data(addr, &tmp);
    if (err < 0)
        return err;

    *val = (uint8_t)(tmp >> ((addr & 0x03) << 3));

    qDebug("[DAP_HID] dap_read_byte addr:0x%X val:%08X", addr, *val);

    return 0;
}

int32_t DAP_HID::dap_write_byte(uint32_t addr, uint8_t val)
{
    int32_t err;
    uint32_t tmp;

    err = dap_swd_write_ap(AP_CSW, CSW_VALUE | CSW_SIZE8);
    if (err < 0)
        return err;

    tmp = val << ((addr & 0x03) << 3);

    err = dap_write_data(addr, tmp);
    if (err < 0)
        return err;

    return 0;
}

int32_t DAP_HID::dap_read_block(uint32_t addr, uint8_t *data, uint32_t size)
{
    int32_t err;
    uint8_t req, resp;
    uint32_t size_in_words;
    uint32_t i;

    uint8_t *prev_data = data;

    if (size == 0)
        return 0;

    size_in_words = size / 4;
    err = dap_swd_write_ap(AP_CSW, CSW_VALUE | CSW_SIZE32);
    if (err < 0)
        return err;

    // TAR write
    req = DAP_TRANS_AP | DAP_TRANS_WRITE_REG | AP_TAR;
    err = dap_swd_transfer_retry(req, addr, &resp, NULL);
    if (err < 0)
        return err;

    // read data
    req = DAP_TRANS_AP | DAP_TRANS_READ_REG | AP_DRW;

    // TODO:为什么有的芯片需要加这个dummy而有的不用
    // initiate first read, data comes back in next read
    // err = dap_swd_transfer_retry(req, 0, &resp, NULL);
    // if (err < 0)
    //     return err;

    for (i = 0; i < (size_in_words); i++)
    {
        err = dap_swd_transfer_retry(req, 0, &resp, (uint32_t *)data);
        if (err < 0)
            return err;

        data += 4;
    }

    // read last word
    // req = DAP_TRANS_DP | DAP_TRANS_READ_REG | DAP_TRANS_REG_ADDR(DP_RDBUFF);
    // err = dap_swd_transfer_retry(req, 0, &resp, (uint32_t *)data);
    // if (err < 0)
    //     return err;
    req = DAP_TRANS_DP | DAP_TRANS_READ_REG | DAP_TRANS_REG_ADDR(DP_RDBUFF);
    err = dap_swd_transfer_retry(req, 0, &resp, NULL);
    if (err < 0)
        return err;

    // qDebug("[DAP_HID] read_block addr:0x%X size:0x%X", addr, size);
    // hexdump(prev_data, size);

    return 0;
}

int32_t DAP_HID::dap_write_block(uint32_t addr, uint8_t *data, uint32_t size)
{
    int32_t err;
    uint8_t req, resp;
    uint32_t size_in_words;
    uint32_t i;

    if (size == 0)
    {
        return 0;
    }

    size_in_words = size / 4;
    err = dap_swd_write_ap(AP_CSW, CSW_VALUE | CSW_SIZE32);
    if (err < 0)
        return err;

    // TAR write
    req = DAP_TRANS_AP | DAP_TRANS_WRITE_REG | AP_TAR;
    err = dap_swd_transfer_retry(req, addr, &resp, NULL);
    if (err < 0)
        return err;

    // DRW write
    req = DAP_TRANS_AP | DAP_TRANS_WRITE_REG | AP_DRW;

    for (i = 0; i < size_in_words; i++)
    {
        err = dap_swd_transfer_retry(req, *((uint32_t *)data), &resp, NULL);
        if (err < 0)
            return err;

        data += 4;
    }

    // dummy read
    req = DAP_TRANS_DP | DAP_TRANS_READ_REG | DAP_TRANS_REG_ADDR(DP_RDBUFF);
    err = dap_swd_transfer_retry(req, 0, &resp, NULL);
    if (err < 0)
        return err;

    return 0;
}

int32_t DAP_HID::dap_read_memory(uint32_t addr, uint8_t *data, uint32_t size)
{
    int32_t err;
    uint32_t n;

    // Read bytes until word aligned
    while ((size > 0) && (addr & 0x3))
    {
        err = dap_read_byte(addr, data);
        if (err < 0)
        {
            return err;
        }

        addr++;
        data++;
        size--;
    }

    // Read word aligned blocks
    while (size > 3)
    {
        // Limit to auto increment page size
        n = Flash_Page_Size - (addr & (Flash_Page_Size - 1));

        if (size < n)
        {
            n = size & 0xFFFFFFFC; // Only count complete words remaining
        }

        err = dap_read_block(addr, data, n);
        if (err < 0)
        {
            return err;
        }

        addr += n;
        data += n;
        size -= n;
    }

    // Read remaining bytes
    while (size > 0)
    {
        err = dap_read_byte(addr, data);
        if (err < 0)
            return err;

        addr++;
        data++;
        size--;
    }

    return 0;
}

int32_t DAP_HID::dap_write_memory(uint32_t addr, uint8_t *data, uint32_t size)
{
    int32_t err;
    uint32_t n = 0;

    // Write bytes until word aligned
    while ((size > 0) && (addr & 0x3))
    {
        err = dap_write_byte(addr, *data);
        if (err < 0)
            return err;

        addr++;
        data++;
        size--;
    }

    // Write word aligned blocks
    while (size > 3)
    {
        // Limit to auto increment page size
        n = Flash_Page_Size - (addr & (Flash_Page_Size - 1));

        if (size < n)
        {
            n = size & 0xFFFFFFFC; // Only count complete words remaining
        }

        err = dap_write_block(addr, data, n);
        if (err < 0)
            return err;

        addr += n;
        data += n;
        size -= n;
    }

    // Write remaining bytes
    while (size > 0)
    {
        err = dap_write_byte(addr, *data);
        if (err < 0)
            return err;

        addr++;
        data++;
        size--;
    }

    return 0;
}

int32_t DAP_HID::swd_write_debug_state(debug_state_t *state)
{
    int32_t err;
    uint32_t i, status;

    err = dap_swd_write_dp(DP_SELECT, 0);
    if (err < 0)
        return err;

    // R0, R1, R2, R3
    for (i = 0; i < 4; i++)
    {
        err = swd_write_core_register(i, state->r[i]);
        if (err < 0)
            return err;
    }

    // R9
    err = swd_write_core_register(9, state->r[9]);
    if (err < 0)
        return err;

    // R13, R14, R15
    for (i = 13; i < 16; i++)
    {
        err = swd_write_core_register(i, state->r[i]);
        if (err < 0)
            return err;
    }

    // xPSR
    err = swd_write_core_register(16, state->xpsr);
    if (err < 0)
        return err;

    err = dap_write_word(DBG_HCSR, DBGKEY | C_DEBUGEN);
    if (err < 0)
        return err;

    // check status
    err = dap_swd_read_dp(DP_CTRL_STAT, &status);
    if (err < 0)
        return err;

    if (status & (STICKYERR | WDATAERR))
        return -1;

    return 0;
}

int32_t DAP_HID::swd_read_core_register(uint32_t n, uint32_t *val)
{
    int32_t err;
    int i = 0, timeout = 100;

    err = dap_write_word(DCRSR, n);
    if (err < 0)
        return err;

    // wait for S_REGRDY
    for (i = 0; i < timeout; i++)
    {
        err = dap_read_word(DHCSR, val);
        if (err < 0)
            return err;

        if (*val & S_REGRDY)
        {
            break;
        }
    }

    if (i == timeout)
        return -1;

    err = dap_read_word(DCRDR, val);
    if (err < 0)
        return err;

    return 0;
}

int32_t DAP_HID::swd_write_core_register(uint32_t n, uint32_t val)
{
    int32_t err;
    int i = 0, timeout = 100;

    err = dap_write_word(DCRDR, val);
    if (err < 0)
        return err;

    err = dap_write_word(DCRSR, n | REGWnR);
    if (err < 0)
        return err;

    // wait for S_REGRDY
    for (i = 0; i < timeout; i++)
    {
        err = dap_read_word(DHCSR, &val);
        if (err < 0)
            return err;

        if (val & S_REGRDY)
        {
            return 0;
        }
    }

    return -1;
}

int32_t DAP_HID::swd_wait_until_halted(void)
{
    // Wait for target to stop
    int32_t err;
    uint32_t val, i, timeout = 1500;

    // TODO: 需要根据实际的情况设置超时值
    for (i = 0; i < timeout; i++)
    {
        err = dap_read_word(DBG_HCSR, &val);
        if (err < 0)
            return -1;

        if (val & S_HALT)
            return 0;
    }

    return -1;
}

int32_t DAP_HID::swd_flash_syscall_exec(const program_syscall_t *sysCallParam, uint32_t entry, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4)
{
    int32_t err;
    debug_state_t state = {{0}, 0};

    // Call flash algorithm function on target and wait for result.
    state.r[0] = arg1;                         // R0: Argument 1
    state.r[1] = arg2;                         // R1: Argument 2
    state.r[2] = arg3;                         // R2: Argument 3
    state.r[3] = arg4;                         // R3: Argument 4
    state.r[9] = sysCallParam->static_base;    // SB: Static Base
    state.r[13] = sysCallParam->stack_pointer; // SP: Stack Pointer
    state.r[14] = sysCallParam->breakpoint;    // LR: Exit Point
    state.r[15] = entry;                       // PC: Entry Point
    state.xpsr = 0x01000000;                   // xPSR: T = 1, ISR = 0

    err = swd_write_debug_state(&state);
    if (err < 0)
    {
        qDebug("[DA_PHID] swd_write_debug_state fail");
        return err;
    }

    err = swd_wait_until_halted();
    if (err < 0)
    {
        qDebug("[DA_PHID] wait_until_halted fail");
        return err;
    }

    err = swd_read_core_register(0, &state.r[0]);
    if (err < 0)
    {
        qDebug("[DA_PHID] swd_read_core_register fail");
        return err;
    }

    // Flash functions return 0 if successful.
    if (state.r[0] != 0)
    {
        qDebug("[DAP_HID] flash_func return:0x%X", state.r[0]);
        return -1;
    }

    return 0;
}
