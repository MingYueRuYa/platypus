#ifndef config_manager_h
#define config_manager_h

#include <map>
#include <string>

#include "singleton.h"

using std::string;

#define ConfigManagerInstance SINGLETON_INSTANCE(ConfigManager)
#define ConfigManagerFinalize SINGLETON_FINALIZE(ConfigManager)

class ConfigManager : SINGLETON_INHERIT(ConfigManager) {
 public:
  enum class ConfigType {
    Begin = 0,
    Create_New_Tab,
    Delete_Tab,
    Switch_Back_Tab,
    Switch_Forward_Tab,
    Max_Min_Window,
    End
  };

 public:
  ~ConfigManager();
  string GetConfig(ConfigType type) const;
  bool LoadSuccesful() const;

 private:
  void init();
  bool load();

 private:
  std::map<ConfigType, std::pair<string, string>> short_cut_map_;
  bool load_successful_ = false;

  DECLARE_FRIEND_SINGLETON(ConfigManager);
  DECLARE_PRIVATE_CONSTRUCTOR(ConfigManager, init);
};

#endif  // config_manager_h