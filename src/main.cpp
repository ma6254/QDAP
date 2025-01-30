#include "mainwindow.h"
#include "devices.h"
#include "device_list.h"

#include <QApplication>
#include <QDebug>
#include <libusb.h>

libusb_context *g_libusb_context = NULL;

#if !_WIN32
libusb_hotplug_callback_handle g_libusb_hotplug_callback_handle;

static int libusb_hotplug_callback(libusb_context *ctx, libusb_device *device, libusb_hotplug_event event, void *user_data)
{
    if (event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED)
    {
        qDebug("Device inserted");
    }
    else if (event == LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT)
    {
        qDebug("Device removed");
    }
    return 0;
}
#endif // !_WIN32

int main(int argc, char *argv[])
{
    int rc = 0;

    rc = libusb_init(&g_libusb_context);
    if (rc != 0)
    {
        qDebug("[amin] libusb_init fail %d", rc);
        libusb_exit(g_libusb_context);
        return -1;
    }

#if !_WIN32

    rc = libusb_hotplug_register_callback(g_libusb_context,
                                          LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT,
                                          LIBUSB_HOTPLUG_NO_FLAGS,
                                          LIBUSB_HOTPLUG_MATCH_ANY,
                                          LIBUSB_HOTPLUG_MATCH_ANY,
                                          LIBUSB_HOTPLUG_MATCH_ANY,
                                          libusb_hotplug_callback,
                                          NULL,
                                          &g_libusb_hotplug_callback_handle);

    if (rc != LIBUSB_SUCCESS)
    {
        fprintf(stderr, "Failed to register hotplug callback\n");
        return 1;
    }

#endif // !_WIN32

    qRegisterMetaType<uint32_t>("uint32_t");
    qRegisterMetaType<ProgramWorker::ChipOp>("ProgramWorker::ChipOp");
    // qRegisterMetaType<Devices>("Devices");
    qRegisterMetaType<Devices *>("Devices *");
    qRegisterMetaType<DeviceList>("DeviceList");

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    a.exec();

#if !_WIN32
    libusb_hotplug_deregister_callback(g_libusb_context, g_libusb_hotplug_callback_handle);
#endif // !_WIN32
    libusb_exit(g_libusb_context);
    return 0;
}
