#ifndef CHIP_SELECTER_H
#define CHIP_SELECTER_H

#include <QDialog>

namespace Ui
{
    class ChipSelecter;
}

class ChipSelecter : public QDialog
{
    Q_OBJECT

public:
    explicit ChipSelecter(QWidget *parent = nullptr);
    ~ChipSelecter();

public slots:
    void cb_combobox_manufacturer(int index);

private:
    Ui::ChipSelecter *ui;

    QList<QList<QString>> chip_name_list;
};

#endif // CHIP_SELECTER_H
