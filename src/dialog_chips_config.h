#ifndef DIALOG_CHIPS_CONFIG_H
#define DIALOG_CHIPS_CONFIG_H

#include <QDialog>

namespace Ui
{
    class DialogChipsConfig;
}

class DialogChipsConfig : public QDialog
{
    Q_OBJECT

public:
    explicit DialogChipsConfig(QWidget *parent = nullptr);
    ~DialogChipsConfig();

    QString get_chips_url();
    void set_chips_url(QString url);

public slots:
    void cb_btn_ok();

private:
    Ui::DialogChipsConfig *ui;
};

#endif // DIALOG_CHIPS_CONFIG_H
 