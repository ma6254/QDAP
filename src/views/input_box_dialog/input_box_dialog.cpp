#include "input_box_dialog.h"

InputBoxDialog::InputBoxDialog(QDialog *parent) : QDialog(parent),
                                                  ui(new Ui::input_box_dialog)
{
    ui->setupUi(this);
    setWindowTitle("QDAP 参数输入窗口");
    validator = NULL;
    ui->label_status->setVisible(false);
}

InputBoxDialog::~InputBoxDialog()
{
    delete ui;
}

void InputBoxDialog::lineEdit_input_textEdited(QString text)
{
    // qDebug("[InputBoxDialog] textEdited %s", qPrintable(text));
}

void InputBoxDialog::set_validate_status(bool valid)
{
    ui->btn_ok->setEnabled(valid);

    if (valid)
    {
        ui->label_status->setText("✅");
    }
    else
    {
        ui->label_status->setText("❌");
    }
}

void InputBoxDialog::set_validator(QValidator *v)
{
    if (v == NULL)
    {
        ui->label_status->setVisible(false);
        validator = NULL;
        disconnect(ui->lineEdit_input, SIGNAL(textEdited(QString)), this, SLOT(lineEdit_input_textEdited(QString)));
        return;
    }

    ui->label_status->setVisible(true);

    int pos = 0;
    QString str = get_value();
    QValidator::State state = validator->validate(str, pos);
    set_validate_status(state == QValidator::Acceptable);
    connect(ui->lineEdit_input, SIGNAL(textEdited(QString)), this, SLOT(lineEdit_input_textEdited(QString)));
}
