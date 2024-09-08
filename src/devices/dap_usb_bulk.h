#ifndef DAP_USB_BULK_H
#define DAP_USB_BULK_H

#include <QWidget>
#include "libusb.h"
#include "devices.h"

/*******************************************************************************
 * @brief DAP_USB_Bulk CMSIS-DAP v2 uses USB bulk endpoints
 ******************************************************************************/
class CMSIS_DAP_V2 : public CMSIS_DAP_Base
{
    Q_OBJECT

public:
    CMSIS_DAP_V2(libusb_device *dev, int interface_number, uint8_t ep_in_addr, uint8_t ep_out_addr);
    ~CMSIS_DAP_V2();

    static int32_t enum_device(DeviceList *dev_list);
    static bool device_list_compare(QList<CMSIS_DAP_V2 *> *now_list, QList<CMSIS_DAP_V2 *> *prev_list);

    bool equal(const Devices &device) override;

    int32_t open_device();
    void close_device();

    int32_t error() { return err_code; }

    DeviceType type() const override { return DAP_USB_Bulk; }
    QString get_manufacturer_string() override { return manufacturer_str; }
    QString get_product_string() override { return product_str; }
    QString get_serial_string() override { return serial_number_str; }
    QString get_version_string() override { return version_str; }
    QString get_interface_string() { return interface_str; }
    // QString get_version_string() override { return hid_version; }

    int32_t dap_request(uint8_t *tx_data, uint8_t *rx_data) override;

private:
    libusb_context *context = NULL;
    libusb_device *dev = NULL;
    libusb_device_handle *handle = NULL;
    libusb_device_descriptor desc = {0};
    int interface_number;
    uint8_t ep_in_addr;
    uint8_t ep_out_addr;

    QString product_str;
    QString manufacturer_str;
    QString serial_number_str;
    QString version_str;
    QString interface_str;

    int32_t err_code;
};

#endif // DAP_USB_BULK_H
