#pragma once

#include <iostream>
#include <format>

#define LOGI(...) std::cout << "[INFO] " << std::format(__VA_ARGS__)
#define LOGW(...) std::cout << "[WARN] " << std::format(__VA_ARGS__)
#define LOGE(...) std::cout << "[ERROR] " << std::format(__VA_ARGS__)
#define LOGD(...) std::cout << "[DEBUG] " << std::format(__VA_ARGS__)