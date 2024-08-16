#include "dap_usb_bulk.h"

CMSIS_DAP_V2::CMSIS_DAP_V2(libusb_device *dev)
{
    int32_t err;
    int rc = 0;
    char buf_str_descriptor[1024];

    this->dev = dev;
    err_code = 0;

    rc = libusb_get_device_descriptor(this->dev, &desc);
    if (rc < LIBUSB_SUCCESS)
    {
        qDebug("[CMSIS_DAP_V2] libusb_get_device_descriptor fail %d", rc);
        err_code = -1;
        return;
    }

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
        close_device();
        return;
    }
    serial_number_str = QString::fromUtf8(buf_str_serial, rc);

    // qDebug("         iProduct: %s", qPrintable(product_str));
    // qDebug("    iManufacturer: %s", qPrintable(manufacturer_str));
    // qDebug("    iSerialNumber: %s", qPrintable(serial_number_str));

    close_device();
}

CMSIS_DAP_V2::~CMSIS_DAP_V2()
{
    // close_device();
}

int32_t CMSIS_DAP_V2::enum_device(QList<CMSIS_DAP_V2 *> *dev_list)
{
    libusb_context *context = NULL;
    libusb_device **list = NULL;
    libusb_device_handle *handle = NULL;
    int rc = 0;
    int err;
    ssize_t count = 0;
    char buf_product[1024];
    // char buf_manufacturer[1024];
    // char buf_serial_number[1024];

    if (dev_list == NULL)
        return -1;

    dev_list->clear();

    rc = libusb_init(&context);
    if (rc != 0)
    {
        qDebug("[CMSIS_DAP_V2] libusb_init fail %d", rc);
        libusb_exit(context);
        return -1;
    }

    count = libusb_get_device_list(context, &list);

    // qDebug("[CMSIS_DAP_V2] =====================================================");
    // qDebug("[CMSIS_DAP_V2] libusb_get_device_list %d", count);

    for (size_t idx = 0; idx < count; ++idx)
    {
        libusb_device *device = list[idx];
        libusb_device_descriptor dev_desc = {0};

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
            // qDebug("[CMSIS_DAP_V2] libusb_open fail %d", rc);
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

        rc = libusb_get_string_descriptor_ascii(handle, dev_desc.iProduct, (unsigned char *)buf_product, sizeof(buf_product));
        if (rc < LIBUSB_SUCCESS)
        {
            qDebug("[CMSIS_DAP_V2] libusb_get_string_descriptor_ascii iProduct fail %d", rc);
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
        }

        libusb_close(handle);
        CMSIS_DAP_V2 *tmp_dev = new CMSIS_DAP_V2(device);
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

    libusb_free_device_list(list, 1);
    libusb_exit(context);

    return dev_list->count();
}

bool CMSIS_DAP_V2::device_list_compare(QList<CMSIS_DAP_V2 *> a_list, QList<CMSIS_DAP_V2 *> b_list)
{
    // 比较大小
    if (a_list.count() != b_list.count())
        return 1;

    // 比较内容

    return 0;
}

int32_t CMSIS_DAP_V2::open_device()
{
    int rc = 0;

    rc = libusb_open(dev, &handle);
    if (rc != LIBUSB_SUCCESS)
    {
        return -1;
    }

    return 0;
}

void CMSIS_DAP_V2::close_device()
{
    libusb_close(handle);
    handle = NULL;
}
