#include "devices.h"

Devices::Devices()
{
}

Devices::~Devices()
{
}

QString Devices::device_type_to_string(device_type_t t)
{
    switch (t)
    {
    case DAP_USB_HID:
        return "DAP_USB_HID";
    case DAP_USB_Bulk:
        return "DAP_USB_Bulk";
    }

    return QString::asprintf("UnknownDeviceType:%d", t);
}

Devices::device_type_t Devices::string_to_device_type(QString str)
{
    if (str == "DAP_USB_HID")
        return DAP_USB_HID;

    if (str == "DAP_USB_Bulk")
        return DAP_USB_Bulk;

    return UnknownDeviceType;
}
