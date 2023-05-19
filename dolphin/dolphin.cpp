#include <iostream>
#include <string>
#include <fstream>
#include <json.hpp>

#include "PEVersionInfo.h"
#include "string_utils.hpp"

using json = nlohmann::json;
using std::string;

// https://github.com/blackknifes/PEVersionInfo

// TODO：1、读取配置文件，批量修改exe信息
//   {
//          "FileVersion":1.0.0.200,
//          "CopyRight":"我是测试性文字",
//          "ProductVersion":1.0.0.200,
//  }
//  2、默认读取当前目录下的config.json
int main(int argc, char *argv[]) {
  if (argc < 3) {
    std::cout << "usage: app config_path exe_path" << std::endl;
    return 0;
  }
  std::ifstream f("example.json");
  json data = json::parse(f);
  if (data.is_null()) {
    std::cout << "parse json data error" << std::endl;
    return -1;
  }

  PEVersionInfo info;
  info.loadFromPEFile(argv[2]);

  string value = data.value("FileVersion", "");
  info.setFileVersion(to_wide_string(value));
  value = data.value("Copyright", "");
  info.setFileDescription(to_wide_string(value));

  info.saveToPEFile(argv[2]);
  std::cout << "update exe info successful" << std::endl;
  std::wstring version = info.getFileVersion();
  std::wstring desc = info.getFileDescription();
  std::wcout << L"file version:" << info.getFileVersion()
             << " file desc:" << info.getFileDescription() << std::endl;
  return 0;
}
