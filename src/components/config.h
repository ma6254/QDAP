#ifndef QDAP_CONFIG_H
#define QDAP_CONFIG_H

#include <QObject>
#include <yaml-cpp/yaml.h>
#include "devices.h"

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

    uint32_t get_cmsis_dap_clock();

    QString firmware_file_path;
    bool auto_refresh_enum_devices;

    QString chip_vendor_name;
    QString chip_series_name;
    QString chip_name;

    QString chips_url;

    QString cmsis_dap_port_str;
    CMSIS_DAP_Base::Port cmsis_dap_port;
    bool cmsis_dap_swj;
    QString cmsis_dap_clock_str;
    uint64_t cmsis_dap_clock;
    Devices::ClockUnit cmsis_dap_clock_unit;

    int hexview_line_bytes;  // 每行字节数
    int hexview_group_bytes; // 分组字节数

private:
};

#endif // QDAP_CONFIG_H
