#ifndef QDAP_CONFIG_H
#define QDAP_CONFIG_H

#include <QObject>
#include <yaml-cpp/yaml.h>

class Config : public QObject
{
    Q_OBJECT

public:
    Config();
    ~Config();

    static QString get_default_path();
    static Config *get_default();

    int from_file(QString file_path = "");
    int from_node(YAML::Node node);

    int to_file(QString file_path = "");
    int to_node(YAML::Node *node);

    QString firmware_file_path;
    bool auto_refresh_enum_devices;

    QString chip_vendor_name;
    QString chip_series_name;
    QString chip_name;

    QString chips_url;

private:
};

#endif // QDAP_CONFIG_H
