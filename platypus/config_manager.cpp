#include "config_manager.h"
#include "spdhelper.h"
#include "json.hpp"
#include "string_utils.hpp"
#include "singleton_spdlog.hpp"

#include <iostream>
#include <QObject>
#include <QFile>

using nlohmann::json;
using spdlog::SpdHelper;

const QString kCONFIG_FILE_NAME = "config.json";

ConfigManager::~ConfigManager() {}

string ConfigManager::GetConfig(ConfigType type) const {
  string config;
  auto ifind = short_cut_map_.find(type);
  if (ifind == short_cut_map_.end()) {
    spdlog::logInstance().error(L"Not find specific type {}", (int)type);
    return config;
  }
  config = ifind->second.second;
  return config;
}

bool ConfigManager::LoadSuccesful() const { return load_successful_; }

void ConfigManager::init() {
  if (!(load_successful_ = load())) {
    spdlog::logInstance().error(L"{}", L"load config error");
  }
}

bool ConfigManager::load() {
  const QString file_path = QString("%1").arg(kCONFIG_FILE_NAME);
  if (!QFile::exists(file_path)) {
    spdlog::logInstance().error(L"the file {} is not existed",
                                kCONFIG_FILE_NAME.toStdWString().c_str());
    return false;
  }

  QFile file(file_path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    spdlog::logInstance().error(L"{}", L"read confit file error");
    return false;
  }

  const std::map<std::string, ConfigType> config_map = {
      {"create_new_tab", ConfigType::Create_New_Tab},
      {"delete_tab", ConfigType::Delete_Tab},
      {"switch_forward_tab", ConfigType::Switch_Forward_Tab},
      {"switch_back_tab", ConfigType::Switch_Back_Tab},
      {"max_min_window", ConfigType::Max_Min_Window}};

  QByteArray array = file.readAll();
  QString config_str = array;
  string json_msg = to_utf8_string(config_str.toStdWString().c_str());
  json json_obj = json::parse(json_msg);
  auto short_cut_json = json_obj.at("short_cut");
  for (json::iterator it = short_cut_json.begin(); it != short_cut_json.end();
       ++it) {
    auto ifind = config_map.find(it.key());
    if (ifind != config_map.end()) {
      short_cut_map_[ifind->second] =
          std::pair<string, string>(it.key(), it.value());
    }
  }

  return true;
}
