#pragma once
#ifndef RUSTSPDLOG_H_
#define RUSTSPDLOG_H_

#include <SpdLog.h>



void spdlog_critical(const std::string& message);
void spdlog_error(const std::string& message);;
void spdlog_warn(const std::string& message);
void spdlog_info(const std::string& message);
void spdlog_debug(const std::string& message);
void spdlog_trace(const std::string& message);

#endif