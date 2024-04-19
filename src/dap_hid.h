#ifndef DAP_HID_H
#define DAP_HID_H

#include "hidapi.h"
#include "hidapi_winapi.h"

#include <QList>
#include <QThread>
#include <QDebug>

#define DAP_HID_VID 0x0D28
#define DAP_HID_PID 0x0204

/*******************************************************************************
 * @brief Information and Control commands for the CMSIS-DAP Debug Unit.
 ******************************************************************************/
typedef enum
{
    DAP_CMD_INFO = 0,            // DAP_Info       : Get Information about CMSIS-DAP Debug Unit.
    DAP_CMD_HOST_STATUS = 0x01,  // DAP_HostStatus : Sent status information of the debugger to Debug Unit.
    DAP_CMD_CONNECT = 0x02,      // DAP_Connect    : Connect to Device and selected DAP mode.
    DAP_CMD_DISCONNECT = 0x03,   // DAP_Disconnect : Disconnect from active Debug Port.
    DAP_CMD_WRITE_ABORT = 0x08,  // DAP_WriteABORT : Write ABORT Register.
    DAP_CMD_DELAY = 0x09,        // DAP_Delay      : Wait for specified delay.
    DAP_CMD_RESET_TARGET = 0x0A, // DAP_ResetTarget: Reset Target with Device specific sequence.
} dap_gen_cmd_t;

/*******************************************************************************
 * @brief The DAP_Info Command provides configuration information about the Debug Unit itself and the capabilities.
 ******************************************************************************/
typedef enum
{
    DAP_GEN_INFO_CMD_VENDER_NAME = 1,            // 0x01 = Get the Vendor Name (string).
    DAP_GEN_INFO_CMD_PRODUCT_NAME,               // 0x02 = Get the Product Name (string).
    DAP_GEN_INFO_CMD_SERIAL_NNBER,               // 0x03 = Get the Serial Number (string).
    DAP_GEN_INFO_CMD_PROTOCOL_VER,               // 0x04 = Get the CMSIS-DAP Protocol Version (string).
    DAP_GEN_INFO_CMD_TARGET_DEV_VENDOR,          // 0x05 = Get the Target Device Vendor (string).
    DAP_GEN_INFO_CMD_TARGET_DEV_NAME,            // 0x06 = Get the Target Device Name (string).
    DAP_GEN_INFO_CMD_TARGET_BOARD_VENDOR,        // 0x07 = Get the Target Board Vendor (string).
    DAP_GEN_INFO_CMD_TARGET_BOARD_NAME,          // 0x08 = Get the Target Board Name (string).
    DAP_GEN_INFO_CMD_PRODUCT_FIRMWARE_VER,       // 0x09 = Get the Product Firmware Version (string, vendor-specific format).
    DAP_GEN_INFO_CMD_CAPS = 0xF0,                // 0xF0 = Get information about the Capabilities (BYTE) of the Debug Unit (see below for details).
    DAP_GEN_INFO_CMD_FREQ = 0xF1,                // 0xF1 = Get the Test Domain Timer parameter information (see below for details).
    DAP_GEN_INFO_CMD_UART_R_BUFF_SIZE = 0xFB,    // 0xFB = Get the UART Receive Buffer Size (WORD).
    DAP_GEN_INFO_CMD_UART_T_BUFF_SIZE = 0xFC,    // 0xFC = Get the UART Transmit Buffer Size (WORD).
    DAP_GEN_INFO_CMD_SWO_TRACK_BUFF_SIZE = 0xFD, // 0xFD = Get the SWO Trace Buffer Size (WORD).
    DAP_GEN_INFO_CMD_MAX_PKG_COUNT = 0xFE,       // 0xFE = Get the maximum Packet Count (BYTE).
    DAP_GEN_INFO_CMD_MAX_PKG_SIZE = 0xFF,        // 0xFF = Get the maximum Packet Size (SHORT).
} dap_gen_info_cmd_t;

typedef enum
{
    DAP_CONNECT_DEFAULT = 0, // 0 = Default mode: configuration of the DAP port mode is derived from DAP_DEFAULT_PORT (zero configuration).
    DAP_CONNECT_SWD = 1,     // 1 = SWD mode: connect with Serial Wire Debug mode.
    DAP_CONNECT_JTAG = 2,    // 2 = JTAG mode: connect with 4/5-pin JTAG mode.
} dap_connect_port_type_t;

#define DAP_OK 0x00    // 0x00 = DAP_OK: Command has been successfully executed
#define DAP_ERROR 0xFF // 0xFF = DAP_ERROR: Command did not execute due to communication failure with the device.

#define DAP_TRANS_DP 0x00
#define DAP_TRANS_AP 0x01
#define DAP_TRANS_READ_REG (1 << 1)
#define DAP_TRANS_WRITE_REG (0 << 1)
#define DAP_TRANS_REG_ADDR(a) (a & 0x0c)

// DAP Transfer Response
#define DAP_TRANSFER_OK (1U << 0)
#define DAP_TRANSFER_WAIT (1U << 1)
#define DAP_TRANSFER_FAULT (1U << 2)
#define DAP_TRANSFER_ERROR (1U << 3)
#define DAP_TRANSFER_MISMATCH (1U << 4)

const char *dap_state_to_string(uint8_t state);

/*******************************************************************************
 * @brief HID设备
 ******************************************************************************/
class DAP_HID : public QObject
{
    Q_OBJECT

public:
    DAP_HID(QString usb_path, QWidget *parent = nullptr);
    ~DAP_HID();
    static int32_t enum_device(void);

    int32_t open_device();
    int32_t close_device();
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
    int32_t dap_swd_transfer(uint8_t req, uint32_t tx_data, uint8_t *resp, uint32_t *timestamp, uint32_t *rx_data);
    int32_t dap_swd_transfer_block(uint8_t req, uint32_t tx_data, uint8_t *resp, uint32_t *timestamp, uint32_t *rx_data);

    int32_t dap_swd_reset();
    int32_t dap_swd_switch(uint16_t val);

    int32_t dap_swd_read_dp(uint8_t addr, uint32_t *_VA_LIST);
    int32_t dap_swd_write_dp();
    int32_t dap_swd_read_ap();
    int32_t dap_swd_write_ap();

    int32_t dap_swd_read_idcode(uint32_t *idcode);

private:
    hid_device *dev;
    QString usb_path; //
};

#endif // DAP_HID_H
