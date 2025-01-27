#ifndef CHIPS_CONFIG_H
#define CHIPS_CONFIG_H

#include <QObject>
#include <yaml-cpp/yaml.h>
#include "devices.h"

class ChipsConfig : public QObject
{
    Q_OBJECT

public:
    ChipsConfig();
    ~ChipsConfig();

    static ChipsConfig *get_default();

    int from_file(QString file_path = "");
    int from_node(YAML::Node node);

    int to_file(QString file_path = "");
    int to_node(YAML::Node *node);

    uint32_t get_cmsis_dap_clock();

    QString url;
    QString latest;

    
    // private:
};

#endif // CHIPS_CONFIG_H
