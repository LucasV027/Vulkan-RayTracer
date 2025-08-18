#include "Raytracer.h"

#include <fstream>

#include "Serialize/Serialize.h"
#include "Core/Log.h"

Raytracer::Raytracer(const uint32_t width, const uint32_t height) : width(width),
                                                                    height(height),
                                                                    scene({}) {
    SetAllDirty();
}

void Raytracer::LoadFromFile(const std::filesystem::path& filepath) {
    if (filepath.extension() != ".json") {
        LOGE("Loading only support json file");
        return;
    }

    std::ifstream read(filepath);
    if (!read.is_open()) {
        LOGE("Failed to open file: {}", filepath.string());
    } else {
        try {
            const Json r = Json::parse(read);
            from_json(r, *this);
            LOGI("Successfully loaded file: {}", filepath.string());
            SetAllDirty();
        } catch (const Json::exception& e) {
            LOGE("Failed to parse JSON: {}", e.what());
        }

        read.close();
    }
}

void Raytracer::SaveToFile(const std::filesystem::path& filepath) {
    if (filepath.extension() != ".json") {
        LOGE("Loading only support json file");
        return;
    }

    std::ofstream write(filepath);
    if (!write.is_open()) {
        LOGE("Failed to open file: {}", filepath.string());
    } else {
        write << std::setw(4) << static_cast<Json>(*this) << std::endl;
        write.close();
        LOGI("Saved to file: {}", filepath.string());
    }
}

void Raytracer::Update(const uint32_t newWidth, const uint32_t newHeight) {
    if (newWidth != width || newHeight != height) {
        width = newWidth;
        height = newHeight;
        SetAllDirty();
    }
}
