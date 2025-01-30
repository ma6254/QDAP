#include "win_hotplug_notify.h"

WinHotplugNotify::WinHotplugNotify(QObject *parent) : QObject{parent}
{
    HINSTANCE hi = ::GetModuleHandleW(nullptr);

    // 创建隐藏窗口以接收设备通知
    WNDCLASS wc;
    memset(&wc, 0, sizeof(WNDCLASSW));
    wc.lpfnWndProc = windowMessageProcess;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hi;
    wc.lpszClassName = get_window_class_name().toStdString().c_str();
    RegisterClass(&wc);

    hwnd = CreateWindowW(get_window_class_name().toStdWString().c_str(), // classname
                         get_window_class_name().toStdWString().c_str(), // window name
                         0,                                              // style
                         0,                                              // x
                         0,                                              // y
                         0,                                              // width
                         0,                                              // height
                         0,                                              // parent
                         0,                                              // menu handle
                         hi,                                             // application
                         0);                                             // windows creation data.

    if (hwnd == NULL)
    {
        qDebug("createMessageWindow error %d", (int)GetLastError());
        return;
    }

    // 初始化 DEV_BROADCAST_DEVICEINTERFACE 数据结构
    DEV_BROADCAST_DEVICEINTERFACE_W filter_data;
    memset(&filter_data, 0, sizeof(DEV_BROADCAST_DEVICEINTERFACE_W));
    filter_data.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE_W);
    filter_data.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;

    QUuid uuid = GUID_DEVINTERFACE_USB_DEVICE;

    filter_data.dbcc_classguid = uuid;
    usb_device_notification_handle = RegisterDeviceNotificationW(hwnd, &filter_data, DEVICE_NOTIFY_WINDOW_HANDLE);
    if (usb_device_notification_handle == NULL)
    {
        qDebug("[win_hotplug_notify] RegisterDeviceNotification error");
    }

    SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)this);
}

WinHotplugNotify::~WinHotplugNotify()
{
    if (hwnd)
    {
        DestroyWindow(hwnd);
        hwnd = NULL;

        if (usb_device_notification_handle)
        {
            UnregisterDeviceNotification(usb_device_notification_handle);
            usb_device_notification_handle = NULL;
        }
    }

    UnregisterClassW(
        reinterpret_cast<const wchar_t *>(get_window_class_name().toStdString().c_str()),
        GetModuleHandleW(nullptr));
}

LRESULT CALLBACK WinHotplugNotify::windowMessageProcess(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // qDebug("[WinHotplugNotify] windowMessageProcess");

    static int i = 0;

    if (message == WM_DEVICECHANGE)
    {
        do
        {
            // 设备可用事件
            const bool is_add = wParam == DBT_DEVICEARRIVAL;
            // 设备移除事件
            const bool is_remove = wParam == DBT_DEVICEREMOVECOMPLETE;
            if (!is_add && !is_remove)
                break;

            // 过滤 device interface class 以外类型的消息
            DEV_BROADCAST_HDR *broadcast = reinterpret_cast<DEV_BROADCAST_HDR *>(lParam);
            if (!broadcast || broadcast->dbch_devicetype != DBT_DEVTYP_DEVICEINTERFACE)
                break;

            // 获取 SetWindowLongPtrW 设置的对象
            WinHotplugNotify *data = reinterpret_cast<WinHotplugNotify *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
            if (!data)
                break;

            // 过滤不监听的设备类型
            DEV_BROADCAST_DEVICEINTERFACE *device_interface = reinterpret_cast<DEV_BROADCAST_DEVICEINTERFACE *>(broadcast);
            QUuid uid(device_interface->dbcc_classguid);
            if (uid != GUID_DEVINTERFACE_USB_DEVICE)
                break;

            QString device_name;
            if (device_interface->dbcc_name)
            {
#ifdef UNICODE
                device_name = QString::fromWCharArray(device_interface->dbcc_name);
#else
                device_name = QString(device_interface->dbcc_name);
#endif
            }

            i++;
            // qDebug("[WinHotplugNotify] windowMessageProcess WM_DEVICECHANGE %s %d", qPrintable(device_name), i);

            Event e;

            if (is_add)
            {
                e = EVENT_DEVICE_ARRIVED;
            }
            else if (is_remove)
            {
                e = EVENT_DEVICE_REMOVED;
            }

            emit data->device_change();
        } while (false);
    }

    return DefWindowProcW(hwnd, message, wParam, lParam);
}
