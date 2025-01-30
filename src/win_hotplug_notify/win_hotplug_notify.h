#ifndef WIN_HOTPLUG_NOTIFY_H
#define WIN_HOTPLUG_NOTIFY_H

#include <windows.h>
#include <Dbt.h>
#include <devguid.h>
#include <initguid.h>
#include <usbiodef.h>
#include <hidclass.h>

#include <QObject>
#include <QUuid>
#include <QSharedPointer>

class WinHotplugNotify : public QObject
{
    Q_OBJECT

public:
    WinHotplugNotify(QObject *parent = nullptr);
    ~WinHotplugNotify();

    enum Event
    {
        EVENT_DEVICE_ARRIVED = 1,
        EVENT_DEVICE_REMOVED = 2,
    };
    Q_ENUM(Event)

    QString get_window_class_name() const
    {
        return QString("QDAP_win_win_hotplug_notify_window %1").arg(QString::number((uint64_t)hwnd, 16));
    }

signals:
    void device_change();

private:
    static LRESULT CALLBACK windowMessageProcess(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    HWND hwnd;

    HDEVNOTIFY usb_device_notification_handle;
};

#endif // WIN_HOTPLUG_NOTIFY_H
