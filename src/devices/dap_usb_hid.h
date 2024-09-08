#ifndef DAP_USB_HID_H
#define DAP_USB_HID_H

#include "hidapi.h"
#include "hidapi_winapi.h"
#include "dap.h"
#include "flash_algo.h"
#include "devices.h"
#include "device_list.h"

#include <QList>
#include <QThread>
#include <QDebug>

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

    int32_t open_device();
    int32_t close_device();

    int32_t connect();
    int32_t run();

    QString dap_hid_get_manufacturer_string();
    QString dap_hid_get_product_string();
    int32_t dap_hid_get_info();

    uint32_t idcode() { return tmp_idcode; }

    int32_t dap_request(uint8_t *tx_data, uint8_t *rx_data) override;

    int32_t dap_hid_request(uint8_t *tx_data, uint8_t *rx_data);
    int32_t dap_hid_resp_status_return(uint8_t *rx_data);

    QString dap_get_info_vendor_name();
    QString dap_get_info_product_name();
    QString dap_get_info_serial_number();
    QString dap_get_info_cmsis_dap_protocol_version();
    int32_t dap_get_info_caps();
    int32_t dap_get_info_freq();

    int32_t dap_connect(uint8_t port);
    int32_t dap_disconnect();
    int32_t dap_reset_target();

    int32_t dap_swj_sequence(uint8_t bit_count, uint8_t *data);
    int32_t dap_swd_config(uint8_t cfg);
    int32_t dap_swd_sequence_write(uint8_t count, uint8_t *tx_data);

    int32_t dap_transfer_config(uint8_t idle_cyless, uint16_t wait_retry, uint16_t match_retry);
    int32_t dap_swd_transfer(uint8_t req, uint32_t tx_data, uint8_t *resp, uint32_t *rx_data);
    int32_t dap_swd_transfer_retry(uint8_t req, uint32_t tx_data, uint8_t *resp, uint32_t *rx_data);
    int32_t dap_swd_transfer_block(uint8_t req, uint32_t tx_data, uint8_t *resp, uint32_t *timestamp, uint32_t *rx_data);

    int32_t dap_swd_reset();
    int32_t dap_swd_switch(uint16_t val);

    int32_t dap_swd_read_dp(uint8_t addr, uint32_t *val);
    int32_t dap_swd_write_dp(uint8_t addr, uint32_t val);
    int32_t dap_swd_read_ap(uint8_t addr, uint32_t *val);
    int32_t dap_swd_write_ap(uint8_t addr, uint32_t val);

    int32_t dap_read_data(uint32_t addr, uint32_t *data);
    int32_t dap_write_data(uint32_t addr, uint32_t data);
    int32_t dap_read_word(uint32_t addr, uint32_t *val);
    int32_t dap_write_word(uint32_t addr, uint32_t val);

    int32_t dap_swd_read_idcode(uint32_t *idcode);
    int32_t dap_jtag_2_swd();
    int32_t swd_init_debug();
    int32_t dap_set_target_reset(uint8_t asserted);
    int32_t dap_set_target_state_hw(dap_target_reset_state_t state);

    int32_t dap_read_byte(uint32_t addr, uint8_t *val);
    int32_t dap_write_byte(uint32_t addr, uint8_t val);

    int32_t dap_read_block(uint32_t addr, uint8_t *data, uint32_t size);
    int32_t dap_write_block(uint32_t addr, uint8_t *data, uint32_t size);

    int32_t dap_read_memory(uint32_t addr, uint8_t *data, uint32_t size);
    int32_t dap_write_memory(uint32_t addr, uint8_t *data, uint32_t size);

    int32_t swd_write_debug_state(debug_state_t *state);
    int32_t swd_wait_until_halted(void);
    int32_t swd_read_core_register(uint32_t n, uint32_t *val);
    int32_t swd_write_core_register(uint32_t n, uint32_t val);
    int32_t swd_flash_syscall_exec(const program_syscall_t *sysCallParam, uint32_t entry, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4);

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
