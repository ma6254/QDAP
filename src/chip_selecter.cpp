#include "chip_selecter.h"
#include "ui_chip_selecter.h"

ChipSelecter::ChipSelecter(QWidget *parent) : QDialog(parent),
                                              ui(new Ui::ChipSelecter)
{
    ui->setupUi(this);

    connect(ui->comboBox_manufacturer, SIGNAL(currentIndexChanged(int)), this, SLOT(cb_combobox_manufacturer(int)));

    QList<QString> *st_chip_list = new QList<QString>();
    st_chip_list->push_back(QString("STM32F0"));
    st_chip_list->push_back(QString("STM32F1"));
    st_chip_list->push_back(QString("STM32F4"));
    chip_name_list.push_back(*st_chip_list);

    QList<QString> *puya_chip_list = new QList<QString>();
    puya_chip_list->push_back(QString("PY32F002"));
    puya_chip_list->push_back(QString("PY32F003"));
    puya_chip_list->push_back(QString("PY32F030"));
    chip_name_list.push_back(*puya_chip_list);
}

ChipSelecter::~ChipSelecter()
{
    delete ui;
}

void ChipSelecter::cb_combobox_manufacturer(int index)
{
    ui->comboBox_series->clear();

    QList<QString> series_list = chip_name_list.at(index);

    for (int i = 0; i < series_list.count(); i++)
    {
        ui->comboBox_series->addItem(series_list.at(i));
    }

    ui->comboBox_series->setCurrentIndex(0);
}
