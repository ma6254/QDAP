#ifndef ENUM_WRITER_LIST_H
#define ENUM_WRITER_LIST_H

#include <QDialog>
#include "dap_hid.h"

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

    void dap_hid_list_clear(void);

public slots:
    void cb_device_changed(QList<DAP_HID *> dev_list);

private:
    Ui::enum_writer_list *ui;
};

#endif // ENUM_WRITER_LIST_H
