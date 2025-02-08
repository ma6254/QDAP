#include "dap.h"
#include "utils.h"
#include "debug_cm.h"

static uint32_t Flash_Page_Size = 4096;

#define SCS_BASE (0xE000E000UL)
#define SCB_BASE (SCS_BASE + 0x0D00UL)

#define SCB_AIRCR_VECTKEY_Pos 16U                                    /*!< SCB AIRCR: VECTKEY Position */
#define SCB_AIRCR_VECTKEY_Msk (0xFFFFUL << SCB_AIRCR_VECTKEY_Pos)    /*!< SCB AIRCR: VECTKEY Mask */
#define SCB_AIRCR_PRIGROUP_Pos 8U                                    /*!< SCB AIRCR: PRIGROUP Position */
#define SCB_AIRCR_PRIGROUP_Msk (7UL << SCB_AIRCR_PRIGROUP_Pos)       /*!< SCB AIRCR: PRIGROUP Mask */
#define SCB_AIRCR_SYSRESETREQ_Pos 2U                                 /*!< SCB AIRCR: SYSRESETREQ Position */
#define SCB_AIRCR_SYSRESETREQ_Msk (1UL << SCB_AIRCR_SYSRESETREQ_Pos) /*!< SCB AIRCR: SYSRESETREQ Mask */

#define SCB_AIRCR 0xE000ED0C

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
    memset(&dap_state, 0, sizeof(dap_state));
    dap_state.select = 0xFFFFFFFF;
    dap_state.csw = 0xFFFFFFFF;
}

CMSIS_DAP_Base::~CMSIS_DAP_Base()
{
}

int32_t CMSIS_DAP_Base::connect()
{
    int32_t err;

    tmp_idcode = 0;

    qDebug("[DAP_Base] connect open");
    qDebug("[DAP_Base]         Verion: %s", qPrintable(get_version_string()));
    qDebug("[DAP_Base]       iProduct: %s", qPrintable(get_product_string()));
    qDebug("[DAP_Base]  iManufacturer: %s", qPrintable(get_manufacturer_string()));
    qDebug("[DAP_Base]  iSerialNumber: %s", qPrintable(get_serial_string()));

    memset(&dap_state, 0, sizeof(dap_state));
    dap_state.select = 0xFFFFFFFF;
    dap_state.csw = 0xFFFFFFFF;

    err = open_device();
    if (err < 0)
    {
        qDebug("[DAP_Base] connect open fail");
        return err;
    }

    err = dap_connect(1);
    if (err < 0)
    {
        qDebug("[DAP_Base] connect dap_connect fail");
        close_device();
        return err;
    }

    err = dap_set_swj_clock(1000000);
    if (err < 0)
    {
        qDebug("[DAP_Base] connect dap_set_swj_clock fail");
        close_device();
        return err;
    }

    err = dap_reset_target();
    if (err < 0)
    {
        qDebug("[DAP_Base] connect dap_reset_target fail");
        close_device();
        return err;
    }

    // err = dap_swd_config(0x02);
    // if (err < 0)
    // {
    //     qDebug("[enum_device] dap_swd_config fail");

    //     // if (err == Devices::Error::DEVICE_ERR_DAP_REQUEST_TIMEOUT)
    //     // {
    //     // }
    //     // else
    //     //     return err;
    // }

    err = dap_transfer_config(0, 0, 0);
    if (err < 0)
    {
        qDebug("[enum_device] dap_swd_config fail");

        // if (err == Devices::Error::DEVICE_ERR_DAP_REQUEST_TIMEOUT)
        // {
        // }
        // else
        //     return err;
    }

    err = dap_swd_reset();
    if (err < 0)
    {
        qDebug("[DAP_Base] connect dap_swd_reset fail");
        close_device();
        return err;
    }

    // JTAG-to-SWD
    err = dap_swd_switch(0xE79E);
    if (err < 0)
    {
        qDebug("[DAP_Base] connect dap_swd_switch fail");
        close_device();
        return err;
    }

    memset(&dap_state, 0, sizeof(dap_state));
    dap_state.select = 0xFFFFFFFF;
    dap_state.csw = 0xFFFFFFFF;

    err = dap_swd_reset();
    if (err < 0)
    {
        qDebug("[DAP_Base] connect dap_swd_reset fail");
        close_device();
        return err;
    }

    err = dap_swd_read_idcode(&tmp_idcode);
    if (err < 0)
    {
        qDebug("[DAP_Base] connect dap_swd_read_idcode fail %d idcode:0x%08X", err, tmp_idcode);
        return err;
    }

    qDebug("[DAP_Base] connect dap_swd_read_idcode 0x%08X", tmp_idcode);

    if ((tmp_idcode == 0x00000000) || (tmp_idcode == 0xFFFFFFFF))
    {
        qDebug("[DAP_Base] read idcode fail");
        return -1;
    }

    err = swd_init_debug();
    if (err < 0)
    {
        qDebug("[DAP_Base] connect swd_init_debug fail");
        return err;
    }

    qDebug("[DAP_Base] connect swd_init_debug ok");

    err = dap_set_target_state_hw(DAP_TARGET_RESET_PROGRAM);
    if (err < 0)
    {
        qDebug("[CMSIS_DAP_Base] connect dap_set_target_state_hw RESET_PROGRAM fail");
        return err;
    }

    // err = dap_set_target_state_hw(DAP_TARGET_RESET_RUN);
    // if (err < 0)
    // {
    //     qDebug("[CMSIS_DAP_Base] run dap_set_target_state_hw DAP_TARGET_RESET_RUN fail");
    //     return err;
    // }

    qDebug("[CMSIS_DAP_Base] connect dap_set_target_state_hw RESET_PROGRAM ok");

    // dap_disconnect();
    // close_device();

    return 0;
}

int32_t CMSIS_DAP_Base::run()
{

    int32_t err;
    uint32_t idcode;

    err = open_device();
    if (err < 0)
    {
        qDebug("[CMSIS_DAP_Base] run open fail");
        return err;
    }

    err = dap_connect(1);
    if (err < 0)
    {
        qDebug("[CMSIS_DAP_Base] run dap_connect fail");
        return err;
    }

    // err = dap_set_target_state_hw(DAP_TARGET_RESET_RUN);
    err = dap_set_target_state_hw(DAP_TARGET_RESET_RUN);
    if (err < 0)
    {
        qDebug("[CMSIS_DAP_Base] run dap_set_target_state_hw DAP_TARGET_RESET_RUN fail");
        return err;
    }

    qDebug("[CMSIS_DAP_Base] run");

    dap_disconnect();
    close_device();
    return 0;
}

int CMSIS_DAP_Base::chip_read_memory(uint32_t addr, uint8_t *data, uint32_t size)
{
    int32_t err;
    uint32_t n;

    // qDebug("[CMSIS_DAP_Base] chip_read_memory");

    // Read bytes until word aligned
    while ((size > 0) && (addr & 0x3))
    {
        err = dap_read_byte(addr, data);
        if (err < 0)
        {
            return err;
        }

        addr++;
        data++;
        size--;
    }

    // Read word aligned blocks
    while (size > 3)
    {
        // Limit to auto increment page size
        n = Flash_Page_Size - (addr & (Flash_Page_Size - 1));

        if (size < n)
        {
            n = size & 0xFFFFFFFC; // Only count complete words remaining
        }

        err = dap_read_block(addr, data, n);
        if (err < 0)
        {
            return err;
        }

        addr += n;
        data += n;
        size -= n;
    }

    // Read remaining bytes
    while (size > 0)
    {
        err = dap_read_byte(addr, data);
        if (err < 0)
            return err;

        addr++;
        data++;
        size--;
    }

    // qDebug("[CMSIS_DAP_Base] chip_read_memory done");

    return 0;
}

int CMSIS_DAP_Base::chip_write_memory(uint32_t addr, uint8_t *data, uint32_t size)
{
    int32_t err;
    uint32_t n = 0;

    // Write bytes until word aligned
    while ((size > 0) && (addr & 0x3))
    {
        err = dap_write_byte(addr, *data);
        if (err < 0)
            return err;

        addr++;
        data++;
        size--;
    }

    // Write word aligned blocks
    while (size > 3)
    {
        // Limit to auto increment page size
        n = Flash_Page_Size - (addr & (Flash_Page_Size - 1));

        if (size < n)
        {
            n = size & 0xFFFFFFFC; // Only count complete words remaining
        }

        err = dap_write_block(addr, data, n);
        if (err < 0)
            return err;

        addr += n;
        data += n;
        size -= n;
    }

    // Write remaining bytes
    while (size > 0)
    {
        err = dap_write_byte(addr, *data);
        if (err < 0)
            return err;

        addr++;
        data++;
        size--;
    }

    return 0;
}

int32_t CMSIS_DAP_Base::chip_syscall_exec(const program_syscall_t *sysCallParam, uint32_t entry, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4)
{
    int32_t err;
    debug_state_t state = {{0}, 0};

    // Call flash algorithm function on target and wait for result.
    state.r[0] = arg1;                         // R0: Argument 1
    state.r[1] = arg2;                         // R1: Argument 2
    state.r[2] = arg3;                         // R2: Argument 3
    state.r[3] = arg4;                         // R3: Argument 4
    state.r[9] = sysCallParam->static_base;    // SB: Static Base
    state.r[13] = sysCallParam->stack_pointer; // SP: Stack Pointer
    state.r[14] = sysCallParam->breakpoint;    // LR: Exit Point
    state.r[15] = entry;                       // PC: Entry Point
    state.xpsr = 0x01000000;                   // xPSR: T = 1, ISR = 0

    err = swd_write_debug_state(&state);
    if (err < 0)
    {
        qDebug("[CMSIS_DAP_Base] swd_write_debug_state fail");
        return err;
    }

    err = swd_wait_until_halted();
    if (err < 0)
    {
        qDebug("[CMSIS_DAP_Base] wait_until_halted fail");
        return err;
    }

    err = swd_read_core_register(0, &state.r[0]);
    if (err < 0)
    {
        qDebug("[CMSIS_DAP_Base] swd_read_core_register fail");
        return err;
    }

    // Flash functions return 0 if successful.
    if (state.r[0] != 0)
    {
        qDebug("[CMSIS_DAP_Base] flash_func return:0x%X", state.r[0]);
        return -1;
    }

    return 0;
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
    uint8_t rx_buf[64] = {0};
    int32_t err;
    uint32_t rx_len;

    // qDebug("[dap_hid] dap_get_info_cmsis_dap_protocol_version");

    tx_buf[0] = 0x00;
    tx_buf[1] = 0x04;
    err = dap_request(tx_buf, 2, rx_buf, sizeof(rx_buf), &rx_len);
    if (err < 0)
    {
        return -1;
    }

    // hexdump(rx_buf, rx_len);

    uint8_t len = rx_buf[1];

    if (len == 0)
    {
        return -1;
    }

    if (len > 62)
        len = 62;

    *version = QString(QLatin1String((char *)rx_buf + 2, len));
    // qDebug("[CMSIS_DAP_V2] get_info_cmsis_dap_protocol_version: %s", qPrintable(*version));
    // hexdump(rx_buf, 64);
    return 0;
}

/*******************************************************************************
 * @brief Connect to Device and selected DAP mode.
 * @ref https://arm-software.github.io/CMSIS_5/DAP/html/group__DAP__Connect.html
 * @param None
 * @return None
 ******************************************************************************/
int32_t CMSIS_DAP_Base::dap_connect(uint8_t port)
{
    uint8_t tx_buf[512] = {0};
    uint8_t rx_buf[64] = {0};
    uint8_t rx_port;
    int32_t err;
    uint32_t rx_len;

    // qDebug("[dap_hid] dap_connect");

    tx_buf[0] = DAP_CMD_CONNECT;
    tx_buf[1] = port;
    err = dap_request(tx_buf, 2, rx_buf, sizeof(rx_buf), &rx_len);
    if (err < 0)
    {
        return err;
    }

    rx_port = rx_buf[1];

    if (rx_port == 0)
        return -1;

    if (rx_port != port)
        return -1;

    return 0;
}

/*******************************************************************************
 * @brief Get the CMSIS-DAP Protocol Version (string).
 * @ref https://arm-software.github.io/CMSIS_5/DAP/html/group__DAP__Disconnect.html
 * @param None
 * @return None
 ******************************************************************************/
int32_t CMSIS_DAP_Base::dap_disconnect()
{
    uint8_t tx_buf[512] = {0};
    uint8_t rx_buf[64] = {0};
    int32_t err;
    uint32_t rx_len;

    // qDebug("[dap_hid] dap_disconnect");

    tx_buf[0] = DAP_CMD_DISCONNECT;
    err = dap_request(tx_buf, 1, rx_buf, sizeof(rx_buf), &rx_len);
    if (err < 0)
        return err;

    return dap_hid_resp_status_return(rx_buf);
}

/*******************************************************************************
 * @brief The DAP_ResetTarget Command requests a target reset with a device
 *        specific command sequence. This command calls the user configurable
 *        function RESET_TARGET.
 * @ref https://arm-software.github.io/CMSIS_5/DAP/html/group__DAP__ResetTarget.html
 * @param None
 * @return None
 ******************************************************************************/
int32_t CMSIS_DAP_Base::dap_reset_target()
{
    qDebug("[CMSIS_DAP_Base] dap_reset_target");

    uint8_t tx_buf[512] = {0};
    uint8_t rx_buf[64] = {0};
    int32_t err;
    uint32_t rx_len;

    // qDebug("[dap_hid] dap_reset_target");

    // 报告编号
    tx_buf[0] = DAP_CMD_RESET_TARGET;
    err = dap_request(tx_buf, 1, rx_buf, sizeof(rx_buf), &rx_len);
    if (err < 0)
        return err;

    // qDebug("[CMSIS_DAP_Base] dap_reset_target resp:");
    // hexdump(rx_buf, 64);

    uint8_t status = rx_buf[1];
    // uint8_t execute = rx_buf[2];

    if (status == DAP_OK)
        return 1;
    else if (status == DAP_ERROR)
        return 0;
    else
        return -1;
}

int32_t CMSIS_DAP_Base::dap_set_swj_clock(uint32_t clock)
{
    qDebug("[CMSIS_DAP_Base] dap_set_swj_clock");

    uint8_t tx_buf[512] = {0};
    uint8_t rx_buf[64] = {0};
    int32_t err;
    uint32_t rx_len;

    // qDebug("[dap_hid] dap_reset_target");

    // 报告编号
    tx_buf[0] = 0x11;
    tx_buf[1] = clock;
    tx_buf[2] = clock >> 8;
    tx_buf[3] = clock >> 16;
    tx_buf[4] = clock >> 24;

    err = dap_request(tx_buf, 5, rx_buf, sizeof(rx_buf), &rx_len);
    if (err < 0)
        return err;

    // qDebug("[CMSIS_DAP_Base] dap_reset_target resp:");
    // hexdump(rx_buf, 64);

    uint8_t rx_cmd = rx_buf[0];
    uint8_t status = rx_buf[1];
    // uint8_t execute = rx_buf[2];

    if (rx_cmd != tx_buf[0])
        return -1;

    if (status == DAP_OK)
        return 1;
    else if (status == DAP_ERROR)
        return 0;
    else
        return -1;
}

int32_t CMSIS_DAP_Base::dap_set_target_reset(uint8_t asserted)
{
    int32_t err;
    uint32_t val_scb_aircr;

    if (asserted == 0)
    {
        err = dap_read_word((uint32_t)SCB_AIRCR, &val_scb_aircr);
        if (err < 0)
            return err;

        // qDebug("[swd_init_debug] set_target_reset SCB_AIRCR: 0x%08X", val_scb_aircr);

        val_scb_aircr &= ~SCB_AIRCR_VECTKEY_Msk;
        val_scb_aircr &= ~SCB_AIRCR_PRIGROUP_Msk;
        val_scb_aircr &= ~SCB_AIRCR_SYSRESETREQ_Msk;

        err = dap_write_word(
            (uint32_t)SCB_AIRCR,
            (val_scb_aircr | (0x5FA << SCB_AIRCR_VECTKEY_Pos) | (0 & SCB_AIRCR_PRIGROUP_Msk) | SCB_AIRCR_SYSRESETREQ_Msk));
        if (err < 0)
            return err;
    }

    return 0;
}

int32_t CMSIS_DAP_Base::dap_set_target_state_hw(dap_target_reset_state_t state)
{
    uint32_t err;
    uint32_t val;
    int8_t ap_retries = 2;
    uint16_t retry = 32;

    switch (state)
    {
    case DAP_TARGET_RESET_HOLD:
        err = dap_set_target_reset(1);
        if (err < 0)
            return err;
        return 0;

    case DAP_TARGET_RESET_RUN:
        err = dap_set_target_reset(1);
        if (err < 0)
            return err;
        QThread::msleep(20);
        err = dap_set_target_reset(0);
        if (err < 0)
            return err;
        QThread::msleep(20);
        return 0;

    case DAP_TARGET_RESET_PROGRAM:

        err = swd_init_debug();
        if (err < 0)
            return err;

        // Enable debug
        while (1)
        {
            err = dap_write_word(DBG_HCSR, DBGKEY | C_DEBUGEN);
            if (err < 0)
            {
                ap_retries--;
                if (ap_retries == 0)
                    return -1;
            }
            else
            {
                break;
            }

            err = dap_set_target_reset(1);
            if (err < 0)
                return err;
            QThread::msleep(20);
            err = dap_set_target_reset(0);
            if (err < 0)
                return err;
            QThread::msleep(20);
        }

        // Enable halt on reset
        err = dap_write_word(DBG_EMCR, VC_CORERESET);
        if (err < 0)
            return err;

        // Reset again
        err = dap_set_target_reset(1);
        if (err < 0)
            return err;
        QThread::msleep(20);
        err = dap_set_target_reset(0);
        if (err < 0)
            return err;
        QThread::msleep(20);

        // Enable halt on reset
        err = dap_write_word(DBG_EMCR, VC_CORERESET);
        if (err < 0)
            return err;

        // Enable halt on reset
        err = dap_write_word(DBG_HCSR, C_HALT);
        if (err < 0)
            return err;

        do
        {
            if (retry == 0)
            {
                qDebug("[CMSIS_DAP_Base] dap_set_target_state_hw wait S_HALT fail");
                return -1;
            }

            err = dap_read_word(DBG_HCSR, &val);
            if (err < 0)
                return err;

            qDebug("[CMSIS_DAP_Base] DBG_HCSR: 0x%08X", val);

            if (retry)
                retry--;

        } while ((val & S_HALT) == 0);

        // Disable halt on reset
        err = dap_write_word(DBG_EMCR, 0);
        if (err < 0)
            return err;

        return 0;

    case DAP_TARGET_RUN:
    {
        int32_t err;
        uint32_t idcode;

        err = dap_swd_reset();
        if (err < 0)
        {
            qDebug("[CMSIS_DAP_Base] connect dap_swd_reset fail");
            return err;
        }

        // JTAG-to-SWD
        err = dap_swd_switch(0xE79E);
        if (err < 0)
        {
            qDebug("[CMSIS_DAP_Base] connect dap_swd_switch fail");
            return err;
        }

        err = dap_swd_reset();
        if (err < 0)
        {
            qDebug("[CMSIS_DAP_Base] connect dap_swd_reset fail");
            return err;
        }

        err = dap_swd_read_idcode(&idcode);
        if (err < 0)
        {
            qDebug("[CMSIS_DAP_Base] connect dap_swd_read_idcode fail idcode:0x%08X", idcode);
        }
        else
        {
            qDebug("[CMSIS_DAP_Base] connect dap_swd_read_idcode 0x%08X", idcode);
        }

        err = swd_init_debug();
        if (err < 0)
        {
            qDebug("[CMSIS_DAP_Base] connect swd_init_debug fail");
            return err;
        }

        err = dap_write_word(DBG_HCSR, DBGKEY);
        if (err < 0)
        {
            return err;
        }
        break;
    }

    default:
        return -1;
    }

    return 0;
}

/*******************************************************************************
 * @brief The DAP_SWJ_Sequence Command can be used to generate required SWJ
 *        sequences for SWD/JTAG Reset, SWD<->JTAG switch and Dormant operation.
 * @ref https://arm-software.github.io/CMSIS_5/DAP/html/group__DAP__SWJ__Sequence.html
 * @param None
 * @return None
 ******************************************************************************/
int32_t CMSIS_DAP_Base::dap_swj_sequence(uint8_t bit_count, uint8_t *data)
{
    // qDebug("[CMSIS_DAP_Base] dap_swj_sequence");

    uint8_t tx_buf[512] = {0};
    uint8_t rx_buf[64] = {0};
    int32_t err;
    uint8_t count_byte;
    uint32_t rx_len;

    // qDebug("[dap_hid] dap_swj_sequence");

    tx_buf[0] = 0x12;
    tx_buf[1] = bit_count; // Sequence Count
    // tx_buf[3] = tx_data; // SWDIO Data

    count_byte = bit_count / 8;
    if (bit_count % 8)
        count_byte += 1;
    memcpy(tx_buf + 2, data, count_byte);

    err = dap_request(tx_buf, count_byte + 2, rx_buf, sizeof(rx_buf), &rx_len);
    if (err < 0)
        return err;

    uint8_t status = rx_buf[1];

    err = (status == DAP_OK) ? 0 : -1;

    if (err < 0)
    {
        qDebug("[CMSIS_DAP_Base] dap_swj_sequence_write resp:");
        hexdump(rx_buf, 64);
    }

    return err;
}

int32_t CMSIS_DAP_Base::dap_swd_config(uint8_t cfg)
{
    qDebug("[CMSIS_DAP_Base] dap_swd_config");

    uint8_t tx_buf[512] = {0};
    uint8_t rx_buf[64] = {0};
    int32_t err;
    uint32_t rx_len;

    // qDebug("[dap_hid] dap_swd_config");

    tx_buf[0] = 0x13;
    tx_buf[1] = cfg; // Configuration
    err = dap_request(tx_buf, 2, rx_buf, sizeof(rx_buf), &rx_len);
    if (err < 0)
        return err;

    uint8_t status = rx_buf[1];
    if (status != DAP_OK)
        return -1;

    return 0;
}

int32_t CMSIS_DAP_Base::dap_swd_sequence_write(uint8_t count, uint8_t *tx_data)
{
    qDebug("[CMSIS_DAP_Base] dap_swd_sequence_write");

    uint8_t tx_buf[512] = {0};
    uint8_t rx_buf[64] = {0};
    int32_t err;
    uint32_t rx_len;
    uint8_t count_byte;

    // qDebug("[dap_hid] dap_swd_sequence_write");

    tx_buf[0] = 0x1D;
    tx_buf[1] = count; // Sequence Count
    tx_buf[2] = 0;     // Sequence Info
    // tx_buf[4] = tx_data; // SWDIO Data

    count_byte = count / 8;
    if (count % 8)
        count_byte += 1;

    memcpy(tx_buf + 4, tx_data, count_byte);

    err = dap_request(tx_buf, count_byte + 3, rx_buf, sizeof(rx_buf), &rx_len);
    if (err < 0)
        return -1;

    uint8_t status = rx_buf[1];
    // uint8_t rx_data = rx_buf[2];

    if (status == DAP_OK)
        return 0;
    else
        return -1;
}

int32_t CMSIS_DAP_Base::dap_transfer_config(uint8_t idle_cyless, uint16_t wait_retry, uint16_t match_retry)
{
    qDebug("[CMSIS_DAP_Base] dap_transfer_config");

    uint8_t tx_buf[512] = {0};
    uint8_t rx_buf[64] = {0};
    int32_t err;
    uint32_t rx_len;

    // qDebug("[dap_hid] dap_transfer_config");

    tx_buf[0] = 0x04;
    tx_buf[1] = idle_cyless;
    tx_buf[2] = wait_retry & 0xFF;
    tx_buf[3] = wait_retry >> 8;
    tx_buf[4] = match_retry & 0xFF;
    tx_buf[5] = match_retry >> 8;

    err = dap_request(tx_buf, 6, rx_buf, sizeof(rx_buf), &rx_len);
    if (err < 0)
        return -1;

    uint8_t status = rx_buf[1];

    if (status == DAP_OK)
        return 0;
    else
        return -1;
}

/*******************************************************************************
 * @brief Read/write single and multiple registers.
 * @ref https://arm-software.github.io/CMSIS_5/DAP/html/group__DAP__Transfer.html
 * @param None
 * @return None
 ******************************************************************************/
int32_t CMSIS_DAP_Base::dap_swd_transfer(uint8_t req, uint32_t tx_data, uint8_t *resp, uint32_t *rx_data)
{
    // qDebug("[dap_hid] dap_swd_transfer");
    // qDebug("[CMSIS_DAP_Base] dap_swd_transfer req: 0x%02X tx: 0x%08X", req, tx_data);

    uint8_t tx_buf[512] = {0};
    uint8_t rx_buf[64] = {0};
    int32_t err;
    uint32_t rx_len;

    tx_buf[0] = 0x05; // cmd
    tx_buf[1] = 0;    // DAP_Index
    tx_buf[2] = 1;    // Transfer Count
    tx_buf[3] = req;  // Transfer Request
                      // memcpy(tx_buf + 4, &tx_data, 4); // Transfer Data
    tx_buf[4] = tx_data;
    tx_buf[5] = tx_data >> 8;
    tx_buf[6] = tx_data >> 16;
    tx_buf[7] = tx_data >> 24;

    err = dap_request(tx_buf, 8, rx_buf, sizeof(rx_buf), &rx_len);
    if (err < 0)
        return err;

    // qDebug("[CMSIS_DAP_Base] dap_request");

    uint8_t resp_count = rx_buf[1];

    if (resp_count == 0)
    {
    }

    if (resp_count != 1)
    {
        qDebug("[CMSIS_DAP_Base] dap_swd_transfer resp_count: %d rx_len: %d", resp_count, rx_len);
    }

    if (resp)
        *resp = rx_buf[2];

    if (req & 0x80)
    {
        if (rx_data)
            memcpy(rx_data, rx_buf + 7, 4);
    }
    else
    {
        if (rx_data)
            memcpy(rx_data, rx_buf + 3, 4);
    }

    // if (rx_data)
    //     memcpy(rx_data, rx_buf + 3, 4);

    // if (rx_data)
    //     qDebug("[CMSIS_DAP_Base] dap_swd_transfer done resp: 0x%08X", *rx_data);

    // hexdump(rx_buf, rx_len);

    return 0;
}

int32_t CMSIS_DAP_Base::dap_swd_transfer_retry(uint8_t req, uint32_t tx_data, uint8_t *resp, uint32_t *rx_data)
{
    // qDebug("[CMSIS_DAP_Base] dap_swd_transfer_retry");

    int32_t err;

    for (uint8_t retry_i = 0; retry_i < 100; retry_i++)
    {
        err = dap_swd_transfer(req, tx_data, resp, rx_data);
        if (err < 0)
            return -1;

        if (rx_data && retry_i)
        {
            qDebug("[CMSIS_DAP_Base] dap_swd_transfer_retry i:%d resp: 0x%08X", retry_i, *rx_data);
        }

        // qDebug("[CMSIS_DAP_Base] dap_swd_transfer_retry i:%d resp: 0x%02X", retry_i, *resp);

        if (*resp != DAP_TRANSFER_WAIT)
            // if ((*resp & DAP_TRANSFER_WAIT) == 0)
            break;

        // QThread::msleep(100);
    }

    if (*resp != DAP_TRANSFER_OK)
        // if ((*resp & DAP_TRANSFER_OK) == 0)
        return Devices::Error::DEVICE_ERR_DAP_TRANSFER_ERROR;

    return 0;
}

int32_t CMSIS_DAP_Base::dap_swd_transfer_block(uint8_t req, uint32_t tx_data, uint8_t *resp, uint32_t *timestamp, uint32_t *rx_data)
{
    // qDebug("[CMSIS_DAP_Base] dap_swd_transfer_block");

    uint8_t tx_buf[512] = {0};
    uint8_t rx_buf[64] = {0};
    int32_t err;
    uint32_t rx_len;

    tx_buf[0] = 0x06;
    tx_buf[1] = 0;                   // DAP_Index
    tx_buf[2] = 1;                   // Transfer Count
    tx_buf[3] = req;                 // Transfer Request
    memcpy(tx_buf + 4, &tx_data, 4); // Transfer Data

    err = dap_request(tx_buf, 8, rx_buf, sizeof(rx_buf), &rx_len);
    if (err < 0)
        return err;

    qDebug("[DAP_Base] dap_swd_transfer_block resp:");
    hexdump(rx_buf, 64);

    *(resp) = rx_buf[2];
    memcpy(timestamp, rx_buf + 3, 4);
    memcpy(rx_data, rx_buf + 7, 4);

    return 0;
}

int32_t CMSIS_DAP_Base::dap_swd_reset()
{
    // qDebug("[CMSIS_DAP_Base] dap_swd_reset");

    uint8_t tmp_in[8];

    memset(tmp_in, 0xFF, sizeof(tmp_in));
    return dap_swj_sequence(51, tmp_in);
}

int32_t CMSIS_DAP_Base::dap_swd_switch(uint16_t val)
{
    // qDebug("[CMSIS_DAP_Base] dap_swd_switch");

    uint8_t tmp_in[2];
    tmp_in[0] = val & 0xff;
    tmp_in[1] = (val >> 8) & 0xff;

    return dap_swj_sequence(16, tmp_in);
}

int32_t CMSIS_DAP_Base::dap_swd_read_dp(uint8_t addr, uint32_t *val)
{
    // qDebug("[CMSIS_DAP_Base] dap_swd_read_dp");

    int32_t err;
    uint8_t resp = 0;

    uint8_t req = DAP_TRANS_DP | DAP_TRANS_READ_REG | DAP_TRANS_REG_ADDR(addr);
    err = dap_swd_transfer_retry(req, 0, &resp, val);
    if (err < 0)
        return Devices::Error::DEVICE_ERR_DAP_TRANSFER_ERROR;

    if (resp != DAP_TRANSFER_OK)
        return Devices::Error::DEVICE_ERR_DAP_TRANSFER_ERROR;

    return 0;
}

int32_t CMSIS_DAP_Base::dap_swd_write_dp(uint8_t addr, uint32_t val)
{
    // qDebug("[CMSIS_DAP_Base] dap_swd_write_dp");

    uint8_t req;
    uint8_t data[4];
    int32_t err;

    uint8_t resp = 0;
    uint32_t rx_data;

    switch (addr)
    {
    case DP_SELECT:
        if (dap_state.select == val)
        {
            return 1;
        }

        dap_state.select = val;
        break;

    default:
        break;
    }

    req = DAP_TRANS_DP | DAP_TRANS_WRITE_REG | DAP_TRANS_REG_ADDR(addr);

    for (uint8_t i = 0; i < 10; i++)
    {
        err = dap_swd_transfer(req, val, &resp, &rx_data);
        if (err < 0)
            return -1;

        if ((resp != DAP_TRANSFER_WAIT))
            break;
    }

    if (resp != DAP_TRANSFER_OK)
        return -1;

    return 0;
}

int32_t CMSIS_DAP_Base::dap_swd_read_ap(uint8_t addr, uint32_t *val)
{
    // qDebug("[CMSIS_DAP_Base] dap_swd_read_ap");

    int32_t err;
    uint8_t req;
    uint8_t resp = 0;

    uint32_t apsel = addr & 0xff000000;
    uint32_t bank_sel = addr & APBANKSEL;

    err = dap_swd_write_dp(DP_SELECT, apsel | bank_sel);
    if (err < 0)
        return 0;

    req = DAP_TRANS_AP | DAP_TRANS_READ_REG | DAP_TRANS_REG_ADDR(addr);

    for (uint8_t i = 0; i < 10; i++)
    {
        err = dap_swd_transfer(req, 0, &resp, val);
        if (err < 0)
            return -1;

        if ((resp != DAP_TRANSFER_WAIT))
            break;
    }

    if (resp != DAP_TRANSFER_OK)
        return -1;

    return 0;
}

int32_t CMSIS_DAP_Base::dap_swd_write_ap(uint8_t addr, uint32_t val)
{
    // qDebug("[CMSIS_DAP_Base] dap_swd_write_ap");

    uint32_t apsel = addr & 0xff000000;
    uint32_t bank_sel = addr & APBANKSEL;
    int32_t err;
    uint8_t req;
    uint8_t resp;
    uint32_t rx_data;
    uint8_t i;

    err = dap_swd_write_dp(DP_SELECT, apsel | bank_sel);
    if (err < 0)
        return 0;

    switch (addr)
    {
    case AP_CSW:
        if (dap_state.csw == val)
        {
            return 1;
        }

        dap_state.csw = val;
        break;

    default:
        break;
    }

    req = DAP_TRANS_AP | DAP_TRANS_WRITE_REG | DAP_TRANS_REG_ADDR(addr);
    for (i = 0; i < 10; i++)
    {
        err = dap_swd_transfer(req, val, &resp, &rx_data);
        if (err < 0)
            return -1;

        if ((resp != DAP_TRANSFER_WAIT))
            break;
    }

    if (i >= 10)
        return -1;

    if (resp != DAP_TRANSFER_OK)
        return -1;

    req = DAP_TRANS_DP | DAP_TRANS_READ_REG | DAP_TRANS_REG_ADDR(DP_RDBUFF);
    for (i = 0; i < 10; i++)
    {
        err = dap_swd_transfer(req, val, &resp, &rx_data);
        if (err < 0)
            return -1;

        if ((resp != DAP_TRANSFER_WAIT))
            break;
    }

    if (i >= 10)
        return -1;

    if (resp != DAP_TRANSFER_OK)
        return -1;

    // qDebug("[CMSIS_DAP_Base] dap_swd_write_ap done");

    return 0;
}

int32_t CMSIS_DAP_Base::dap_read_data(uint32_t addr, uint32_t *data)
{
    // qDebug("[CMSIS_DAP_Base] dap_read_data");

    int32_t err;
    uint8_t req;
    uint8_t resp;
    uint32_t rx_data;

    //  put addr in TAR register
    req = DAP_TRANS_AP | DAP_TRANS_WRITE_REG | (1 << 2);
    for (uint8_t i = 0; i < 10; i++)
    {
        err = dap_swd_transfer(req, addr, &resp, &rx_data);
        if (err < 0)
            return -1;

        if ((resp != DAP_TRANSFER_WAIT))
            break;
    }

    if (resp != DAP_TRANSFER_OK)
        return -1;

    // read data
    req = DAP_TRANS_AP | DAP_TRANS_READ_REG | (3 << 2);
    for (uint8_t i = 0; i < 10; i++)
    {
        err = dap_swd_transfer(req, 0, &resp, data);
        if (err < 0)
            return -1;

        if ((resp != DAP_TRANSFER_WAIT))
            break;
    }

    if (resp != DAP_TRANSFER_OK)
        return -1;

    // qDebug("[swd_init_debug] read_data 0x%08X", rx_data);

    // dummy read
    req = DAP_TRANS_DP | DAP_TRANS_READ_REG | DAP_TRANS_REG_ADDR(DP_RDBUFF);
    for (uint8_t i = 0; i < 10; i++)
    {
        err = dap_swd_transfer(req, 0, &resp, &rx_data);
        if (err < 0)
            return -1;

        if ((resp != DAP_TRANSFER_WAIT))
            break;
    }

    if (resp != DAP_TRANSFER_OK)
        return -1;

    // qDebug("[swd_init_debug] read_data 0x%08X", data);

    return 0;
}

int32_t CMSIS_DAP_Base::dap_write_data(uint32_t addr, uint32_t data)
{
    // qDebug("[CMSIS_DAP_Base] dap_write_data");

    int32_t err;
    uint8_t req;
    uint8_t resp;
    uint32_t rx_data;

    //  put addr in TAR register
    req = DAP_TRANS_AP | DAP_TRANS_WRITE_REG | (1 << 2);
    for (uint8_t i = 0; i < 10; i++)
    {
        err = dap_swd_transfer(req, addr, &resp, &rx_data);
        if (err < 0)
            return -1;

        if ((resp != DAP_TRANSFER_WAIT))
            break;
    }

    if (resp != DAP_TRANSFER_OK)
        return -1;

    //  write data
    req = DAP_TRANS_AP | DAP_TRANS_WRITE_REG | (3 << 2);
    for (uint8_t i = 0; i < 10; i++)
    {
        err = dap_swd_transfer(req, data, &resp, &rx_data);
        if (err < 0)
            return -1;

        if ((resp != DAP_TRANSFER_WAIT))
            break;
    }

    if (resp != DAP_TRANSFER_OK)
        return -1;

    // dummy read
    req = DAP_TRANS_DP | DAP_TRANS_READ_REG | DAP_TRANS_REG_ADDR(DP_RDBUFF);
    for (uint8_t i = 0; i < 10; i++)
    {
        err = dap_swd_transfer(req, 0, &resp, &rx_data);
        if (err < 0)
            return -1;

        if ((resp != DAP_TRANSFER_WAIT))
            break;
    }

    if (resp != DAP_TRANSFER_OK)
        return -1;

    return 0;
}

int32_t CMSIS_DAP_Base::dap_read_word(uint32_t addr, uint32_t *val)
{
    // qDebug("[CMSIS_DAP_Base] dap_read_word");

    int32_t err;

    err = dap_swd_write_ap(AP_CSW, CSW_VALUE | CSW_SIZE32);
    if (err < 0)
        return err;

    err = dap_read_data(addr, val);
    if (err < 0)
        return err;

    return 0;
}

int32_t CMSIS_DAP_Base::dap_write_word(uint32_t addr, uint32_t val)
{
    // qDebug("[CMSIS_DAP_Base] dap_write_word");

    int32_t err;

    err = dap_swd_write_ap(AP_CSW, CSW_VALUE | CSW_SIZE32);
    if (err < 0)
        return err;

    err = dap_write_data(addr, val);
    if (err < 0)
        return err;

    return 0;
}

int32_t CMSIS_DAP_Base::dap_read_byte(uint32_t addr, uint8_t *val)
{
    // qDebug("[CMSIS_DAP_Base] dap_read_byte");

    int32_t err;
    uint32_t tmp;

    err = dap_swd_write_ap(AP_CSW, CSW_VALUE | CSW_SIZE8);
    if (err < 0)
        return err;

    err = dap_read_data(addr, &tmp);
    if (err < 0)
        return err;

    *val = (uint8_t)(tmp >> ((addr & 0x03) << 3));

    qDebug("[CMSIS_DAP_Base] dap_read_byte addr:0x%X val:%08X", addr, *val);

    return 0;
}

int32_t CMSIS_DAP_Base::dap_write_byte(uint32_t addr, uint8_t val)
{
    // qDebug("[CMSIS_DAP_Base] dap_write_byte");

    int32_t err;
    uint32_t tmp;

    err = dap_swd_write_ap(AP_CSW, CSW_VALUE | CSW_SIZE8);
    if (err < 0)
        return err;

    tmp = val << ((addr & 0x03) << 3);

    err = dap_write_data(addr, tmp);
    if (err < 0)
        return err;

    return 0;
}

int32_t CMSIS_DAP_Base::dap_read_block(uint32_t addr, uint8_t *data, uint32_t size)
{
    // qDebug("[CMSIS_DAP_Base] dap_read_block");

    int32_t err;
    uint8_t req, resp;
    uint32_t size_in_words;
    uint32_t i;

    uint8_t *prev_data = data;

    if (size == 0)
        return -1;

    size_in_words = size / 4;
    err = dap_swd_write_ap(AP_CSW, CSW_VALUE | CSW_SIZE32);
    if (err < 0)
        return err;

    // TAR write
    req = DAP_TRANS_AP | DAP_TRANS_WRITE_REG | AP_TAR;
    err = dap_swd_transfer_retry(req, addr, &resp, NULL);
    if (err < 0)
        return err;

    // read data
    req = DAP_TRANS_AP | DAP_TRANS_READ_REG | AP_DRW;

    // TODO:为什么有的芯片需要加这个dummy而有的不用
    // initiate first read, data comes back in next read
    // err = dap_swd_transfer_retry(req, 0, &resp, NULL);
    // if (err < 0)
    //     return err;

    for (i = 0; i < (size_in_words); i++)
    {
        err = dap_swd_transfer_retry(req, 0, &resp, (uint32_t *)data);
        if (err < 0)
            return err;

        data += 4;
    }

    // read last word
    // req = DAP_TRANS_DP | DAP_TRANS_READ_REG | DAP_TRANS_REG_ADDR(DP_RDBUFF);
    // err = dap_swd_transfer_retry(req, 0, &resp, (uint32_t *)data);
    // if (err < 0)
    //     return err;
    req = DAP_TRANS_DP | DAP_TRANS_READ_REG | DAP_TRANS_REG_ADDR(DP_RDBUFF);
    err = dap_swd_transfer_retry(req, 0, &resp, NULL);
    if (err < 0)
        return err;

    // qDebug("[CMSIS_DAP_Base] read_block addr:0x%X size:0x%X", addr, size);
    // hexdump(prev_data, size);

    return 0;
}

int32_t CMSIS_DAP_Base::dap_write_block(uint32_t addr, uint8_t *data, uint32_t size)
{
    // qDebug("[CMSIS_DAP_Base] dap_write_block");

    int32_t err;
    uint8_t req, resp;
    uint32_t size_in_words;
    uint32_t i;

    if (size == 0)
    {
        return 0;
    }

    size_in_words = size / 4;
    err = dap_swd_write_ap(AP_CSW, CSW_VALUE | CSW_SIZE32);
    if (err < 0)
        return err;

    // TAR write
    req = DAP_TRANS_AP | DAP_TRANS_WRITE_REG | AP_TAR;
    err = dap_swd_transfer_retry(req, addr, &resp, NULL);
    if (err < 0)
        return err;

    // DRW write
    req = DAP_TRANS_AP | DAP_TRANS_WRITE_REG | AP_DRW;

    for (i = 0; i < size_in_words; i++)
    {
        err = dap_swd_transfer_retry(req, *((uint32_t *)data), &resp, NULL);
        if (err < 0)
            return err;

        data += 4;
    }

    // dummy read
    req = DAP_TRANS_DP | DAP_TRANS_READ_REG | DAP_TRANS_REG_ADDR(DP_RDBUFF);
    err = dap_swd_transfer_retry(req, 0, &resp, NULL);
    if (err < 0)
        return err;

    return 0;
}

int32_t CMSIS_DAP_Base::dap_swd_read_idcode(uint32_t *idcode)
{
    // qDebug("[CMSIS_DAP_Base] dap_swd_read_idcode");

    int32_t err;

    uint8_t tmp_in[1];
    tmp_in[0] = 0x00;

    err = dap_swj_sequence(8, tmp_in);
    if (err < 0)
        return err;

    return dap_swd_read_dp(DP_IDCODE, idcode);
}

int32_t CMSIS_DAP_Base::dap_jtag_2_swd()
{
    int32_t err;
    uint32_t idcode;

    err = dap_swd_reset();
    if (err < 0)
        return err;

    err = dap_swd_switch(0xE79E);
    if (err < 0)
        return err;

    err = dap_swd_reset();
    if (err < 0)
        return err;

    err = dap_swd_read_idcode(&idcode);
    if (err < 0)
        return err;

    return 0;
}

int32_t CMSIS_DAP_Base::swd_init_debug()
{
    int32_t err;
    uint32_t tmp = 0;
    int timeout = 100;
    uint8_t i = 0;

    memset(&dap_state, 0, sizeof(dap_state));
    dap_state.select = 0xFFFFFFFF;
    dap_state.csw = 0xFFFFFFFF;

    err = dap_jtag_2_swd();
    if (err < 0)
        return err;

    err = dap_swd_write_dp(DP_ABORT, STKCMPCLR | STKERRCLR | WDERRCLR | ORUNERRCLR);
    if (err < 0)
    {
        qDebug("[swd_init_debug] dap_swd_write_dp DP_ABORT fail");
        return err;
    }

    // Ensure CTRL/STAT register selected in DPBANKSEL
    err = dap_swd_write_dp(DP_SELECT, 0);
    if (err < 0)
    {
        qDebug("[swd_init_debug] dap_swd_write_dp DP_SELECT fail");
        return err;
    }

    // Power up
    err = dap_swd_write_dp(DP_CTRL_STAT, CSYSPWRUPREQ | CDBGPWRUPREQ);
    if (err < 0)
    {
        qDebug("[swd_init_debug] Power up fail");
        return err;
    }

    for (i = 0; i < timeout; i++)
    {
        err = dap_swd_read_dp(DP_CTRL_STAT, &tmp);
        if (err < 0)
            return err;

        if ((tmp & (CDBGPWRUPACK | CSYSPWRUPACK)) == (CDBGPWRUPACK | CSYSPWRUPACK))
        {
            // Break from loop if powerup is complete
            break;
        }
    }

    if (i == timeout)
    {
        return -1;
    }

    err = dap_swd_write_dp(DP_CTRL_STAT, CSYSPWRUPREQ | CDBGPWRUPREQ | TRNNORMAL | MASKLANE);
    if (err < 0)
        return err;

    // call a target dependant function:
    // some target can enter in a lock state, this function can unlock these targets
    // target_unlock_sequence();

    err = dap_swd_write_dp(DP_SELECT, 0);
    if (err < 0)
        return err;

    return 0;
}

int32_t CMSIS_DAP_Base::swd_write_debug_state(debug_state_t *state)
{
    int32_t err;
    uint32_t i, status;

    err = dap_swd_write_dp(DP_SELECT, 0);
    if (err < 0)
        return err;

    // R0, R1, R2, R3
    for (i = 0; i < 4; i++)
    {
        err = swd_write_core_register(i, state->r[i]);
        if (err < 0)
            return err;
    }

    // R9
    err = swd_write_core_register(9, state->r[9]);
    if (err < 0)
        return err;

    // R13, R14, R15
    for (i = 13; i < 16; i++)
    {
        err = swd_write_core_register(i, state->r[i]);
        if (err < 0)
            return err;
    }

    // xPSR
    err = swd_write_core_register(16, state->xpsr);
    if (err < 0)
        return err;

    err = dap_write_word(DBG_HCSR, DBGKEY | C_DEBUGEN);
    if (err < 0)
        return err;

    // check status
    err = dap_swd_read_dp(DP_CTRL_STAT, &status);
    if (err < 0)
        return err;

    if (status & (STICKYERR | WDATAERR))
        return -1;

    return 0;
}

int32_t CMSIS_DAP_Base::swd_wait_until_halted(void)
{
    // Wait for target to stop
    int32_t err;
    uint32_t val, i, timeout = 1500;

    // TODO: 需要根据实际的情况设置超时值
    for (i = 0; i < timeout; i++)
    {
        err = dap_read_word(DBG_HCSR, &val);
        if (err < 0)
            return -1;

        if (val & S_HALT)
            return 0;
    }

    return -1;
}

int32_t CMSIS_DAP_Base::swd_read_core_register(uint32_t n, uint32_t *val)
{
    int32_t err;
    int i = 0, timeout = 100;

    err = dap_write_word(DCRSR, n);
    if (err < 0)
        return err;

    // wait for S_REGRDY
    for (i = 0; i < timeout; i++)
    {
        err = dap_read_word(DHCSR, val);
        if (err < 0)
            return err;

        if (*val & S_REGRDY)
        {
            break;
        }
    }

    if (i == timeout)
        return -1;

    err = dap_read_word(DCRDR, val);
    if (err < 0)
        return err;

    return 0;
}

int32_t CMSIS_DAP_Base::swd_write_core_register(uint32_t n, uint32_t val)
{
    int32_t err;
    int i = 0, timeout = 100;

    err = dap_write_word(DCRDR, val);
    if (err < 0)
        return err;

    err = dap_write_word(DCRSR, n | REGWnR);
    if (err < 0)
        return err;

    // wait for S_REGRDY
    for (i = 0; i < timeout; i++)
    {
        err = dap_read_word(DHCSR, &val);
        if (err < 0)
            return err;

        if (val & S_REGRDY)
        {
            return 0;
        }
    }

    return -1;
}

int32_t CMSIS_DAP_Base::dap_hid_resp_status_return(uint8_t *rx_data)
{
    qDebug("[CMSIS_DAP_Base] dap_hid_resp_status_return");

    uint8_t status = rx_data[1];

    if (status == DAP_OK)
        return 1;
    else if (status == DAP_ERROR)
        return 0;
    else
        return -1;
}
