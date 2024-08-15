#include "chip_selecter.h"
#include "ui_chip_selecter.h"

const QString config_dir_path = QDir::homePath() + QDir::separator() + ".qdap";

ChipSelecter::ChipSelecter(QWidget *parent) : QDialog(parent),
                                              ui(new Ui::ChipSelecter)
{
    ui->setupUi(this);

    // set_dd_vendor_connect();
    // set_dd_series_connect();
    // set_dd_chip_connect();

    // QList<QString> *st_chip_list = new QList<QString>();
    // st_chip_list->push_back(QString("STM32F0"));
    // st_chip_list->push_back(QString("STM32F1"));
    // st_chip_list->push_back(QString("STM32F4"));
    // chip_name_list.push_back(*st_chip_list);

    // QList<QString> *puya_chip_list = new QList<QString>();
    // puya_chip_list->push_back(QString("PY32F002"));
    // puya_chip_list->push_back(QString("PY32F003"));
    // puya_chip_list->push_back(QString("PY32F030"));
    // chip_name_list.push_back(*puya_chip_list);

    connect(ui->btn_ok, SIGNAL(clicked()), this, SLOT(cb_btn_ok()));

    chip_core_homepage_map.insert("Cortex-M0", "https://developer.arm.com/Processors/Cortex-M0");
    chip_core_homepage_map.insert("Cortex-M0+", "https://developer.arm.com/Processors/Cortex-M0-Plus");
    chip_core_homepage_map.insert("Cortex-M3", "https://developer.arm.com/Processors/Cortex-M3");
    chip_core_homepage_map.insert("Cortex-M4", "https://developer.arm.com/Processors/Cortex-M4");
    chip_core_homepage_map.insert("Cortex-M33", "https://developer.arm.com/Processors/Cortex-M33");

    load_chips();
}

ChipSelecter::~ChipSelecter()
{
    delete ui;
}

void ChipSelecter::log_clear()
{
    ui->label_log->clear();
}

void ChipSelecter::log_output()
{
    // qDebug("log_vendor: %s", log_vendor.toUtf8().constData());
    // qDebug("log_series: %s", log_series.toUtf8().constData());
    // qDebug("log_chip: %s", log_chip.toUtf8().constData());

    QString log_str;
    log_str.append(log_vendor);
    log_str.append(log_series);
    log_str.append(log_chip);

    ui->label_log->setText(log_str);
}

void ChipSelecter::switch_vendor(uint32_t index)
{
    set_dd_vendor_disconnect();
    ui->comboBox_vendor->setCurrentIndex(index);
    set_dd_vendor_connect();

    set_dd_series_disconnect();
    ui->comboBox_series->clear();
    ui->comboBox_series->setEnabled(true);
    set_dd_series_connect();

    log_vendor.clear();
    log_series.clear();
    log_chip.clear();
    error_info.clear();
    ui->btn_ok->setEnabled(false);

    // qDebug("[ChipSelecter] switch_vendor %d", index);

    if (index >= vendor_list.count())
    {
        return;
    }

    if (vendor_list[index] == NULL)
    {
        set_dd_series_disconnect();
        ui->comboBox_series->addItem("[无效]");
        ui->comboBox_series->setDisabled(true);
        set_dd_series_connect();

        set_dd_chip_disconnect();
        ui->comboBox_chip->clear();
        ui->comboBox_chip->addItem("[无效]");
        ui->comboBox_chip->setDisabled(true);
        set_dd_chip_connect();

        log_vendor += QString("厂商: [无效]\r\n");
        log_output();

        error_info += "[厂商无效]";
        ui->label_error->setText(error_info.join(' '));
        ui->label_error->setVisible(true);

        return;
    }

    current_vendor_index = index;

    set_dd_series_disconnect();
    ui->comboBox_series->addItems(current_vendor()->series_name_list());
    set_dd_series_connect();

    if (current_vendor()->homepage.isEmpty() == false)
    {
        log_vendor += QString("厂商: [%1](%2)\r\n\r\n").arg(current_vendor()->name, current_vendor()->homepage);
    }
    else
    {
        log_vendor += QString("厂商: %1\r\n\r\n").arg(current_vendor()->name);
    }

    ui->btn_ok->setEnabled(true);

    log_output();
    switch_series(0);
}

void ChipSelecter::switch_series(uint32_t index)
{
    set_dd_series_disconnect();
    ui->comboBox_series->setCurrentIndex(index);
    set_dd_series_connect();

    set_dd_chip_disconnect();
    ui->comboBox_chip->clear();
    ui->comboBox_chip->setEnabled(true);
    set_dd_chip_connect();
    log_series.clear();
    log_chip.clear();

    error_info.clear();

    // qDebug("[ChipSelecter] switch_series %d", index);

    if (current_vendor_index >= vendor_list.count())
    {
        return;
    }

    if (index >= vendor_list[current_vendor_index]->series_list.count())
    {
        return;
    }

    if (vendor_list[current_vendor_index]->series_list[index] == NULL)
    {
        set_dd_chip_disconnect();
        ui->comboBox_chip->addItem("[无效]");
        ui->comboBox_chip->setDisabled(true);
        set_dd_chip_connect();

        log_series += QString("系列: [无效]\r\n\r\n");
        log_output();
        ui->btn_ok->setEnabled(false);

        error_info += "[系列无效]";
        ui->label_error->setText(error_info.join(' '));
        ui->label_error->setVisible(true);

        return;
    }

    current_series_index = index;

    if (vendor_list[current_vendor_index]->series_list[index]->chip_count() == 0)
    {
        set_dd_chip_disconnect();
        ui->comboBox_chip->clear();
        ui->comboBox_chip->addItem("[无效]");
        ui->comboBox_chip->setDisabled(true);
        set_dd_chip_connect();

        log_chip += QString("芯片: [无效]\r\n\r\n");
        log_output();
        ui->btn_ok->setEnabled(false);

        error_info += "[系列无效]";
        ui->label_error->setText(error_info.join(' '));
        ui->label_error->setVisible(true);

        return;
    }

    set_dd_chip_disconnect();
    ui->comboBox_chip->addItems(current_series()->chips_name_list());
    set_dd_chip_connect();
    if (current_series()->homepage.isEmpty() == false)
    {
        log_vendor += QString("系列: [%1](%2)\r\n\r\n").arg(current_series()->name, current_series()->homepage);
        // log_vendor += QString(R""(系列: <a href="%1">%2</a>)"").arg(current_series()->homepage, current_series()->name);
        log_vendor += QString("\r\n\r\n");
    }
    else
    {
        log_vendor += QString("系列: %1\r\n\r\n").arg(current_series()->name);
    }

    log_output();
    switch_chip(0);
}

void ChipSelecter::switch_chip(uint32_t index)
{
    set_dd_chip_disconnect();
    ui->comboBox_chip->setCurrentIndex(index);
    set_dd_chip_connect();

    log_chip.clear();
    ui->label_error->setVisible(false);
    ui->btn_ok->setEnabled(false);
    error_info.clear();

    // qDebug("[ChipSelecter] switch_chip %d", index);

    if (current_vendor_index >= vendor_list.count())
    {
        ui->btn_ok->setEnabled(false);
        return;
    }

    if (current_series_index >= current_vendor()->series_list.count())
    {
        ui->btn_ok->setEnabled(false);
        return;
    }

    if (index >= current_series()->chip_info_list.count())
    {
        ui->btn_ok->setEnabled(false);
        return;
    }

    if (current_series()->chip_info_list[index] == NULL)
    {
        set_dd_chip_disconnect();
        ui->comboBox_chip->addItem("[无效]");
        ui->comboBox_chip->setDisabled(true);
        set_dd_chip_connect();

        ui->label_error->setText("[无效]");
        ui->label_error->setVisible(true);
        ui->btn_ok->setEnabled(false);

        log_chip += QString("芯片: [无效]\r\n\r\n");
        log_output();
        return;
    }

    current_chip_index = index;

    log_chip += QString("芯片: %1\r\n\r\n").arg(current_chip_info()->name);

    QString chip_core_str = chip_core();

    if (chip_core_homepage_map.keys().contains(chip_core_str))
    {
        log_chip += QString("内核: [%1](%2)\r\n\r\n").arg(chip_core_str, chip_core_homepage_map[chip_core_str]);
    }
    else
    {
        log_chip += QString("内核: %1\r\n\r\n").arg(chip_core_str);
    }

    log_chip += QString("RAM容量: %1\r\n\r\n").arg(chip_ram_size());
    log_chip += QString("Flash容量: %1\r\n\r\n").arg(chip_flash_size());

    QString tmp_str = chip_algo();
    if (tmp_str.isEmpty())
    {
        log_chip += QString("算法: [❌缺失]\r\n\r\n");

        error_info += QString("[算法缺失]");
    }
    else
    {
        log_chip += QString("算法: %1\r\n\r\n").arg(chip_algo());
    }

    log_output();

    if (error_info.count())
    {
        ui->label_error->setText(error_info.join(' '));
        ui->label_error->setVisible(true);

        return;
    }

    ui->btn_ok->setEnabled(true);

    // QString log_text;
    // log_text += QString("厂商: %1\r\n").arg(current_vendor()->name.toUtf8().constData());
    // log_text += QString("系列/家族: %1\r\n").arg(current_series()->name.toUtf8().constData());
    // // log_text += QString("芯片型号: %1\r\n").arg(current_chip_info()->name.toUtf8().constData());
    // ui->label_log->setText(log_text);
}

bool ChipSelecter::switch_vendor(QString vendor_name)
{
    for (uint32_t i = 0; i < vendor_list.count(); i++)
    {
        if (vendor_list[i] == NULL)
            continue;

        if (vendor_list[i]->name == vendor_name)
        {
            current_vendor_index = i;
            switch_vendor(i);
            return true;
        }
    }

    return false;
}

bool ChipSelecter::switch_series(QString series_name)
{
    QList<Series *> series_list = current_vendor()->series_list;

    for (uint32_t i = 0; i < series_list.count(); i++)
    {
        if (series_list[i] == NULL)
            continue;

        if (series_list[i]->name == series_name)
        {
            current_series_index = i;
            switch_series(i);
            return true;
        }
    }

    return false;
}

bool ChipSelecter::switch_chip(QString chip_name)
{
    QList<ChipInfo *> chip_list = current_series()->chip_info_list;

    for (uint32_t i = 0; i < chip_list.count(); i++)
    {
        if (chip_list[i] == NULL)
            continue;

        if (chip_list[i]->name == chip_name)
        {
            current_chip_index = i;
            switch_chip(i);
            return true;
        }
    }

    return false;
}

bool ChipSelecter::switch_chip(QString vendor_name, QString series_name, QString chip_name)
{
    bool ok;

    ok = switch_vendor(vendor_name);
    if (ok == false)
    {
        return false;
    }

    ok = switch_series(series_name);
    if (ok == false)
    {
        return false;
    }

    ok = switch_chip(chip_name);
    if (ok == false)
    {
        return false;
    }

    return true;
}

QString ChipSelecter::vendor_name()
{
    return current_vendor()->name;
}

QString ChipSelecter::vendor_homepage()
{
    return current_vendor()->homepage;
}

QString ChipSelecter::series_name()
{
    return current_series()->name;
}

QString ChipSelecter::series_homepage()
{
    return current_series()->homepage;
}

QString ChipSelecter::chip_name()
{
    return current_chip_info()->name;
}

void ChipSelecter::load_chips(QString chips_dir_path)
{
    if (chips_dir_path.isEmpty())
    {
        chips_dir_path = config_dir_path + QDir::separator() + "chips";
    }

    QDir dir_chips(chips_dir_path);
    if (!dir_chips.exists())
    {
        qDebug("[chips] chips_dir_path is not exists");
        return;
    }

    dir_chips.setNameFilters(QStringList("*.yml"));
    dir_chips.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);

    QStringList file_list = dir_chips.entryList();
    set_dd_vendor_disconnect();
    ui->comboBox_vendor->clear();
    set_dd_vendor_connect();
    vendor_list.clear();

    set_dd_vendor_disconnect();
    for (uint32_t i = 0; i < file_list.count(); i++)
    {
        ui->comboBox_vendor->addItem(QString("%1 | [❌无效] %2").arg(i).arg(file_list[i]));
        vendor_list.push_back(NULL);
    }
    set_dd_vendor_connect();

    for (uint32_t i = 0; i < file_list.count(); i++)
    {
        qDebug("[load_chips] %d %s", i, qPrintable(file_list.at(i)));

        Vendor *tmp_vendor = new Vendor();

        int err = load_chip_vendor(chips_dir_path + QDir::separator() + file_list.at(i), tmp_vendor);
        if (err < 0)
        {
            delete tmp_vendor;
            continue;
        }

        ui->comboBox_vendor->setItemText(i, QString("%1 | %2").arg(i).arg(tmp_vendor->name));
        vendor_list[i] = tmp_vendor;
    }

    switch_vendor(0);
}

int ChipSelecter::load_chip_vendor(QString vendor_file_path, Vendor *vendor)
{
    QFile file;
    QByteArray file_buf;
    QString name;
    bool ok;
    int err;

    file.setFileName(vendor_file_path);
    if (file.exists() == false)
        return -1;

    ok = file.open(QIODevice::ReadOnly);
    file_buf = file.readAll();
    file.close();

    YAML::Node node = YAML::Load(file_buf.constData());

    if (node.IsMap() == false)
        return -1;

    YAML::Node tmp_node;

    // Vendor vendor(node);

    err = vendor->from_node(node);
    if (err < 0)
        return -1;

    // // 厂商名称
    // tmp_node = node["name"];
    // if (tmp_node.IsScalar() == false)
    //     return -1;
    // name = QString(tmp_node.as<std::string>().c_str());
    // vendor->name = name;

    // ui->comboBox_vendor->addItem(name);

    // QList<QString> *cur_series_list = new QList<QString>();
    // QList<QList<QString>> *cur_series_chip_list = new QList<QList<QString>>;

    // // 系列
    // YAML::Node node_series;
    // node_series = node["series"];
    // if (node_series.IsSequence() == false)
    // {
    //     series_name_list.push_back(*cur_series_list);
    //     return;
    // }

    // for (int i = 0; i < node_series.size(); i++)
    // {
    //     tmp_node = node_series[i]["name"];
    //     if (tmp_node.IsScalar() == false)
    //         continue;
    //     name = QString(tmp_node.as<std::string>().c_str());
    //     qDebug("    series: %d %s", i, qPrintable(name));
    //     cur_series_list->push_back(name);

    //     QList<QString> *cur_chip_list = new QList<QString>();

    //     YAML::Node node_chips = node_series[i]["chips"];
    //     if (node_chips.IsSequence() == false)
    //     {
    //         cur_series_chip_list->push_back(*cur_chip_list);
    //         continue;
    //     }

    //     for (int i = 0; i < node_chips.size(); i++)
    //     {
    //         tmp_node = node_chips[i]["name"];
    //         if (tmp_node.IsScalar() == false)
    //             continue;

    //         name = QString(tmp_node.as<std::string>().c_str());

    //         qDebug("        %s", qPrintable(name));

    //         cur_chip_list->push_back(name);
    //     }

    //     cur_series_chip_list->push_back(*cur_chip_list);
    // }

    // series_name_list.push_back(*cur_series_list);
    // chip_name_list.push_back(*cur_series_chip_list);

    return 0;
}

void ChipSelecter::cb_combobox_vendor(int index)
{
    // qDebug("[ChipSelecter] cb_combobox_vendor %d", index);

    if (index < 0)
        return;

    switch_vendor(index);
}

void ChipSelecter::cb_combobox_series(int index)
{
    // qDebug("[ChipSelecter] cb_combobox_vendor %d", index);

    if (index < 0)
        return;

    switch_series(index);
}

void ChipSelecter::cb_combobox_chip(int index)
{
    if (index < 0)
        return;

    switch_chip(index);
}

void ChipSelecter::cb_btn_ok()
{
    qDebug("[ChipSelecter] cb_btn_ok");

    emit accept();
}

void ChipSelecter::set_dd_vendor_connect()
{
    conn_dd_vendor = connect(ui->comboBox_vendor, SIGNAL(currentIndexChanged(int)), this, SLOT(cb_combobox_vendor(int)));
}

void ChipSelecter::set_dd_vendor_disconnect()
{
    disconnect(conn_dd_vendor);
}

void ChipSelecter::set_dd_series_connect()
{
    conn_dd_series = connect(ui->comboBox_series, SIGNAL(currentIndexChanged(int)), this, SLOT(cb_combobox_series(int)));
}

void ChipSelecter::set_dd_series_disconnect()
{
    disconnect(conn_dd_series);
}

void ChipSelecter::set_dd_chip_connect()
{
    conn_dd_chip = connect(ui->comboBox_chip, SIGNAL(currentIndexChanged(int)), this, SLOT(cb_combobox_chip(int)));
}

void ChipSelecter::set_dd_chip_disconnect()
{
    disconnect(conn_dd_chip);
}

ChipInfo ChipSelecter::chip_info()
{
    ChipInfo info = *current_chip_info();

    if (info.flash_size_str.isEmpty())
    {
        if (current_series()->flash_size_str.isEmpty() == false)
        {
            info.flash_size_str = current_series()->flash_size_str;
        }
    }

    if (info.core.isEmpty())
    {
        if (current_series()->core.isEmpty() == false)
        {
            info.core = current_series()->core;
        }
    }

    return info;
}

int Vendor::from_vendor_file(QString vendor_file_path)
{
    QFile file;
    QByteArray file_buf;
    QString name;
    bool ok;

    file.setFileName(vendor_file_path);
    if (file.exists() == false)
        return -1;

    ok = file.open(QIODevice::ReadOnly);
    file_buf = file.readAll();
    file.close();

    YAML::Node node = YAML::Load(file_buf.constData());

    if (node.IsMap() == false)
        return -1;

    return from_node(node);
}

int Vendor::from_node(YAML::Node node)
{
    YAML::Node tmp_node;
    int err;

    // 厂商名称
    tmp_node = node["name"];
    if (tmp_node.IsScalar() == false)
        return -1;
    this->name = QString(tmp_node.as<std::string>().c_str());

    // 主页
    tmp_node = node["homepage"];
    if (tmp_node.IsScalar())
    {
        this->homepage = QString(tmp_node.as<std::string>().c_str());
        // qDebug("[ChipSelecto] Series[%s] core: %s", qPrintable(this->name), qPrintable(this->core));
    }

    // 系列
    YAML::Node node_series;
    node_series = node["series"];
    if (node_series.IsSequence() == false)
        return -1;

    series_list.clear();
    for (uint32_t i = 0; i < node_series.size(); i++)
    {
        series_list.push_back(NULL);
    }

    for (int i = 0; i < node_series.size(); i++)
    {
        Series *tmp_series = new Series();

        err = tmp_series->from_node(node_series[i]);
        if (err < 0)
            continue;

        series_list[i] = tmp_series;
    }

    return 0;
}

// static int from_node(Vendor *vendor, YAML::Node node)
// {
//     return vendor->from_node(node);
// }

// static int from_vendor_file(Vendor *vendor, QString vendor_file_path)
// {

//     return vendor->from_vendor_file(vendor_file_path);
// }

int Series::from_node(YAML::Node node)
{
    YAML::Node tmp_node;
    int err;

    // 系列名称
    tmp_node = node["name"];
    if (tmp_node.IsScalar() == false)
        return -1;
    this->name = QString(tmp_node.as<std::string>().c_str());
    // qDebug("    %s", qPrintable(this->name));

    // 主页
    tmp_node = node["homepage"];
    if (tmp_node.IsScalar())
    {
        this->homepage = QString(tmp_node.as<std::string>().c_str());
        // qDebug("[ChipSelecto] Series[%s] core: %s", qPrintable(this->name), qPrintable(this->core));
    }

    // 内核类型
    tmp_node = node["core"];
    if (tmp_node.IsScalar())
    {
        this->core = QString(tmp_node.as<std::string>().c_str());
        // qDebug("[ChipSelecto] Series[%s] core: %s", qPrintable(this->name), qPrintable(this->core));
    }

    // RAM容量
    tmp_node = node["ram_size"];
    if (tmp_node.IsScalar())
    {
        this->ram_size_str = QString(tmp_node.as<std::string>().c_str());
        // qDebug("[ChipSelecto] Series[%s] algo: %s", qPrintable(this->name), qPrintable(this->algo));
    }

    // Flash容量
    tmp_node = node["flash_size"];
    if (tmp_node.IsScalar())
    {
        this->flash_size_str = QString(tmp_node.as<std::string>().c_str());
        // qDebug("[ChipSelecto] Series[%s] algo: %s", qPrintable(this->name), qPrintable(this->algo));
    }

    // 算法文件
    tmp_node = node["algo"];
    if (tmp_node.IsScalar())
    {
        this->algo = QString(tmp_node.as<std::string>().c_str());
        // qDebug("[ChipSelecto] Series[%s] algo: %s", qPrintable(this->name), qPrintable(this->algo));
    }

    bool is_exist_chips = true;

    // 系列
    YAML::Node node_chips;
    node_chips = node["chips"];
    if (node_chips.IsSequence() == false)
    {
        is_exist_chips = false;
    }

    if (is_exist_chips)
    {
        chip_info_list.clear();
        for (uint32_t i = 0; i < node_chips.size(); i++)
        {
            chip_info_list.push_back(NULL);
        }

        for (int i = 0; i < node_chips.size(); i++)
        {
            ChipInfo *tmp_chip = new ChipInfo();

            err = tmp_chip->from_node(node_chips[i]);
            if (err < 0)
                continue;

            chip_info_list[i] = tmp_chip;
        }
    }

    return 0;
}

int ChipInfo::from_node(YAML::Node node)
{
    YAML::Node tmp_node;

    // 芯片名称
    tmp_node = node["name"];
    if (tmp_node.IsScalar() == false)
        return -1;
    this->name = QString(tmp_node.as<std::string>().c_str());

    // qDebug("        %s", qPrintable(this->name));

    // 内核类型
    tmp_node = node["core"];
    if (tmp_node.IsScalar())
    {
        this->core = QString(tmp_node.as<std::string>().c_str());
    }

    // RAM容量
    tmp_node = node["ram_size"];
    if (tmp_node.IsScalar())
    {
        this->ram_size_str = QString(tmp_node.as<std::string>().c_str());
        // qDebug("[ChipSelecto] Series[%s] algo: %s", qPrintable(this->name), qPrintable(this->algo));
    }

    // Flash容量
    tmp_node = node["flash_size"];
    if (tmp_node.IsScalar())
    {
        this->flash_size_str = QString(tmp_node.as<std::string>().c_str());
        // qDebug("[ChipSelecto] Series[%s] algo: %s", qPrintable(this->name), qPrintable(this->algo));
    }

    // 算法文件
    tmp_node = node["algo"];
    if (tmp_node.IsScalar())
    {
        this->algo = QString(tmp_node.as<std::string>().c_str());
        // qDebug("[ChipSelecto] Series[%s] algo: %s", qPrintable(this->name), qPrintable(this->algo));
    }

    return 0;
}
