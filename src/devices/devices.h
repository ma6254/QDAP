#ifndef DEVICES_H
#define DEVICES_H

#include <QList>
#include <QThread>
#include <QDebug>

class Devices;
class DeviceList;
class DAP_HID;

/*******************************************************************************
 * @brief 设备
 ******************************************************************************/
class Devices : public QObject
{
    Q_OBJECT

public:
    enum DeviceType
    {
        UnknownDeviceType = 0,
        DAP_USB_HID,
        DAP_USB_Bulk,
        DAP_USB_JLink,
        DAP_ETH_JLink,
        DAP_CH347,
        DAP_FT2232,
    };
    Q_ENUM(DeviceType)

    enum ClockUnit
    {
        Hz,
        KHz,
        MHz,
        GHz,
    };
    Q_ENUM(ClockUnit)

    Devices();
    Devices(const Devices &src_device);
    ~Devices();

    virtual int32_t connect() = 0;

    static QString device_type_to_string(DeviceType t);
    static DeviceType string_to_device_type(QString str);
    static int parse_clock_str(QString str, uint64_t *clock, ClockUnit *unit);
    static QString get_clock_unit_str(ClockUnit unit);

    // typedef enum
    // {
    //     JTAG,
    //     SWD,
    // } transport_t;

    // virtual bool support_transports(transport_t t) = 0;
    // bool support_transport_jtag() { return support_transports(JTAG); }
    // bool support_transport_swd() { return support_transports(SWD); }

    virtual DeviceType type() const
    {
        return UnknownDeviceType;
    }
    QString type_str()
    {
        return device_type_to_string(type());
    }

    virtual QString get_manufacturer_string()
    {
        return "Unknow";
    }
    virtual QString get_product_string()
    {
        return "Unknow";
    }

    virtual bool equal(const Devices &device);

    static bool device_list_compare(DeviceList now_list,
                                    DeviceList prev_list,
                                    DeviceList *added_list = NULL,
                                    DeviceList *fixed_list = NULL,
                                    DeviceList *removed_list = NULL);

    // virtual QString get_manufacturer_string() = 0;
    // virtual QString get_product_string() = 0;
};

#include "device_list.h"
#include "dap.h"

#endif // DEVICES_H
