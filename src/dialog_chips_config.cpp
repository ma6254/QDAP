#include "dialog_chips_config.h"
#include "ui_dialog_chips_config.h"

static const QString default_chips_url = "https://github.com/ma6254/qdap_chips";

DialogChipsConfig::DialogChipsConfig(QWidget *parent)
    : QDialog(parent), ui(new Ui::DialogChipsConfig)
{
    ui->setupUi(this);

    connect(ui->btn_ok, SIGNAL(clicked()), this, SLOT(cb_btn_ok()));
}

DialogChipsConfig::~DialogChipsConfig()
{
    delete ui;
}

void DialogChipsConfig::cb_btn_ok()
{
    qDebug("[DialogChipsConfig] cb_btn_ok");

    emit accept();
}

QString DialogChipsConfig::get_chips_url()
{
    QString tmp_str = ui->lineEdit_chips_url->text();

    if (tmp_str.isEmpty())
    {
        ui->lineEdit_chips_url->setText(default_chips_url);
    }

    return ui->lineEdit_chips_url->text();
}

void DialogChipsConfig::set_chips_url(QString url)
{
    if (url.isEmpty())
    {
        url = default_chips_url;
    }

    ui->lineEdit_chips_url->setText(url);
}
