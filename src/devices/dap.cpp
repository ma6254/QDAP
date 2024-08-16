#include "dap.h"

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
