#include <string>

#include "../3rdparty/spdlog/include/spdlog/common.h"
#include "../3rdparty/spdlog/include/spdlog/spdlog.h"

using string = std::string;

namespace spdlog {

class SpdHelper {
 public:
  explicit SpdHelper(const string &file_name, const string &log_name);
  ~SpdHelper();

  template <typename... Args>
  void log(source_loc loc, level::level_enum lvl, wformat_string_t<Args...> fmt,
           Args &&...args) {
    logger_->log_(loc, lvl, fmt, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void log(level::level_enum lvl, wformat_string_t<Args...> fmt,
           Args &&...args) {
    logger_->log(source_loc{}, lvl, fmt, std::forward<Args>(args)...);
  }

  void log(log_clock::time_point log_time, source_loc loc,
           level::level_enum lvl, wstring_view_t msg) {
    logger_->log(log_time, loc, lvl, msg);
  }

  void log(source_loc loc, level::level_enum lvl, wstring_view_t msg) {
    logger_->log(loc, lvl, msg);
  }

  void log(level::level_enum lvl, wstring_view_t msg) {
    logger_->log(source_loc{}, lvl, msg);
  }

  template <typename... Args>
  void trace(wformat_string_t<Args...> fmt, Args &&...args) {
    logger_->log(level::trace, fmt, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void debug(wformat_string_t<Args...> fmt, Args &&...args) {
    logger_->log(level::debug, fmt, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void info(wformat_string_t<Args...> fmt, Args &&...args) {
    logger_->log(level::info, fmt, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void warn(wformat_string_t<Args...> fmt, Args &&...args) {
    logger_->log(level::warn, fmt, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void error(wformat_string_t<Args...> fmt, Args &&...args) {
    logger_->log(level::err, fmt, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void critical(wformat_string_t<Args...> fmt, Args &&...args) {
    logger_->log(level::critical, fmt, std::forward<Args>(args)...);
  }

  template <typename T>
  void trace(const T &msg) {
    logger_->log(level::trace, msg);
  }

  template <typename T>
  void debug(const T &msg) {
    logger_->log(level::debug, msg);
  }

  template <typename T>
  void info(const T &msg) {
    logger_->log(level::info, msg);
  }

  template <typename T>
  void warn(const T &msg) {
    logger_->log(level::warn, msg);
  }

  template <typename T>
  void error(const T &msg) {
    logger_->log(level::err, msg);
  }

  template <typename T>
  void critical(const T &msg) {
    logger_->log(level::critical, msg);
  }

 private:
  void init(const string &file_name, const string &log_name);

 private:
  std::shared_ptr<logger> logger_;
};

}  // namespace spdlog