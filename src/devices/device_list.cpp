#include "device_list.h"

DeviceList::DeviceList()
{
}

DeviceList::~DeviceList()
{
}

// bool DeviceList::remove_device(Devices device, DeviceList *fixed_list)
// {
//     // qDebug("[DeviceList] remove_device one this_len:%d", count());

//     if (count() == 0)
//     {
//         // qDebug("[DeviceList] remove_device one this_len:0 result:false");
//         return false;
//     }

//     bool is_found = false;
//     int i = 0;

//     while (i != count())
//     {

//         if (device.equal(at(i)))
//         {
//             // removeAt(i);
//             Devices *fixed_device = takeAt(i);

//             if (fixed_list)
//                 fixed_list->append(fixed_device);

//             is_found = true;

//             continue;
//         }

//         i++;
//     }

//     // qDebug("[DeviceList] remove_device one result:%d", is_found);

//     return is_found;
// }

bool DeviceList::remove_device(Devices *p_device, DeviceList *fixed_list)
{
    // if (p_device == NULL)
    //     return false;

    // return remove_device(*p_device, fixed_list);

    // qDebug("[DeviceList] remove_device one this_len:%d", count());

    if (count() == 0)
    {
        // qDebug("[DeviceList] remove_device one this_len:0 result:false");
        return false;
    }

    bool is_found = false;
    int i = 0;

    while (i != count())
    {

        if (p_device->equal(*at(i)))
        {
            // removeAt(i);
            Devices *fixed_device = takeAt(i);

            if (fixed_list)
                fixed_list->append(fixed_device);

            is_found = true;

            continue;
        }

        i++;
    }

    // qDebug("[DeviceList] remove_device one result:%d", is_found);

    return is_found;
}

bool DeviceList::remove_device(DeviceList device_list, DeviceList *fixed_list)
{

    // qDebug("[DeviceList] remove_device this_len:%d list_len:%d", count(), device_list.count());

    for (int i = 0; i < device_list.count(); i++)
    {
        // qDebug("[DeviceList] remove_device index:%d", i);

        remove_device(device_list[i], fixed_list);
    }

    // qDebug("[DeviceList] remove_device remaining:%d", count());

    if (count() == 0)
        return false;
    else
        return true;
}

DeviceList DeviceList::filt(Devices::DeviceType device_type)
{
    DeviceList result_list;
    result_list.clear();

    for (int i = 0; i < count(); i++)
    {
        Devices *tmp_dev = this->at(i);

        if (tmp_dev->type() == device_type)
        {
            result_list.append(tmp_dev);
        }
    }

    return result_list;
}

bool DeviceList::contains(const Devices &value) const
{
    for (int i = 0; i < count(); i++)
    {
        if (this->at(i)->equal(value))
        {
            return true;
        }
    }

    return false;
}

void DeviceList::release_all()
{

    // qDebug("[DeviceList] release_all count:%d", count());

    for (int i = 0; i < count(); i++)
    {
        Devices *tmp_dev = this->at(i);

        if (tmp_dev == NULL)
            continue;

        delete tmp_dev;
    }

    this->clear();

    // qDebug("[DeviceList] release_all done");
}
