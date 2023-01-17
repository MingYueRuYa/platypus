#include "spdhelp.h"

#include "../3rdparty/spdlog/include/spdlog/logger.h"
#include "../3rdparty/spdlog/include/spdlog/sinks/basic_file_sink.h"
#include "../3rdparty/spdlog/include/spdlog/sinks/windebug_sink.h"

namespace spdlog {

SpdHelper::SpdHelper(const string &file_name, const string &log_name) { init(file_name, log_name); }
SpdHelper::~SpdHelper() {}

void SpdHelper::init(const string &file_name, const string &log_name) {
  auto console_sink = std::make_shared<sinks::windebug_sink_mt>();
  console_sink->set_level(level::debug);
  console_sink->set_pattern("[%l] [tid %t] %v");

  auto file_sink =
      std::make_shared<sinks::basic_file_sink_mt>(file_name, true);
  file_sink->set_level(level::debug);
  file_sink->set_pattern("%Y-%m-%d %H:%M:%S [%l] [tid %t] %v");

  logger_ =
      std::make_shared<logger>(log_name, sinks_init_list{console_sink, file_sink});
  logger_->set_level(level::debug);
  logger_->flush_on(level::err);
  spdlog::register_logger(logger_);
  spdlog::flush_every(std::chrono::seconds(1));
}

}  // namespace spdlog