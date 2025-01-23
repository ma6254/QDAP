#ifndef INPUT_BOX_DIALOG_H
#define INPUT_BOX_DIALOG_H

#include <QDialog>
#include <QValidator>
#include "ui_input_box_dialog.h"

namespace Ui
{
    class input_box_dialog;
}

class InputBoxDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InputBoxDialog(QDialog *parent = nullptr);
    ~InputBoxDialog();

    void set_description_text(const QString str)
    {
        ui->label_description->setText(str);
    }

    void set_default_value(const QString str) { ui->lineEdit_input->setText(str); }
    QString get_value() { return ui->lineEdit_input->text(); }

    void set_validator(QValidator *v);

public slots:
    void lineEdit_input_textEdited(QString text);

private:
    Ui::input_box_dialog *ui;

    QValidator *validator;

    void set_validate_status(bool valid);
};

#endif // INPUT_BOX_DIALOG_H
