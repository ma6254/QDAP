#ifndef ENUM_DAP_H
#define ENUM_DAP_H

#include <QWidget>
#include <QComboBox>
#include "dap.h"

namespace Ui
{
    class EnumDAP;
}

class EnumDAP : public QWidget
{
    Q_OBJECT

public:
    explicit EnumDAP(QWidget *parent = nullptr);
    ~EnumDAP();

    // QComboBox *dd_dev();

    void dd_dev_clear();
    void dd_dev_append(CMSIS_DAP_Base *dap);
    void set_info(CMSIS_DAP_Base *dap);
    void set_info(int index);
    void set_info_empty();

    int current_index();
    void set_current_index(int index);
    Devices *current_device();
    int set_current_device(Devices device);

    int count();

public slots:
    // void cb_devices_changed(QList<CMSIS_DAP_Base *> dev_list);
    void cb_dd_dev_currentIndexChanged(int index);

private:
    Ui::EnumDAP *ui;

    QList<CMSIS_DAP_Base *> device_list;
};

#endif // ENUM_DAP_H
