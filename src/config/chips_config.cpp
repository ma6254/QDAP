#include "chips_config.h"

ChipsConfig::ChipsConfig()
{
}

ChipsConfig::~ChipsConfig()
{
}

/*******************************************************************************
 * @brief 默认参数的配置文件
 * @param None
 * @return None
 ******************************************************************************/
ChipsConfig *ChipsConfig::get_default()
{
    ChipsConfig *config = new ChipsConfig();

    config->url = "https://github.com/ma6254/qdap_chips/archive/refs/heads/main.zip";
    // config->url = "https://github.com/ma6254/qdap_chips/archive/refs/tags/v0.1.zip";
    // config->url = "http://127.0.0.1:8000/main.zipConnectionRefusedError";
    // config->url = "https://git.s2.ma6254.com/qdap/qdap_chips/archive/main.zip";

    return config;
}

/*******************************************************************************
 * @brief 从YAML节点中加载
 * @param None
 * @return None
 ******************************************************************************/
int ChipsConfig::from_node(YAML::Node node)
{
    YAML::Node tmp_node;
    int n = 0;
    // int err;

    if (node.IsMap() == false)
        return -1;

    tmp_node = node["url"];
    if (tmp_node.IsScalar() == false)
        return -1;
    url = QString(tmp_node.as<std::string>().c_str());

    tmp_node = node["latest"];
    if (tmp_node.IsScalar() == false)
        return -1;
    latest = QString(tmp_node.as<std::string>().c_str());

    return n;
}

/*******************************************************************************
 * @brief 保存到YAML节点中
 * @param None
 * @return None
 ******************************************************************************/
int ChipsConfig::to_node(YAML::Node *node)
{
    int n = 0;
    if (node == nullptr)
        return -1;

    (*node)["url"] = qPrintable(url);
    n++;

    (*node)["latest"] = qPrintable(latest);
    n++;

    return n;
}
