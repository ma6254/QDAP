#include "devices.h"
#include "dap_usb_hid.h"

Devices::Devices()
{
}

Devices::Devices(const Devices &src_device)
{
    memcpy(this, &src_device, sizeof(Devices));
}

Devices::~Devices()
{
}

QString Devices::device_type_to_string(DeviceType t)
{
    switch (t)
    {
    case DAP_USB_HID:
        return "DAP_USB_HID";
    case DAP_USB_Bulk:
        return "DAP_USB_Bulk";
    }

    return QString::asprintf("UnknownDeviceType:%d", t);
}

Devices::DeviceType Devices::string_to_device_type(QString str)
{
    if (str == "DAP_USB_HID")
        return DAP_USB_HID;

    if (str == "DAP_USB_Bulk")
        return DAP_USB_Bulk;

    return UnknownDeviceType;
}

bool Devices::equal(const Devices &device)
{
    bool result = false;

    if (type() != device.type())
        return false;

    switch (type())
    {
    case DAP_USB_HID:
    {
        result = ((DAP_HID *)this)->equal(device);

        qDebug("[Devices] DAP_HID equal result: %d", result);

        return result;
    }
    break;
    case DAP_USB_Bulk:
    {
        result = ((CMSIS_DAP_V2 *)this)->equal(device);

        qDebug("[Devices] DAP_Bulk equal result: %d", result);

        return result;
    }
    break;
    }

    qDebug("[Devices] equal");

    return result;
}

bool Devices::device_list_compare(DeviceList now_list, DeviceList prev_list, DeviceList *added_list, DeviceList *fixed_list, DeviceList *removed_list)
{
    bool is_changed = false;

    DeviceList now_list_bak;
    DeviceList prev_list_bak;
    DeviceList fixed_list_bak;
    DeviceList tmp_list;

    fixed_list_bak.clear();

    now_list_bak.clear();
    now_list_bak.append(now_list);

    prev_list_bak.clear();
    prev_list_bak.append(prev_list);

    // qDebug("[Devices] compare added_list");
    now_list_bak.remove_device(prev_list, &fixed_list_bak);

    // qDebug("[Devices] compare removed_list");
    prev_list_bak.remove_device(now_list, &fixed_list_bak);

    tmp_list.clear();

    // qDebug("[Devices] fixed_list deduplication");
    while (fixed_list_bak.count())
    {
        Devices *tmp_dev = fixed_list_bak.takeFirst();
        bool is_found = false;

        for (int i = 0; i < fixed_list_bak.count(); i++)
        {

            if (tmp_dev->equal(*fixed_list_bak[i]))
            {
                is_found = true;
                break;
            }
        }

        if (is_found == false)
            tmp_list.append(tmp_dev);
    }

    fixed_list_bak.clear();
    fixed_list_bak.append(tmp_list);

    if (prev_list.count() != now_list.count())
        is_changed = true;

    if (now_list_bak.count())
    {
        is_changed = true;
    }

    if (prev_list_bak.count())
    {
        is_changed = true;
    }

    if (is_changed)
    {
        qDebug("[Devices] added:%d fixed:%d removes:%d", now_list_bak.count(), fixed_list_bak.count(), prev_list_bak.count());
    }

    // if (now_list_bak.count() || prev_list_bak.count())
    // {
    //     is_changed = true;
    // }

    if (added_list != NULL)
    {
        added_list->clear();
        added_list->append(now_list_bak);
    }

    if (fixed_list != NULL)
    {
        fixed_list->clear();
        fixed_list->append(fixed_list_bak);
    }

    if (removed_list != NULL)
    {
        removed_list->clear();
        removed_list->append(prev_list_bak);
    }

    return is_changed;
}
