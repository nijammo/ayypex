#pragma once

#include "math.h"
#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <fstream>
#include "logging.h"

using json = nlohmann::json;

namespace configuration {
    extern json settings;

    void init();
    void reload(std::string path);

    template<typename T>
    inline T get(std::string name) {
        return settings.at(name).get<T>();
    }

    template<typename T>
    inline T get_or_default(std::string name, T default_value) {
        if (settings.find(name) == settings.end()) {
            settings[name] = default_value;
            return default_value;
        }

        return settings.at(name).get<T>();
    }
}