#pragma once

#include <iostream>
#include <format>

#define LOGI(...) std::cout << "[INFO] " << std::format(__VA_ARGS__) << std::endl
#define LOGW(...) std::cout << "[WARN] " << std::format(__VA_ARGS__) << std::endl
#define LOGE(...) std::cout << "[ERROR] " << std::format(__VA_ARGS__) << std::endl
#ifndef NDEBUG
#define LOGD(...) std::cout << "[DEBUG] " << std::format(__VA_ARGS__) << std::endl
#endif
