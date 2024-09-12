#ifndef CHIP_SELECTER_H
#define CHIP_SELECTER_H

#include <QDialog>
#include <yaml-cpp/yaml.h>
#ifdef _WIN32
#include <QDIR>
#else
#include <QDir>
#endif // _WIN32
#include <QComboBox>

namespace Ui {
class ChipSelecter;
}

class ChipInfo {
public:
    ChipInfo()
    {
    }
    ~ChipInfo()
    {
    }
    int from_node(YAML::Node node);

    QString name;
    QString log_text;
    QString core;
    QString algo;
    QString ram_size_str;
    QString flash_size_str;

private:
};

class Series {
public:
    Series()
    {
    }

    ~Series()
    {
    }

    int from_node(YAML::Node node);
    inline int chip_count() const { return chip_info_list.count(); }

    QList<QString> chips_name_list() const
    {
        QList<QString> ret;
        for (int i = 0; i < chip_info_list.count(); i++) {
            if (chip_info_list[i])
                ret.push_back(QString("%1 | %2").arg(i).arg(chip_info_list[i]->name));
            else
                ret.push_back(QString("%1 | %2 ").arg(i).arg("NULL"));
        }

        return ret;
    }

    QString name;
    QString homepage;
    QString log_text;
    QList<ChipInfo*> chip_info_list;
    QString core;
    QString algo;
    QString ram_size_str;
    QString flash_size_str;

private:
};

class Vendor {
public:
    Vendor()
    {
    }

    ~Vendor()
    {
    }

    int from_node(YAML::Node node);
    int from_vendor_file(QString vendor_file_path);

    inline int series_count() const { return series_list.count(); }
    QList<QString> series_name_list() const
    {
        QList<QString> ret;
        for (int i = 0; i < series_list.count(); i++) {
            if (series_list[i]->chip_count())
                ret.push_back(QString("%1 | %2").arg(i).arg(series_list[i]->name));
            else
                ret.push_back(QString("%1 | [❌无效] %2 ").arg(i).arg(series_list[i]->name));
        }

        return ret;
    }

    // static int from_node(Vendor *vendor, YAML::Node node);
    // static int from_vendor_file(Vendor *vendor, QString vendor_file_path);

    QString name;
    QString log_text;
    QString homepage;
    QList<Series*> series_list;

private:
};

class ChipSelecter : public QDialog {
    Q_OBJECT

public:
    explicit ChipSelecter(QWidget* parent = nullptr);
    ~ChipSelecter();
    void load_chips(QString chips_dir_path = "");
    int load_chip_vendor(QString vendor_file_path, Vendor* vendor);

    void log_clear();
    void log_output();
    void switch_vendor(uint32_t index);
    void switch_series(uint32_t index);
    void switch_chip(uint32_t index);

    bool switch_vendor(QString vendor_name);
    bool switch_series(QString series_name);
    bool switch_chip(QString chip_name);
    bool switch_chip(QString vendor_name, QString series_name, QString chip_name);

    QString vendor_name();
    QString vendor_homepage();
    QString series_name();
    QString series_homepage();
    QString chip_name();

    QString core_homepage(QString core);

    Vendor* current_vendor()
    {
        return vendor_list[current_vendor_index];
    }

    Series* current_series()
    {
        return current_vendor()->series_list[current_series_index];
    }

    ChipInfo* current_chip_info()
    {
        return current_series()->chip_info_list[current_chip_index];
    }

    bool is_chip_info_available()
    {
        if (current_series()->chip_info_list.count() == 0)
            return false;

        if (current_chip_info() == NULL)
            return false;

        return true;
    }

    QString chip_core()
    {
        QString ret = current_series()->core;

        if (is_chip_info_available() == false)
            return ret;

        if (current_chip_info()->core.isEmpty())
            return ret;

        return current_chip_info()->core;
    }

    QString chip_algo()
    {
        QString ret = current_series()->algo;

        if (is_chip_info_available() == false)
            return ret;

        if (current_chip_info()->algo.isEmpty())
            return ret;

        return current_chip_info()->algo;
    }

    QString chip_ram_size()
    {
        QString ret = current_series()->ram_size_str;

        if (is_chip_info_available() == false)
            return ret;

        if (current_chip_info()->ram_size_str.isEmpty())
            return ret;

        return current_chip_info()->ram_size_str;
    }

    QString chip_flash_size()
    {
        QString ret = current_series()->flash_size_str;

        if (is_chip_info_available() == false)
            return ret;

        if (current_chip_info()->flash_size_str.isEmpty())
            return ret;

        return current_chip_info()->flash_size_str;
    }

    void set_dd_vendor_connect();
    void set_dd_vendor_disconnect();
    void set_dd_series_connect();
    void set_dd_series_disconnect();
    void set_dd_chip_connect();
    void set_dd_chip_disconnect();

    ChipInfo chip_info();

public slots:
    void cb_combobox_vendor(int index);
    void cb_combobox_series(int index);
    void cb_combobox_chip(int index);

    void cb_btn_ok();

private:
    Ui::ChipSelecter* ui;

    // QList<QList<QString>> series_name_list;
    // QList<QList<QList<QString>>> chip_name_list;
    QList<Vendor*> vendor_list;
    int current_vendor_index;
    int current_series_index;
    int current_chip_index;

    QString log_vendor;
    QString log_series;
    QString log_chip;

    QStringList error_info;

    QMetaObject::Connection conn_dd_vendor;
    QMetaObject::Connection conn_dd_series;
    QMetaObject::Connection conn_dd_chip;

    QMap<QString, QString> chip_core_homepage_map;
};

#endif // CHIP_SELECTER_H
