#pragma once

// simplifies the loading of shaders
#include <cmrc/cmrc.hpp>
CMRC_DECLARE(shaders);
static inline const char* load_shader(const std::string& path) {
    auto fs = cmrc::shaders::get_filesystem();
    if (fs.exists(path)) {
        auto file = fs.open(path);
        return file.cbegin();
    }
    else {
        std::cerr << "Unable to load shader: " << path << std::endl;
        return nullptr;
    }
}

// simplifies loading of images
CMRC_DECLARE(images);
static inline std::pair<const unsigned char*, std::size_t> load_image(const std::string& path) {
    auto fs = cmrc::images::get_filesystem();
    if (fs.exists(path)) {
        auto file = fs.open(path);
        return { reinterpret_cast<const unsigned char*>(file.cbegin()), file.size() };
    }
    else {
        std::cerr << "Unable to load image: " << path << std::endl;
        return { nullptr, 0 };
    }
}

// simplifies loading of models
#ifdef EMBEDDED_MODELS
CMRC_DECLARE(models);
static inline std::pair<const unsigned char*, std::size_t> load_model_resource(const std::string& path) {
    auto fs = cmrc::models::get_filesystem();
    if (fs.exists(path)) {
        auto file = fs.open(path);
        return { reinterpret_cast<const unsigned char*>(file.cbegin()), file.size() };
    }
    else {
        std::cerr << "Unable to load model resource: " << path << std::endl;
        return { nullptr, 0 };
    }
}
#endif