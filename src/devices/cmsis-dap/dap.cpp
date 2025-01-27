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

    // qDebug("[dap_hid] dap_get_info_cmsis_dap_protocol_version");

    tx_buf[0] = 0x00;
    tx_buf[1] = 0x04;
    err = dap_request(tx_buf, rx_buf);
    if (err < 0)
    {
        return -1;
    }

    hexdump(rx_buf, 64);

    uint8_t len = rx_buf[1];

    if (len == 0)
    {
        return -1;
    }

    if (len > 62)
        len = 62;

    *version = QString(QLatin1String((char *)rx_buf + 2, len));

    qDebug("[CMSIS_DAP_V2] get_info_cmsis_dap_protocol_version: %s", qPrintable(*version));

    // hexdump(rx_buf, 64);
    return 0;
}
