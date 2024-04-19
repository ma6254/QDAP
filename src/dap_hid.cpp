#include "dap_hid.h"
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

    // qDebug("    vendor_name: %s", qPrintable(dap_get_info_vendor_name()));
    // qDebug("    product_name: %s", qPrintable(dap_get_info_product_name()));
    // qDebug("    serial_number: %s", qPrintable(dap_get_info_serial_number()));
    // qDebug("    protocol_version: %s", qPrintable(dap_get_info_cmsis_dap_protocol_version()));
    // qDebug("    caps: 0x%X", dap_get_info_caps());
    // qDebug("    freq: %d", dap_get_info_freq());

    err = dap_connect(1);
    if (err < 0)
    {
        qDebug("[enum_device] dap_connect fail");
        return;
    }

    err = dap_reset_target();
    if (err < 0)
    {
        qDebug("[enum_device] dap_reset_target fail");
        return;
    }

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

    err = dap_swd_reset();
    if (err < 0)
    {
        qDebug("[enum_device] dap_swd_reset fail");
        return;
    }

    err = dap_swd_switch(0xE79E);
    if (err < 0)
    {
        qDebug("[enum_device] dap_swd_reset fail");
        return;
    }

    err = dap_swd_reset();
    if (err < 0)
    {
        qDebug("[enum_device] dap_swd_reset fail");
        return;
    }

    err = dap_swd_read_idcode(&idcode);
    if (err < 0)
    {
        qDebug("[enum_device] dap_swd_read_idcode fail");
    }
    else
    {
        qDebug("[enum_device] dap_swd_read_idcode 0x%08X", idcode);
    }
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
int32_t DAP_HID::enum_device()
{
    hid_device_info *device_info = hid_enumerate(DAP_HID_VID, DAP_HID_PID);
    if (device_info == NULL)
    {
        qDebug("[enum_device] is empty");

        hid_free_enumeration(device_info);
        return 0;
    }

    uint32_t i = 0;
    while (1)
    {

        DAP_HID *tmp_dev = new DAP_HID(device_info->path);
        qDebug("[enum_device] #%d. ser:%d %ls %ls", i,
               device_info->interface_number,
               device_info->manufacturer_string,
               device_info->product_string);

        i++;
        if (device_info->next == NULL)
            break;

        device_info = device_info->next;
    }

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

QString DAP_HID::dap_get_info_vendor_name()
{
    uint8_t tx_buf[65] = {0};
    uint8_t rx_buf[64] = {0};
    int32_t err;

    // 报告编号
    tx_buf[0] = 0x00;
    tx_buf[1] = 0x00;
    tx_buf[2] = 0x01;

    err = hid_write(dev, tx_buf, 3);
    if (err < 0)
    {
        QString err_info = QString::fromWCharArray(hid_error(dev));
        qDebug("[DAP_HID] hid_write fail: %s", qPrintable(err_info));
        return "";
    }

    err = hid_read_timeout(dev, rx_buf, 64, 100);
    if (err < 0)
    {
        QString err_info = QString::fromWCharArray(hid_error(dev));
        qDebug("[DAP_HID] hid_write fail: %s", qPrintable(err_info));
        return "";
    }

    uint8_t len = rx_buf[1];
    if (len > 62)
        len = 62;

    // hexdump(rx_buf, 64);
    // qDebug("[dap_get_info_vendor_name] ok");
    return QString(QLatin1String((char *)rx_buf + 2, len));
}

QString DAP_HID::dap_get_info_product_name()
{
    uint8_t tx_buf[65] = {0};
    uint8_t rx_buf[64] = {0};
    int32_t err;

    // 报告编号
    tx_buf[0] = 0x00;
    tx_buf[1] = 0x00;
    tx_buf[2] = 0x02;

    err = hid_write(dev, tx_buf, 3);
    if (err < 0)
    {
        QString err_info = QString::fromWCharArray(hid_error(dev));
        qDebug("[DAP_HID] hid_write fail: %s", qPrintable(err_info));
        return "";
    }

    err = hid_read_timeout(dev, rx_buf, 64, 100);
    if (err < 0)
    {
        QString err_info = QString::fromWCharArray(hid_error(dev));
        qDebug("[DAP_HID] hid_write fail: %s", qPrintable(err_info));
        return "";
    }

    uint8_t len = rx_buf[1];
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
    uint8_t tx_buf[65] = {0};
    uint8_t rx_buf[64] = {0};
    int32_t err;

    // 报告编号
    tx_buf[0] = 0x00;
    tx_buf[1] = 0x00;
    tx_buf[2] = 0x03;

    err = hid_write(dev, tx_buf, 3);
    if (err < 0)
    {
        QString err_info = QString::fromWCharArray(hid_error(dev));
        qDebug("[DAP_HID] hid_write fail: %s", qPrintable(err_info));
        return "";
    }

    err = hid_read_timeout(dev, rx_buf, 64, 100);
    if (err < 0)
    {
        QString err_info = QString::fromWCharArray(hid_error(dev));
        qDebug("[DAP_HID] hid_write fail: %s", qPrintable(err_info));
        return "";
    }

    uint8_t len = rx_buf[1];
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
    uint8_t tx_buf[65] = {0};
    uint8_t rx_buf[64] = {0};
    int32_t err;

    // 报告编号
    tx_buf[0] = 0x00;
    tx_buf[1] = 0x00;
    tx_buf[2] = 0x04;

    err = hid_write(dev, tx_buf, 3);
    if (err < 0)
    {
        QString err_info = QString::fromWCharArray(hid_error(dev));
        qDebug("[DAP_HID] hid_write fail: %s", qPrintable(err_info));
        return "";
    }

    err = hid_read_timeout(dev, rx_buf, 64, 100);
    if (err < 0)
    {
        QString err_info = QString::fromWCharArray(hid_error(dev));
        qDebug("[DAP_HID] hid_write fail: %s", qPrintable(err_info));
        return "";
    }

    uint8_t len = rx_buf[1];
    if (len > 62)
        len = 62;

    // hexdump(rx_buf, 64);
    // qDebug("[dap_get_info_serial_number] ok");
    return QString(QLatin1String((char *)rx_buf + 2, len));
}

int32_t DAP_HID::dap_get_info_caps()
{
    uint8_t tx_buf[65] = {0};
    uint8_t rx_buf[64] = {0};
    int32_t err;

    // 报告编号
    tx_buf[0] = 0x00;
    tx_buf[1] = 0x00;
    tx_buf[2] = 0xF0;

    err = hid_write(dev, tx_buf, 3);
    if (err < 0)
    {
        QString err_info = QString::fromWCharArray(hid_error(dev));
        qDebug("[DAP_HID] hid_write fail: %s", qPrintable(err_info));
        return -1;
    }

    err = hid_read_timeout(dev, rx_buf, 64, 100);
    if (err < 0)
    {
        QString err_info = QString::fromWCharArray(hid_error(dev));
        qDebug("[DAP_HID] hid_write fail: %s", qPrintable(err_info));
        return -1;
    }

    // hexdump(rx_buf, 64);

    uint8_t len = rx_buf[1];

    if (len == 1)
    {
        return rx_buf[2];
    }
    else if (len == 2)
    {
        return ((uint16_t)rx_buf[2]) << 8 | ((uint16_t)rx_buf[3]);
    }
    else
    {
        return -1;
    }
}

/*******************************************************************************
 * @brief Get the CMSIS-DAP Protocol Version (string).
 * @param None
 * @return None
 ******************************************************************************/
int32_t DAP_HID::dap_get_info_freq()
{
    uint8_t tx_buf[65] = {0};
    uint8_t rx_buf[64] = {0};
    int32_t err;

    // 报告编号
    tx_buf[0] = 0x00;
    tx_buf[1] = 0x00;
    tx_buf[2] = 0xF1;

    err = hid_write(dev, tx_buf, 3);
    if (err < 0)
    {
        QString err_info = QString::fromWCharArray(hid_error(dev));
        qDebug("[DAP_HID] hid_write fail: %s", qPrintable(err_info));
        return -1;
    }

    err = hid_read_timeout(dev, rx_buf, 64, 100);
    if (err < 0)
    {
        QString err_info = QString::fromWCharArray(hid_error(dev));
        qDebug("[DAP_HID] hid_write fail: %s", qPrintable(err_info));
        return -1;
    }

    if (rx_buf[1] != 0x08)
        return -1;

    uint16_t freq = ((uint16_t)rx_buf[2]) << 8 | ((uint16_t)rx_buf[3]);

    // hexdump(rx_buf, 64);
    // qDebug("[get_info_serial_number] ok");
    return freq;
}

/*******************************************************************************
 * @brief Get the CMSIS-DAP Protocol Version (string).
 * @param None
 * @return None
 ******************************************************************************/
int32_t DAP_HID::dap_connect(uint8_t port)
{
    uint8_t tx_buf[65] = {0};
    uint8_t rx_buf[64] = {0};
    int32_t err;

    // 报告编号
    tx_buf[0] = 0x00;
    tx_buf[1] = DAP_CMD_CONNECT;
    tx_buf[2] = port;

    err = hid_write(dev, tx_buf, 3);
    if (err < 0)
    {
        QString err_info = QString::fromWCharArray(hid_error(dev));
        qDebug("[DAP_HID] hid_write fail: %s", qPrintable(err_info));
        return -1;
    }

    err = hid_read_timeout(dev, rx_buf, 64, 100);
    if (err < 0)
    {
        QString err_info = QString::fromWCharArray(hid_error(dev));
        qDebug("[DAP_HID] hid_write fail: %s", qPrintable(err_info));
        return -1;
    }

    // 首字节一定与发送相等
    if (rx_buf[0] != DAP_CMD_CONNECT)
        return -1;

    if (rx_buf[1] == 0)
        return -1;

    if (rx_buf[1] != port)
        return -1;

    // hexdump(rx_buf, 64);

    return 0;
}

int32_t DAP_HID::dap_disconnect()
{
    uint8_t tx_buf[65] = {0};
    uint8_t rx_buf[64] = {0};
    int32_t err;

    // 报告编号
    tx_buf[0] = 0x00;
    tx_buf[1] = DAP_CMD_DISCONNECT;

    err = hid_write(dev, tx_buf, 2);
    if (err < 0)
    {
        QString err_info = QString::fromWCharArray(hid_error(dev));
        qDebug("[DAP_HID] hid_write fail: %s", qPrintable(err_info));
        return -1;
    }

    err = hid_read_timeout(dev, rx_buf, 64, 100);
    if (err < 0)
    {
        QString err_info = QString::fromWCharArray(hid_error(dev));
        qDebug("[DAP_HID] hid_write fail: %s", qPrintable(err_info));
        return -1;
    }

    // 首字节一定与发送相等
    if (rx_buf[0] != DAP_CMD_DISCONNECT)
        return -1;

    uint8_t status = rx_buf[1];

    if (status == DAP_OK)
        return 1;
    else if (status == DAP_ERROR)
        return 0;
    else
        return -1;
}

int32_t DAP_HID::dap_reset_target()
{
    uint8_t tx_buf[65] = {0};
    uint8_t rx_buf[64] = {0};
    int32_t err;

    // 报告编号
    tx_buf[0] = 0x00;
    tx_buf[1] = DAP_CMD_RESET_TARGET;

    err = hid_write(dev, tx_buf, 2);
    if (err < 0)
    {
        QString err_info = QString::fromWCharArray(hid_error(dev));
        qDebug("[DAP_HID] hid_write fail: %s", qPrintable(err_info));
        return -1;
    }

    err = hid_read_timeout(dev, rx_buf, 64, 100);
    if (err < 0)
    {
        QString err_info = QString::fromWCharArray(hid_error(dev));
        qDebug("[DAP_HID] hid_write fail: %s", qPrintable(err_info));
        return -1;
    }

    // qDebug("[DAP_HID] dap_reset_target resp:");
    // hexdump(rx_buf, 64);

    // 首字节一定与发送相等
    if (rx_buf[0] != DAP_CMD_RESET_TARGET)
        return -1;

    uint8_t status = rx_buf[1];
    uint8_t execute = rx_buf[2];

    if (status == DAP_OK)
        return 1;
    else if (status == DAP_ERROR)
        return 0;
    else
        return -1;
}

int32_t DAP_HID::dap_swj_sequence(uint8_t bit_count, uint8_t *data)
{
    uint8_t tx_buf[65] = {0};
    uint8_t rx_buf[64] = {0};
    int32_t err;
    uint8_t count_byte;

    tx_buf[0] = 0x00; // 报告编号
    tx_buf[1] = 0x12;
    tx_buf[2] = bit_count; // Sequence Count
    // tx_buf[3] = tx_data; // SWDIO Data

    count_byte = bit_count / 8;
    if (bit_count % 8)
        count_byte += 1;

    memcpy(tx_buf + 3, data, count_byte);

    err = hid_write(dev, tx_buf, count_byte + 3);
    if (err < 0)
    {
        QString err_info = QString::fromWCharArray(hid_error(dev));
        qDebug("[DAP_HID] dap_swj_sequence fail: %s", qPrintable(err_info));
        return -1;
    }

    err = hid_read_timeout(dev, rx_buf, 64, 100);
    if (err < 0)
    {
        QString err_info = QString::fromWCharArray(hid_error(dev));
        qDebug("[DAP_HID] dap_swj_sequence fail: %s", qPrintable(err_info));
        return -1;
    }

    // qDebug("[DAP_HID] dap_swd_sequence_write resp:");
    // hexdump(rx_buf, 64);

    // 首字节一定与发送相等
    if (rx_buf[0] != 0x12)
        return -1;

    uint8_t status = rx_buf[1];

    if (status == DAP_OK)
        return 0;
    else
        return -1;
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
    uint8_t rx_data = rx_buf[2];

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

int32_t DAP_HID::dap_swd_transfer(uint8_t req, uint32_t tx_data, uint8_t *resp, uint32_t *timestamp, uint32_t *rx_data)
{
    uint8_t tx_buf[65] = {0};
    uint8_t rx_buf[64] = {0};
    int32_t err;

    // 报告编号
    tx_buf[0] = 0x00;
    tx_buf[1] = 0x05;
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
    if (rx_buf[0] != 0x05)
        return -1;

    qDebug("[DAP_HID] dap_swd_transfer resp:");
    hexdump(rx_buf, 64);

    *(resp) = rx_buf[2];
    // memcpy(timestamp, rx_buf + 3, 4);
    // memcpy(rx_data, rx_buf + 7, 4);
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
    uint32_t timestamp;

    req = DAP_TRANS_DP | DAP_TRANS_READ_REG | DAP_TRANS_REG_ADDR(addr);

    for (uint8_t i = 0; i < 10; i++)
    {
        err = dap_swd_transfer(req, 0, &resp, &timestamp, val);
        if (err < 0)
        {
            return -1;
        }

        if ((resp != DAP_TRANSFER_WAIT))
            return 0;
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

    return dap_swd_read_dp(0x00, idcode);
}
