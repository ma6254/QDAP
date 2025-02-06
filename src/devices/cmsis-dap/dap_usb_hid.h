#ifndef DAP_USB_HID_H
#define DAP_USB_HID_H

#ifdef _WIN32
#include <hidapi.h>
#include <hidapi_winapi.h>
#else
#include <hidapi/hidapi.h>
#include <hidapi/hidapi_libusb.h>
#endif // _WIN32
#include "dap.h"
#include "flash_algo.h"
#include "devices.h"
#include "device_list.h"

#include <QList>
#include <QThread>
#include <QDebug>

#define DAP_HID_VID 0x0D28
#define DAP_HID_PID 0x0204

/*******************************************************************************
 * @brief HID设备
 ******************************************************************************/
class DAP_HID : public CMSIS_DAP_Base
{
    Q_OBJECT

public:
    DAP_HID(QString usb_path);
    DAP_HID(Devices *devices);
    ~DAP_HID();

    static int32_t enum_device_id(DeviceList *dev_list, uint16_t vid = DAP_HID_VID, uint16_t pid = DAP_HID_PID);
    static int32_t enum_device(DeviceList *dev_list);

    bool equal(const Devices &device) override;

    DeviceType type() const override
    {
        return DAP_USB_HID;
    }
    QString get_manufacturer_string() override { return hid_manufacturer; }
    QString get_product_string() override { return hid_product; }
    QString get_serial_string() override { return hid_serial; }
    QString get_version_string() override { return hid_version; }

    int32_t error() { return err_code; }
    QString get_usb_path() { return usb_path; }

    int32_t open_device() override;
    int32_t close_device() override;

    // int32_t connect();
    int32_t run();

    QString dap_hid_get_manufacturer_string();
    QString dap_hid_get_product_string();
    int32_t dap_hid_get_info();

    uint32_t idcode() { return tmp_idcode; }

    int32_t dap_request(uint8_t *tx_data, uint32_t tx_len, uint8_t *rx_data, uint32_t rx_buf_size, uint32_t *rx_len) override;

    int32_t dap_hid_request(uint8_t *tx_data, uint8_t *rx_data);
    int32_t dap_hid_resp_status_return(uint8_t *rx_data);

    QString dap_get_info_vendor_name();
    QString dap_get_info_product_name();
    QString dap_get_info_serial_number();
    QString dap_get_info_cmsis_dap_protocol_version();
    int32_t dap_get_info_caps();
    int32_t dap_get_info_freq();

private:
    hid_device *dev;
    QString usb_path; //
    dap_state_t dap_state;

    QString hid_manufacturer;
    QString hid_product;
    QString hid_serial;
    QString hid_version;

    uint32_t tmp_idcode;
    int32_t err_code;
};

#endif // DAP_USB_HID_H
