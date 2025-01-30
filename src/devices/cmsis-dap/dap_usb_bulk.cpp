#include "dap_usb_bulk.h"
#include "utils.h"

extern libusb_context *g_libusb_context;
libusb_device **CMSIS_DAP_V2::libusb_device_list = NULL;

CMSIS_DAP_V2::CMSIS_DAP_V2(libusb_device *dev, int interface_number, uint8_t ep_in_addr, uint8_t ep_out_addr)
{
    int32_t err;
    int rc = 0;
    char buf_str_descriptor[1024];

    this->dev = dev;
    this->interface_number = interface_number;
    this->ep_in_addr = ep_in_addr;
    this->ep_out_addr = ep_out_addr;
    err_code = 0;

    // qDebug("[CMSIS_DAP_V2] interface:%d ep_in:0x%02X ep_out:0x%02X", interface_number, ep_in_addr, ep_out_addr);

    rc = libusb_get_device_descriptor(this->dev, &desc);
    if (rc < LIBUSB_SUCCESS)
    {
        qDebug("[CMSIS_DAP_V2] libusb_get_device_descriptor fail %d", rc);
        err_code = -1;
        return;
    }

    close_device();
    err = open_device();
    if (err < 0)
    {
        qDebug("[CMSIS_DAP_V2] open fail");
        err_code = -1;
        return;
    }

    memset(buf_str_descriptor, 0, sizeof(buf_str_descriptor));
    rc = libusb_get_string_descriptor_ascii(handle, desc.iProduct, (unsigned char *)buf_str_descriptor, sizeof(buf_str_descriptor));
    if (rc < LIBUSB_SUCCESS)
    {
        qDebug("[CMSIS_DAP_V2] libusb_get_string_descriptor_ascii iProduct fail %d", rc);
        err_code = -1;
        close_device();
        return;
    }
    product_str = QString::fromUtf8(buf_str_descriptor, rc);

    rc = libusb_get_string_descriptor_ascii(handle, desc.iManufacturer, (unsigned char *)buf_str_descriptor, sizeof(buf_str_descriptor));
    if (rc < LIBUSB_SUCCESS)
    {
        qDebug("[CMSIS_DAP_V2] libusb_get_string_descriptor_ascii iManufacturer fail %d", rc);
        err_code = -1;
        close_device();
        return;
    }
    manufacturer_str = QString::fromUtf8(buf_str_descriptor, rc);

    char buf_str_serial[33];
    rc = libusb_get_string_descriptor_ascii(handle, desc.iSerialNumber, (unsigned char *)buf_str_serial, sizeof(buf_str_serial));
    if (rc < LIBUSB_SUCCESS)
    {
        qDebug("[CMSIS_DAP_V2] libusb_get_string_descriptor_ascii iSerialNumber fail %d", rc);
        err_code = -1;
        close_device();
        return;
    }
    serial_number_str = QString::fromUtf8(buf_str_serial, rc);

    struct libusb_config_descriptor *config_desc;
    err = libusb_get_config_descriptor(dev, 0, &config_desc);
    if (err < LIBUSB_SUCCESS)
    {
        err_code = -1;
        close_device();
        return;
    }

    const struct libusb_interface_descriptor *intf_desc = &config_desc->interface[interface_number].altsetting[0];

    char interface_str_buf[256] = {0};
    err = libusb_get_string_descriptor_ascii(
        handle, intf_desc->iInterface,
        (uint8_t *)interface_str_buf, sizeof(interface_str_buf));
    if (err < LIBUSB_SUCCESS)
    {
        err_code = -1;
        close_device();
        return;
    }
    interface_str = QString::fromUtf8(interface_str_buf, sizeof(interface_str_buf));

    err = get_info_cmsis_dap_protocol_version(&version_str);
    if (err > 0)
    {
        qDebug("[CMSIS_DAP_V2] get_version fail");
        err_code = -1;
        close_device();
        return;
    }

    // qDebug("[CMSIS_DAP_V2] get_version: %s", qPrintable(version_str));

    // qDebug("         iProduct: %s", qPrintable(product_str));
    // qDebug("    iManufacturer: %s", qPrintable(manufacturer_str));
    // qDebug("    iSerialNumber: %s", qPrintable(serial_number_str));

    close_device();
}

CMSIS_DAP_V2::~CMSIS_DAP_V2()
{
    close_device();
}

int32_t CMSIS_DAP_V2::enum_device(DeviceList *dev_list)
{
    libusb_device_handle *handle = NULL;
    int rc = 0;
    int err;
    ssize_t count = 0;
    char buf_product[1024];
    char buf_serial[1024];
    uint8_t ep_out_addr;
    uint8_t ep_in_addr;
    // char buf_manufacturer[1024];
    // char buf_serial_number[1024];

    if (dev_list == NULL)
        return -1;

    dev_list->clear();

    if (libusb_device_list)
    {
        libusb_free_device_list(libusb_device_list, 1);
        libusb_device_list = NULL;
    }

    // count = libusb_get_device_list(context, &list);
    count = libusb_get_device_list(g_libusb_context, &libusb_device_list);

    // qDebug("[CMSIS_DAP_V2] =====================================================");
    // qDebug("[CMSIS_DAP_V2] libusb_get_device_list %d", count);

    for (size_t idx = 0; idx < count; idx++)
    {
        libusb_device *device = libusb_device_list[idx];
        libusb_device_descriptor dev_desc = {0};
        int interface_number = -1;

        rc = libusb_get_device_descriptor(device, &dev_desc);
        if (rc != LIBUSB_SUCCESS)
        {
            qDebug("[CMSIS_DAP_V2] libusb_get_device_descriptor fail %d", rc);
            continue;
        }
        // qDebug("Vendor:Device = 0x%04X:0x%04X", desc.idVendor, desc.idProduct);

        rc = libusb_open(device, &handle);
        if (rc != LIBUSB_SUCCESS)
        {
            if (rc == LIBUSB_ERROR_NOT_SUPPORTED)
                continue;

            qDebug("[CMSIS_DAP_V2] libusb_open fail %d %s", rc, libusb_error_name(rc));
            continue;
        }

        // memset(buf_manufacturer, 0, sizeof(buf_manufacturer));
        memset(buf_product, 0, sizeof(buf_product));
        // memset(buf_serial_number, 0, sizeof(buf_serial_number));

        if (dev_desc.iProduct == 0)
        {
            qDebug("[CMSIS_DAP_V2] iProduct empty");
            libusb_close(handle);
            continue;
        }

        rc = libusb_get_string_descriptor_ascii(handle, dev_desc.iSerialNumber, (unsigned char *)buf_serial, sizeof(buf_serial));
        if (rc < LIBUSB_SUCCESS)
        {
            // qDebug("[CMSIS_DAP_V2] libusb_get_string_descriptor_ascii iSerialNumber fail %d", rc);
            libusb_close(handle);
            continue;
        }

        rc = libusb_get_string_descriptor_ascii(handle, dev_desc.iProduct, (unsigned char *)buf_product, sizeof(buf_product));
        if (rc < LIBUSB_SUCCESS)
        {
            // qDebug("[CMSIS_DAP_V2] libusb_get_string_descriptor_ascii iProduct fail %d", rc);
            libusb_close(handle);
            continue;
        }

        // qDebug("[CMSIS_DAP_V2] iProduct %s", buf_product);

        if (strstr(buf_product, "CMSIS-DAP") == NULL)
        {
            libusb_close(handle);
            continue;
        }

        for (int config = 0; config < dev_desc.bNumConfigurations; config++)
        {
            struct libusb_config_descriptor *config_desc;
            err = libusb_get_config_descriptor(device, config, &config_desc);
            if (err != LIBUSB_SUCCESS)
            {
                qDebug("[CMSIS_DAP_V2] libusb_get_config_descriptor fail %d", err);
                libusb_close(handle);
                continue;
            }

            for (int interface = 0; interface < config_desc->bNumInterfaces; interface++)
            {
                const struct libusb_interface_descriptor *intf_desc = &config_desc->interface[interface].altsetting[0];

                if (intf_desc->iInterface != 0)
                {
                    char interface_str[256] = {0};

                    err = libusb_get_string_descriptor_ascii(
                        handle, intf_desc->iInterface,
                        (uint8_t *)interface_str, sizeof(interface_str));
                    if (err < LIBUSB_SUCCESS)
                    {
                        continue;
                    }

                    if (strstr(interface_str, "CMSIS-DAP") == NULL)
                    {
                        continue;
                    }

                    if (intf_desc->bNumEndpoints < 2)
                    {
                        continue;
                    }

                    if ((intf_desc->endpoint[0].bmAttributes & 3) != LIBUSB_TRANSFER_TYPE_BULK ||
                        (intf_desc->endpoint[0].bEndpointAddress & 0x80) != LIBUSB_ENDPOINT_OUT)
                    {
                        continue;
                    }

                    if ((intf_desc->endpoint[1].bmAttributes & 3) != LIBUSB_TRANSFER_TYPE_BULK ||
                        (intf_desc->endpoint[1].bEndpointAddress & 0x80) != LIBUSB_ENDPOINT_IN)
                    {
                        continue;
                    }

                    ep_out_addr = intf_desc->endpoint[0].bEndpointAddress;
                    ep_in_addr = intf_desc->endpoint[1].bEndpointAddress;

                    interface_number = intf_desc->bInterfaceNumber;

                    // qDebug("[CMSIS_DAP_V2] found interface %d string '%s'", intf_desc->bInterfaceNumber, interface_str);
                }
            }
        }
        libusb_close(handle);

        if (interface_number == -1)
            continue;

        CMSIS_DAP_V2 *tmp_dev = new CMSIS_DAP_V2(device, interface_number, ep_in_addr, ep_out_addr);
        if (tmp_dev->error() == -1)
        {
            delete tmp_dev;
            continue;
        }

        dev_list->push_back(tmp_dev);
    }

    // if (dev_list->count() != 0)
    // {
    //     qDebug("[CMSIS_DAP_V2] =====================================================");
    //     qDebug("[CMSIS_DAP_V2] dev_list %d", dev_list->count());
    // }

    return dev_list->count();
}

bool CMSIS_DAP_V2::equal(const Devices &device)
{
    CMSIS_DAP_V2 *dap_v2 = (CMSIS_DAP_V2 *)&device;
    bool result;

    if ((interface_number == dap_v2->interface_number) &&   // 接口号
        (interface_str == dap_v2->interface_str) &&         // 接口名称
        (serial_number_str == dap_v2->serial_number_str) && // 序列号
        (manufacturer_str == dap_v2->manufacturer_str) &&   // 厂商
        (product_str == dap_v2->product_str)                // 产品
    )
    {
        result = true;
    }
    else
    {
        result = false;
    }

    return result;
}

int32_t CMSIS_DAP_V2::open_device()
{
    int err;

    close_device();
    err = libusb_open(dev, &handle);
    if (err != LIBUSB_SUCCESS)
    {
        QString err_info = QString::fromUtf8(libusb_error_name(err));
        qDebug("[CMSIS_DAP_V2] libusb_open fail: %s", qUtf8Printable(err_info));
        return -1;
    }

    err = libusb_claim_interface(handle, interface_number);
    if (err != LIBUSB_SUCCESS)
    {
        QString err_info = QString::fromUtf8(libusb_error_name(err));
        qDebug("[CMSIS_DAP_V2] libusb_claim_interface fail: %s", qUtf8Printable(err_info));
        return -1;
    }

    return 0;
}

int32_t CMSIS_DAP_V2::close_device()
{

    if (handle == NULL)
        return -1;

    libusb_release_interface(handle, interface_number);
    libusb_close(handle);
    handle = NULL;

    return 0;
}

int32_t CMSIS_DAP_V2::dap_request(uint8_t *tx_data, uint32_t tx_len, uint8_t *rx_data, uint32_t rx_buf_size, uint32_t *rx_len)
{
    int err;
    int actual_length;
    // uint8_t tx_buf[512] = {0};
    // uint8_t rx_buf[512] = {0};

    // 输出端点
    err = libusb_bulk_transfer(handle, ep_out_addr, tx_data, tx_len, &actual_length, 100);
    if (err < 0)
    {
        QString err_info = QString::fromUtf8(libusb_error_name(err));
        qDebug("[CMSIS_DAP_V2] dap_request out fail: %s", qUtf8Printable(err_info));
        return err;
    }

    // 输入端点
    err = libusb_bulk_transfer(handle, ep_in_addr, rx_data, rx_buf_size, &actual_length, 100);
    if (err < 0)
    {
        QString err_info = QString::fromUtf8(libusb_error_name(err));
        qDebug("[CMSIS_DAP_V2] dap_request in fail: %s", qUtf8Printable(err_info));
        return err;
    }

    // 首字节一定与发送相等
    if (rx_data[0] != tx_data[0])
    {
        qDebug("[CMSIS_DAP_V2] dap_request out&in cmd not equal: 0x%02X 0x%02x", tx_data[0], rx_data[0]);
        return -1;
    }

    *rx_len = actual_length;
    return 0;
}
