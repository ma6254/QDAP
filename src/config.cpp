#include <QCoreApplication>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include "config.h"

Config::Config()
{
}

Config::~Config()
{
}

/*******************************************************************************
 * @brief 默认参数的配置文件
 * @param None
 * @return None
 ******************************************************************************/
Config *Config::get_default()
{
    Config *config = new Config();
    config->firmware_file_path = "";
    config->auto_refresh_enum_devices = true;
    config->chip_vendor_name = "";
    config->chip_series_name = "";
    config->chip_name = "";
    config->chips_url = "https://github.com/ma6254/qdap_chips";

    return config;
}

/*******************************************************************************
 * @brief 配置文件默认路径
 * @param None
 * @return None
 ******************************************************************************/
QString Config::get_default_path()
{
    // QStringList path;
    // path.append(QCoreApplication::applicationDirPath());
    // path.append("config.yml");
    // return path.join(QDir::separator());
    return QCoreApplication::applicationDirPath() + QDir::separator() + "config.yml";
}

/*******************************************************************************
 * @brief 从文件中加载
 * @param None
 * @return None
 ******************************************************************************/
int Config::from_file(QString file_path)
{
    QFile file;
    QByteArray config_file_buf;
    bool ok;

    if (file_path.isEmpty())
    {
        file_path = get_default_path();
    }

    file.setFileName(file_path);
    if (file.exists() == false)
        return -1;

    ok = file.open(QIODevice::ReadOnly);
    config_file_buf = file.readAll();
    file.close();

    YAML::Node node = YAML::Load(config_file_buf.constData());
    return from_node(node);
}

/*******************************************************************************
 * @brief 从YAML节点中加载
 * @param None
 * @return None
 ******************************************************************************/
int Config::from_node(YAML::Node node)
{
    YAML::Node tmp_node;

    if (node.IsMap() == false)
        return -1;

    tmp_node = node["firmware_file_path"];
    if (tmp_node.IsScalar() == false)
        return -1;
    firmware_file_path = QString(tmp_node.as<std::string>().c_str());

    qDebug("[cfg] firmware_file_path: %s", qPrintable(firmware_file_path));

    // 自动刷新枚举设备
    tmp_node = node["auto_refresh_enum_devices"];
    if (tmp_node.IsScalar() == false)
        return -1;
    auto_refresh_enum_devices = tmp_node.as<bool>();

    YAML::Node node_chip_selected = node["chip_selected"];
    if (node_chip_selected.IsMap() == false)
        return -1;

    tmp_node = node_chip_selected["vendor_name"];
    if (tmp_node.IsScalar() == false)
        return -1;
    chip_vendor_name = QString(tmp_node.as<std::string>().c_str());

    tmp_node = node_chip_selected["series_name"];
    if (tmp_node.IsScalar() == false)
        return -1;
    chip_series_name = QString(tmp_node.as<std::string>().c_str());

    tmp_node = node_chip_selected["chip_name"];
    if (tmp_node.IsScalar() == false)
        return -1;
    chip_name = QString(tmp_node.as<std::string>().c_str());

    qDebug("[cfg] chip_selected: [%s] [%s] [%s]",
           qUtf8Printable(chip_vendor_name),
           qUtf8Printable(chip_series_name),
           qUtf8Printable(chip_name));

    YAML::Node node_chips_library = node["chips_library"];
    if (node_chips_library.IsMap() == false)
        return -1;

    tmp_node = node_chips_library["url"];
    if (tmp_node.IsScalar() == false)
        return -1;
    chips_url = QString(tmp_node.as<std::string>().c_str());

    qDebug("[Config] load ok");
    return 0;
}

/*******************************************************************************
 * @brief 保存到文件中
 * @param None
 * @return None
 ******************************************************************************/
int Config::to_file(QString file_path)
{
    int err;
    YAML::Node node;

    err = to_node(&node);
    if (err < 0)
        return err;

    if (file_path.isEmpty())
    {
        file_path = get_default_path();
    }

    qDebug("[cfg] emitter");
    YAML::Emitter emitter;
    // emitter.SetIndent(4);
    emitter << node;

    QFile file;
    file.setFileName(file_path);
    if (file.exists())
        file.remove();

    file.open(QIODevice::ReadWrite);
    file.write(emitter.c_str());
    file.flush();
    file.close();

    qDebug("[cfg] config_save");
    return 0;
}

/*******************************************************************************
 * @brief 保存到YAML节点中
 * @param None
 * @return None
 ******************************************************************************/
int Config::to_node(YAML::Node *node)
{
    if (node == nullptr)
        return -1;

    qDebug("[cfg] config_save start");

    QDateTime now_date_time = QDateTime::currentDateTime();
    QString now_datetime_str = now_date_time.toString("yyyy/MM/dd hh:mm:ss");

    YAML::Node node_fimware_history;
    node_fimware_history.push_back("123");
    node_fimware_history.push_back("456");

    // YAML::Node node;
    (*node)["notes"] = "Please do not edit this file manually.";
    (*node)["notes_zh_cn"] = "请不要手动编辑该文件";
    (*node)["latest"] = qPrintable(now_datetime_str);
    (*node)["firmware_file_path"] = qPrintable(firmware_file_path);
    (*node)["firmware_history"] = node_fimware_history;
    (*node)["auto_refresh_enum_devices"] = auto_refresh_enum_devices;

    // 已选中的芯片
    qDebug("[cfg] node_chip_selected");
    YAML::Node node_chip_selected;
    node_chip_selected["vendor_name"] = qUtf8Printable(chip_vendor_name);
    node_chip_selected["series_name"] = qUtf8Printable(chip_series_name);
    node_chip_selected["chip_name"] = qUtf8Printable(chip_name);
    (*node)["chip_selected"] = node_chip_selected;

    // 芯片器件库
    qDebug("[cfg] node_chips_library");
    YAML::Node node_chips_library;
    node_chips_library["url"] = qUtf8Printable(chips_url);
    (*node)["chips_library"] = node_chips_library;

    // 已选中的设备
    qDebug("[cfg] node_devices_selected");
    YAML::Node node_devices_selected;
    node_devices_selected["any"] = "";
    node_devices_selected["type"] = "";
    node_devices_selected["manufacturer"] = "";
    node_devices_selected["product"] = "";
    node_devices_selected["serial"] = "";
    (*node)["devices_selected"] = node_devices_selected;

    // QDir dir_config(config_dir_path);
    // if (!dir_config.exists())
    // {
    //     dir_config.mkdir(".");
    // }

    return 0;
}
