#ifndef ENUM_WRITER_LIST_H
#define ENUM_WRITER_LIST_H

#include <QWidget>
#include <QDialog>
#include <QVBoxLayout>
#include <QComboBox>
#include <QGroupBox>
#include "devices.h"
#include "enum_writer_list.h"
#include "enum_dap.h"

namespace Ui
{
    class enum_writer_list;
}

class enum_writer_list : public QDialog
{
    Q_OBJECT

public:
    explicit enum_writer_list(QWidget *parent = nullptr);
    ~enum_writer_list();

    void set_auto_refresh(bool auto_refresh);

    void dap_hid_list_clear(void);
    int currentIndex(void);
    void setCurrentIndex(int n);

    Devices *current_device();

    void set_collapse_icon(uint32_t index);
    void set_btn_manual_refresh_enabled(bool enabled);

signals:
    void refresh_enum_devides();

public slots:
    void cb_device_changed(DeviceList dev_list, bool changed);
    void cb_collapse_currentChanged(int index);
    void cb_btn_ok(void);
    void cb_refresh_enum_devices();

private:
    Ui::enum_writer_list *ui;

    DeviceList device_list;
    int current_device_index;
    Devices *tmp_current_device;

    // QWidget *widget_dap;
    // QVBoxLayout *vbox_dap;
    // QComboBox *dd_dap;

    EnumDAP *enum_dap;

    QIcon icon_arrow_right;
    QIcon icon_arrow_down;

    bool is_auto_refresh_enum_devices;

    QStringList collapse_title_bak_list;
};

#endif // ENUM_WRITER_LIST_H
