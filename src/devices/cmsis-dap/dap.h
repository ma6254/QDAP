#ifndef DAP_H
#define DAP_H

#include <stdint.h>
#include <QString>
#include <QWidget>
#include "devices.h"
#include "dap.h"

typedef struct
{
    uint32_t select;
    uint32_t csw;
} dap_state_t;

typedef struct
{
    uint32_t r[16];
    uint32_t xpsr;
} debug_state_t;

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

#define DCRDR 0xE000EDF8
#define DCRSR 0xE000EDF4
#define DHCSR 0xE000EDF0
#define REGWnR (1 << 16)

// DAP Transfer Response
#define DAP_TRANSFER_OK (1U << 0)
#define DAP_TRANSFER_WAIT (1U << 1)
#define DAP_TRANSFER_FAULT (1U << 2)
#define DAP_TRANSFER_ERROR (1U << 3)
#define DAP_TRANSFER_MISMATCH (1U << 4)

// Debug Port Register Addresses
#define DP_IDCODE 0x00U    // IDCODE Register (SW Read only)
#define DP_ABORT 0x00U     // Abort Register (SW Write only)
#define DP_CTRL_STAT 0x04U // Control & Status
#define DP_WCR 0x04U       // Wire Control Register (SW Only)
#define DP_SELECT 0x08U    // Select Register (JTAG R/W & SW W)
#define DP_RESEND 0x08U    // Resend (SW Read Only)
#define DP_RDBUFF 0x0CU    // Read Buffer (Read Only)

// AP CSW register, base value
#define CSW_VALUE (CSW_RESERVED | CSW_MSTRDBG | CSW_HPROT | CSW_DBGSTAT | CSW_SADDRINC)

#define NVIC_Addr (0xe000e000)
#define DBG_Addr (0xe000edf0)

typedef enum
{
    DAP_TARGET_RESET_HOLD,    // Hold target in reset
    DAP_TARGET_RESET_PROGRAM, // Reset target and setup for flash programming.
    DAP_TARGET_RESET_RUN,     // Reset target and run normally
    DAP_TARGET_NO_DEBUG,      // Disable debug on running target
    DAP_TARGET_DEBUG,         // Enable debug on running target
    DAP_TARGET_HALT,          // Halt the target without resetting it
    DAP_TARGET_RUN            // Resume the target without resetting it
} dap_target_reset_state_t;

const char *dap_state_to_string(uint8_t state);

/*******************************************************************************
 * @brief HID设备
 ******************************************************************************/
class CMSIS_DAP_Base : public Devices
{
    Q_OBJECT

public:
    enum Port
    {
        SWD = 1,
        JTAG = 2,
    };
    Q_ENUM(Port)

    CMSIS_DAP_Base();
    ~CMSIS_DAP_Base();

    int32_t connect() override;
    int32_t run() override;
    int chip_read_memory(uint32_t addr, uint8_t *data, uint32_t size) override;
    int chip_write_memory(uint32_t addr, uint8_t *data, uint32_t size) override;
    int32_t chip_syscall_exec(const program_syscall_t *sysCallParam, uint32_t entry, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4) override;

    static int parse_port_str(QString str, Port *port);

    virtual QString get_manufacturer_string() { return "Unknow"; }
    virtual QString get_product_string() { return "Unknow"; }
    virtual QString get_serial_string() { return "Unknow"; }
    virtual QString get_version_string() { return "Unknow"; }

    virtual int32_t open_device() = 0;
    virtual int32_t close_device() = 0;

    virtual int32_t dap_request(uint8_t *tx_data, uint32_t tx_len, uint8_t *rx_data, uint32_t rx_buf_size, uint32_t *rx_len) = 0;

    int get_info_cmsis_dap_protocol_version(QString *version);

    int32_t dap_connect(uint8_t port);
    int32_t dap_disconnect();
    int32_t dap_reset_target();
    int32_t dap_set_swj_clock(uint32_t clock);
    int32_t dap_set_target_state_hw(dap_target_reset_state_t state);
    int32_t dap_set_target_reset(uint8_t asserted);

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

    int32_t dap_read_byte(uint32_t addr, uint8_t *val);
    int32_t dap_write_byte(uint32_t addr, uint8_t val);

    int32_t dap_read_block(uint32_t addr, uint8_t *data, uint32_t size);
    int32_t dap_write_block(uint32_t addr, uint8_t *data, uint32_t size);

    int32_t dap_swd_read_idcode(uint32_t *idcode);
    int32_t dap_jtag_2_swd();
    int32_t swd_init_debug();

    int32_t swd_write_debug_state(debug_state_t *state);
    int32_t swd_wait_until_halted(void);
    int32_t swd_read_core_register(uint32_t n, uint32_t *val);
    int32_t swd_write_core_register(uint32_t n, uint32_t val);

private:
    int32_t dap_hid_resp_status_return(uint8_t *rx_data);

    dap_state_t dap_state;
};

#include "dap_usb_hid.h"
#include "dap_usb_bulk.h"

#endif // DAP_H
