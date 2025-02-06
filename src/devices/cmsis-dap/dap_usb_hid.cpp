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

int32_t DAP_HID::dap_request(uint8_t *tx_data, uint32_t tx_len, uint8_t *rx_data, uint32_t rx_buf_size, uint32_t *rx_len)
{
    int32_t err;
    uint8_t tx_buf[65] = {0};

    if (tx_len > 64)
        return -1;

    tx_buf[0] = 0x00;
    memcpy(tx_buf + 1, tx_data, tx_len);

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

    err = hid_read_timeout(dev, rx_data, rx_buf_size, 500);
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

    *rx_len = err;
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
        return -1;
    }

    err = hid_read_timeout(dev, rx_data, 64, 100);
    if (err < 0)
    {
        QString err_info = QString::fromWCharArray(hid_error(dev));
        qDebug("[DAP_HID] dap_hid_request fail: %s", qUtf8Printable(err_info));
        return -1;
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
