#include "dap.h"
#include "utils.h"

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

CMSIS_DAP_Base::CMSIS_DAP_Base()
{
}

CMSIS_DAP_Base::~CMSIS_DAP_Base()
{
}

int32_t CMSIS_DAP_Base::connect()
{
    int32_t err;

    qDebug("[DAP_Base] connect open");
    qDebug("[DAP_Base]         Verion: %s", qPrintable(get_version_string()));
    qDebug("[DAP_Base]       iProduct: %s", qPrintable(get_product_string()));
    qDebug("[DAP_Base]  iManufacturer: %s", qPrintable(get_manufacturer_string()));
    qDebug("[DAP_Base]  iSerialNumber: %s", qPrintable(get_serial_string()));

    err = open_device();
    if (err < 0)
    {
        qDebug("[DAP_Base] connect open fail");
        return err;
    }

    err = dap_connect(1);
    if (err < 0)
    {
        qDebug("[DAP_Base] connect dap_connect fail");
        close_device();
        return err;
    }

    err = dap_reset_target();
    if (err < 0)
    {
        qDebug("[DAP_Base] connect dap_reset_target fail");
        close_device();
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
        qDebug("[DAP_Base] connect dap_swd_reset fail");
        close_device();
        return err;
    }

    // JTAG-to-SWD
    err = dap_swd_switch(0xE79E);
    if (err < 0)
    {
        qDebug("[DAP_Base] connect dap_swd_switch fail");
        close_device();
        return err;
    }

    err = dap_swd_reset();
    if (err < 0)
    {
        qDebug("[DAP_Base] connect dap_swd_reset fail");
        close_device();
        return err;
    }

    err = dap_swd_read_idcode(&tmp_idcode);
    if (err < 0)
    {
        qDebug("[DAP_Base] connect dap_swd_read_idcode fail idcode:0x%08X", tmp_idcode);
    }
    else
    {
        qDebug("[DAP_Base] connect dap_swd_read_idcode 0x%08X", tmp_idcode);
    }

    // err = swd_init_debug();
    // if (err < 0)
    // {
    //     qDebug("[DAP_HID] connect swd_init_debug fail");
    //     return err;
    // }

    // qDebug("[DAP_HID] connect swd_init_debug ok");

    // err = dap_set_target_state_hw(DAP_TARGET_RESET_PROGRAM);
    // if (err < 0)
    // {
    //     qDebug("[DAP_HID] connect dap_set_target_state_hw RESET_PROGRAM fail");
    //     return err;
    // }
    // qDebug("[DAP_HID] connect dap_set_target_state_hw RESET_PROGRAM ok");

    // dap_disconnect();
    // close_device();

    close_device();
    return 0;
}

int CMSIS_DAP_Base::parse_port_str(QString str, Port *port)
{
    str = str.toUpper();

    if (str == "SWD")
    {
        *port = SWD;
        return 0;
    }
    else if (str == "JTAG")
    {
        *port = JTAG;
        return 0;
    }

    return -1;
}

int CMSIS_DAP_Base::get_info_cmsis_dap_protocol_version(QString *version)
{
    uint8_t tx_buf[512] = {0};
    uint8_t rx_buf[512] = {0};
    int32_t err;
    uint32_t rx_len;

    // qDebug("[dap_hid] dap_get_info_cmsis_dap_protocol_version");

    tx_buf[0] = 0x00;
    tx_buf[1] = 0x04;
    err = dap_request(tx_buf, 2, rx_buf, sizeof(rx_buf), &rx_len);
    if (err < 0)
    {
        return -1;
    }

    // hexdump(rx_buf, rx_len);

    uint8_t len = rx_buf[1];

    if (len == 0)
    {
        return -1;
    }

    if (len > 62)
        len = 62;

    *version = QString(QLatin1String((char *)rx_buf + 2, len));
    // qDebug("[CMSIS_DAP_V2] get_info_cmsis_dap_protocol_version: %s", qPrintable(*version));
    // hexdump(rx_buf, 64);
    return 0;
}

/*******************************************************************************
 * @brief Connect to Device and selected DAP mode.
 * @ref https://arm-software.github.io/CMSIS_5/DAP/html/group__DAP__Connect.html
 * @param None
 * @return None
 ******************************************************************************/
int32_t CMSIS_DAP_Base::dap_connect(uint8_t port)
{
    uint8_t tx_buf[512] = {0};
    uint8_t rx_buf[512] = {0};
    uint8_t rx_port;
    int32_t err;
    uint32_t rx_len;

    // qDebug("[dap_hid] dap_connect");

    tx_buf[0] = DAP_CMD_CONNECT;
    tx_buf[1] = port;
    err = dap_request(tx_buf, 2, rx_buf, sizeof(rx_buf), &rx_len);
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
int32_t CMSIS_DAP_Base::dap_disconnect()
{
    uint8_t tx_buf[512] = {0};
    uint8_t rx_buf[512] = {0};
    int32_t err;
    uint32_t rx_len;

    // qDebug("[dap_hid] dap_disconnect");

    tx_buf[0] = DAP_CMD_DISCONNECT;
    err = dap_request(tx_buf, 1, rx_buf, sizeof(rx_buf), &rx_len);
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
int32_t CMSIS_DAP_Base::dap_reset_target()
{
    uint8_t tx_buf[512] = {0};
    uint8_t rx_buf[512] = {0};
    int32_t err;
    uint32_t rx_len;

    // qDebug("[dap_hid] dap_reset_target");

    // 报告编号
    tx_buf[0] = DAP_CMD_RESET_TARGET;
    err = dap_request(tx_buf, 1, rx_buf, sizeof(rx_buf), &rx_len);
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
int32_t CMSIS_DAP_Base::dap_swj_sequence(uint8_t bit_count, uint8_t *data)
{
    uint8_t tx_buf[512] = {0};
    uint8_t rx_buf[512] = {0};
    int32_t err;
    uint8_t count_byte;
    uint32_t rx_len;

    // qDebug("[dap_hid] dap_swj_sequence");

    tx_buf[0] = 0x12;
    tx_buf[1] = bit_count; // Sequence Count
    // tx_buf[3] = tx_data; // SWDIO Data

    count_byte = bit_count / 8;
    if (bit_count % 8)
        count_byte += 1;
    memcpy(tx_buf + 2, data, count_byte);

    err = dap_request(tx_buf, count_byte + 2, rx_buf, sizeof(rx_buf), &rx_len);
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

int32_t CMSIS_DAP_Base::dap_swd_config(uint8_t cfg)
{
    uint8_t tx_buf[512] = {0};
    uint8_t rx_buf[512] = {0};
    int32_t err;
    uint32_t rx_len;

    // qDebug("[dap_hid] dap_swd_config");

    tx_buf[0] = 0x13;
    tx_buf[1] = cfg; // Configuration
    err = dap_request(tx_buf, 1, rx_buf, sizeof(rx_buf), &rx_len);
    if (err < 0)
        return err;

    uint8_t status = rx_buf[1];
    if (status != DAP_OK)
        return -1;

    return 0;
}

int32_t CMSIS_DAP_Base::dap_swd_sequence_write(uint8_t count, uint8_t *tx_data)
{
    uint8_t tx_buf[512] = {0};
    uint8_t rx_buf[512] = {0};
    int32_t err;
    uint32_t rx_len;
    uint8_t count_byte;

    // qDebug("[dap_hid] dap_swd_sequence_write");

    tx_buf[0] = 0x1D;
    tx_buf[1] = count; // Sequence Count
    tx_buf[2] = 0;     // Sequence Info
    // tx_buf[4] = tx_data; // SWDIO Data

    count_byte = count / 8;
    if (count % 8)
        count += 1;

    memcpy(tx_buf + 4, tx_data, count_byte);

    err = dap_request(tx_buf, count_byte + 3, rx_buf, sizeof(rx_buf), &rx_len);
    if (err < 0)
        return -1;

    uint8_t status = rx_buf[1];
    // uint8_t rx_data = rx_buf[2];

    if (status == DAP_OK)
        return 0;
    else
        return -1;
}

int32_t CMSIS_DAP_Base::dap_transfer_config(uint8_t idle_cyless, uint16_t wait_retry, uint16_t match_retry)
{
    uint8_t tx_buf[512] = {0};
    uint8_t rx_buf[512] = {0};
    int32_t err;
    uint32_t rx_len;

    // qDebug("[dap_hid] dap_transfer_config");

    tx_buf[0] = 0x04;
    tx_buf[1] = idle_cyless;
    tx_buf[2] = wait_retry >> 8;
    tx_buf[3] = wait_retry & 0xFF;
    tx_buf[4] = match_retry >> 8;
    tx_buf[5] = match_retry & 0xFF;

    err = dap_request(tx_buf, 6, rx_buf, sizeof(rx_buf), &rx_len);
    if (err < 0)
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
int32_t CMSIS_DAP_Base::dap_swd_transfer(uint8_t req, uint32_t tx_data, uint8_t *resp, uint32_t *rx_data)
{
    uint8_t tx_buf[512] = {0};
    uint8_t rx_buf[512] = {0};
    int32_t err;
    uint32_t rx_len;

    // qDebug("[dap_hid] dap_swd_transfer");

    tx_buf[0] = 0x05;                // cmd
    tx_buf[1] = 0;                   // DAP_Index
    tx_buf[2] = 1;                   // Transfer Count
    tx_buf[3] = req;                 // Transfer Request
    memcpy(tx_buf + 4, &tx_data, 4); // Transfer Data

    err = dap_request(tx_buf, 8, rx_buf, sizeof(rx_buf), &rx_len);
    if (err < 0)
        return err;

    *(resp) = rx_buf[2];

    if (rx_data)
        memcpy(rx_data, rx_buf + 3, 4);

    return 0;
}

int32_t CMSIS_DAP_Base::dap_swd_transfer_retry(uint8_t req, uint32_t tx_data, uint8_t *resp, uint32_t *rx_data)
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

int32_t CMSIS_DAP_Base::dap_swd_transfer_block(uint8_t req, uint32_t tx_data, uint8_t *resp, uint32_t *timestamp, uint32_t *rx_data)
{
    uint8_t tx_buf[512] = {0};
    uint8_t rx_buf[512] = {0};
    int32_t err;
    uint32_t rx_len;

    tx_buf[0] = 0x06;
    tx_buf[1] = 0;                   // DAP_Index
    tx_buf[2] = 1;                   // Transfer Count
    tx_buf[3] = req;                 // Transfer Request
    memcpy(tx_buf + 4, &tx_data, 4); // Transfer Data

    err = dap_request(tx_buf, 8, rx_buf, sizeof(rx_buf), &rx_len);
    if (err < 0)
        return err;

    qDebug("[DAP_Base] dap_swd_transfer_block resp:");
    hexdump(rx_buf, 64);

    *(resp) = rx_buf[2];
    memcpy(timestamp, rx_buf + 3, 4);
    memcpy(rx_data, rx_buf + 7, 4);

    return 0;
}

int32_t CMSIS_DAP_Base::dap_swd_reset()
{
    uint8_t tmp_in[8];
    uint8_t i = 0;

    for (i = 0; i < 8; i++)
    {
        tmp_in[i] = 0xff;
    }

    return dap_swj_sequence(51, tmp_in);
}

int32_t CMSIS_DAP_Base::dap_swd_switch(uint16_t val)
{

    uint8_t tmp_in[2];
    tmp_in[0] = val & 0xff;
    tmp_in[1] = (val >> 8) & 0xff;

    return dap_swj_sequence(16, tmp_in);
}

int32_t CMSIS_DAP_Base::dap_swd_read_dp(uint8_t addr, uint32_t *val)
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

int32_t CMSIS_DAP_Base::dap_swd_read_idcode(uint32_t *idcode)
{
    int32_t err;

    uint8_t tmp_in[1];
    tmp_in[0] = 0x00;

    err = dap_swj_sequence(8, tmp_in);
    if (err < 0)
        return err;

    return dap_swd_read_dp(DP_IDCODE, idcode);
}

int32_t CMSIS_DAP_Base::dap_hid_resp_status_return(uint8_t *rx_data)
{
    uint8_t status = rx_data[1];

    if (status == DAP_OK)
        return 1;
    else if (status == DAP_ERROR)
        return 0;
    else
        return -1;
}
