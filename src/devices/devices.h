#ifndef DEVICES_H
#define DEVICES_H

#include <QList>
#include <QThread>
#include <QDebug>

/*******************************************************************************
 * @brief 设备
 ******************************************************************************/
class Devices : public QObject
{
    Q_OBJECT

public:
    Devices();
    ~Devices();

    typedef enum
    {
        UnknownDeviceType = 0,
        DAP_USB_HID,
        DAP_USB_Bulk,
        DAP_USB_JLink,
        DAP_ETH_JLink,
        DAP_CH347,
        DAP_FT2232,
    } device_type_t;

    static QString device_type_to_string(device_type_t t);
    static device_type_t string_to_device_type(QString str);

    // typedef enum
    // {
    //     JTAG,
    //     SWD,
    // } transport_t;

    // virtual bool support_transports(transport_t t) = 0;
    // bool support_transport_jtag() { return support_transports(JTAG); }
    // bool support_transport_swd() { return support_transports(SWD); }

    virtual device_type_t type() { return UnknownDeviceType; }
    QString type_str() { return device_type_to_string(type()); }

    virtual QString get_manufacturer_string() { return "Unknow"; }
    virtual QString get_product_string() { return "Unknow"; }

    // virtual QString get_manufacturer_string() = 0;
    // virtual QString get_product_string() = 0;
};

#include "dap.h"

#endif // DEVICES_H
