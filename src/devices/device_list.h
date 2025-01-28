#ifndef DEVICE_LIST_H
#define DEVICE_LIST_H

#include <QList>
#include <QThread>
#include <QDebug>
#include "devices.h"

class Devices;
class DeviceList;

/*******************************************************************************
 * @brief 设备列表
 ******************************************************************************/
class DeviceList : public QList<Devices *>
{
public:
    DeviceList();
    ~DeviceList();

    // bool remove_device(Devices device, DeviceList *fixed_list = NULL);
    bool remove_device(Devices *p_device, DeviceList *fixed_list = NULL);
    bool remove_device(DeviceList device_list, DeviceList *fixed_list = NULL);

    DeviceList filt(Devices::DeviceType device_type);

    bool contains(const Devices &value) const;

    void release_all();

private:
    QStringList aaa;
};

#endif // DEVICE_LIST_H
