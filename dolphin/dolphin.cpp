#include <iostream>
#include <string>
#include <ifstream>

#include "PEVersionInfo.h"

using std::string;

// https://github.com/blackknifes/PEVersionInfo

//TODO：1、读取配置文件，批量修改exe信息
//  {
//     "exe_name": {
//         "FileVersion":1.0.0.200,
//         "CopyRight":"我是测试性文字",
//     }
// }
// 2、默认读取当前目录下的config.json
int main(int argc, char *argv[]) { 
    string path = "./config.json";
    if (argc > 1)
        path = argv[1];

    std::ifstream ifs;
    ifs.open(path, ios::in);
    if (!isf.isopen()) {
        std::cout << "Open file failed." << std::endl;
        return -1;
    }
    ifs.readAll();
    return 0; 
}
