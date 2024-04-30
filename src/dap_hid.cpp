#include "dap_hid.h"
#include "debug_cm.h"
#include <QtGlobal>

const char *dap_state_to_string(uint8_t state)
{
    if (state == DAP_OK)
        return "DAP_OK";
    else if (state == DAP_ERROR)
        return "DAP_ERROR";
    else
        return "???";
}

QString parse_dap_hid_info_resp(uint16_t data)
{
    QStringList impl_list;

    if (data & 0x01)
        impl_list.append(QString("SWD"));

    if (data & 0x02)
        impl_list.append(QString("JTAG"));

    if (data & 0x04)
        impl_list.append(QString("SWO_UART"));

    if (data & 0x08)
        impl_list.append(QString("SWO_Manchester"));

    if (data & 0x10)
        impl_list.append(QString("Atomic_Commands"));

    if (data & 0x20)
        impl_list.append(QString("Test Domain Timer"));

    if (data & 0x40)
        impl_list.append(QString("SWO Streaming_Trace"));

    if (data & 0x80)
        impl_list.append(QString("UART_Communication_Port"));

    if (data & 0x0100)
        impl_list.append(QString("USB_COM_Port"));

    return impl_list.join(" ");
}

void hexdump(const uint8_t *buf, uint32_t len)
{
    uint32_t i = 0;
    uint32_t line_i = 0;

    QString res("");

    res.append("[hexdump] ====================================================\r\n");
    res.append("[hexdump]       0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\r\n");

    for (i = 0; i < len; i += 0x10)
    {
        res.append(QString("[hexdump]"));
        res.append(QString(" %1").arg((int)(i / 16), 4, 16, QChar('0')).toUpper());

        for (line_i = 0; line_i < 0x10; line_i++)
        {
            if ((i + line_i) >= len)
            {
                res.append("   ");
                continue;
            }

            res.append(QString(" %1").arg(*(buf + i + line_i), 2, 16, QChar('0')).toUpper());
        }

        res.append("\r\n");
    }

    res.append("[hexdump] ====================================================\r\n");

    qDebug("%s", qPrintable(res));
}

DAP_HID::DAP_HID(QString usb_path, QWidget *parent)
{
    this->usb_path = usb_path;
    int32_t err;

    uint32_t idcode;

    err = open_device();
    if (err < 0)
    {
        qDebug("[enum_device] open fail");
        return;
    }

    dap_hid_get_info();

    // qDebug("    vendor_name: %s", qPrintable(dap_get_info_vendor_name()));
    // qDebug("    product_name: %s", qPrintable(dap_get_info_product_name()));
    // qDebug("    serial_number: %s", qPrintable(dap_get_info_serial_number()));
    // qDebug("    protocol_version: %s", qPrintable(dap_get_info_cmsis_dap_protocol_version()));

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
}

DAP_HID::~DAP_HID()
{
    int32_t err;

    err = dap_disconnect();
    if (err < 0)
    {
        qDebug("[enum_device] dap_disconnect fail");
        return;
    }
    close_device();
}

/*******************************************************************************
 * @brief 枚举扫描设备
 * @param None
 * @return None
 ******************************************************************************/
int32_t DAP_HID::enum_device(QList<DAP_HID *> *dev_list)
{
    uint32_t i = 0;
    hid_device_info *device_info = hid_enumerate(DAP_HID_VID, DAP_HID_PID);
    if (device_info == NULL)
    {
        // qDebug("[enum_device] is empty");

        hid_free_enumeration(device_info);
        return 0;
    }

    dev_list->clear();
    // for (i = 0; i < dev_list->count(); i++)
    // {
    //     DAP_HID tmp_dev = dev_list->first();
    //     dev_list->pop_front();
    //     delete tmp_dev;
    // }

    i = 0;
    while (1)
    {

        DAP_HID *tmp_dev = new DAP_HID(device_info->path);

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

int32_t DAP_HID::open_device()
{
    // qDebug("[DAP_HID] open device %s", qPrintable(usb_path));

    hid_device *hid_dev = hid_open_path(usb_path.toUtf8().constData());
    if (hid_dev == NULL)
    {
        QString err_info = QString::fromWCharArray(hid_error(NULL));
        qDebug("[DAP_HID] hid open fail: %s", qPrintable(err_info));
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

int32_t DAP_HID::dap_hid_request(uint8_t *tx_data, uint8_t *rx_data)
{
    int32_t err;
    uint8_t tx_buf[65] = {0};

    tx_buf[0] = 0x00;
    memcpy(tx_buf + 1, tx_data, 64);

    err = hid_write(dev, tx_buf, 65);
    if (err < 0)
    {
        QString err_info = QString::fromWCharArray(hid_error(dev));
        qDebug("[DAP_HID] dap_hid_requst fail: %s", qPrintable(err_info));
        return err;
    }

    err = hid_read_timeout(dev, rx_data, 64, 100);
    if (err < 0)
    {
        QString err_info = QString::fromWCharArray(hid_error(dev));
        qDebug("[DAP_HID] dap_hid_request fail: %s", qPrintable(err_info));
        return err;
    }

    // 首字节一定与发送相等
    if (rx_data[0] != tx_data[0])
        return -1;

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

void DAP_HID::dap_hid_get_info()
{
    if (dev == NULL)
    {
        return;
    }

    hid_device_info *info = hid_get_device_info(dev);
    if (info == NULL)
    {
        qDebug("[dap_hid] hid_get_device_info fail");
        return;
    }

    hid_manufacturer = QString::fromWCharArray(info->manufacturer_string);
    hid_product = QString::fromWCharArray(info->product_string);

    // qDebug("[dap_hid] hid_get_device_info ok");
    // qDebug("[dap_hid] manufacturer_string: %s", qPrintable(manufacturer));
    // qDebug("[dap_hid] product_string: %s", qPrintable(product));
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

    // qDebug("[DAP_HID] dap_swd_sequence_write resp:");
    // hexdump(rx_buf, 64);

    uint8_t status = rx_buf[1];

    return (status == DAP_OK) ? 0 : -1;
}

int32_t DAP_HID::dap_swd_config(uint8_t cfg)
{
    uint8_t tx_buf[65] = {0};
    uint8_t rx_buf[64] = {0};
    int32_t err;

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
    uint8_t tx_buf[65] = {0};
    uint8_t rx_buf[64] = {0};
    int32_t err;

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
    memcpy(rx_data, rx_buf + 3, 4);

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
        return -1;

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

        qDebug("[swd_init_debug] set_target_reset SCB_AIRCR: 0x%08X", val_scb_aircr);

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
            err = dap_read_word(DBG_HCSR, &val);
            if (err < 0)
                return err;
        } while ((val & S_HALT) == 0);

        return 0;
    default:
        return -1;
    }
}
