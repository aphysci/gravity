#include "RustSpdLog.h"

void spdlog_critical(const std::string& message) {
    gravity::SpdLog::critical(message.c_str());
}
void spdlog_error(const std::string& message) {
    gravity::SpdLog::error(message.c_str());
}
void spdlog_warn(const std::string& message) {
    gravity::SpdLog::warn(message.c_str());
}
void spdlog_info(const std::string& message) {
    gravity::SpdLog::info(message.c_str());
}
void spdlog_debug(const std::string& message) {
    gravity::SpdLog::debug(message.c_str());
}
void spdlog_trace(const std::string& message) {
    gravity::SpdLog::trace(message.c_str());
}