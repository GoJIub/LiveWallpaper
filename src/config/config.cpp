#include "config.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;

const int MIN_WEATHER_UPDATE_INTERVAL = 1;
const double DEFAULT_LATITUDE = 55.0;
const double DEFAULT_LONGITUDE = 37.0;
const int DEFAULT_FPS = 30;
const int DEFAULT_PARTICLE_COUNT = 100;
const int DEFAULT_PARTICLE_LIFETIME = 1000;
const std::string DEFAULT_CONFIG_PATH = "config.default.json";

ConfigManager::ConfigManager(const std::string& file_path): config_file_path(file_path) {
    std::ifstream config_file(config_file_path);
    if (!config_file.is_open()) {
        const char* home_env = std::getenv("HOME");
        if (!home_env) {
            throw std::runtime_error("HOME environment variable not set");
        }
        std::string home_dir = home_env;
        std::string user_config_path = home_dir + "/.config/livewallpaper/config.json";

        fs::create_directories(fs::path(user_config_path).parent_path());
        fs::copy_file(
            fs::path(DEFAULT_CONFIG_PATH),
            fs::path(user_config_path),
            fs::copy_options::overwrite_existing
        );

        std::cout << "Config file not found. A default config has been created at: " << user_config_path << std::endl;
        config_file.open(user_config_path);
    }

    nlohmann::json config_json;
    config_file >> config_json;

    config.latitude = config_json.value("latitude", DEFAULT_LATITUDE);
    config.longitude = config_json.value("longitude", DEFAULT_LONGITUDE);
    config.weather_update_interval = config_json.value("weather_update_interval", 10);
    config.fps = config_json.value("fps", DEFAULT_FPS);
    config.particle_count = config_json.value("particle_count", DEFAULT_PARTICLE_COUNT);
    config.particle_lifetime = config_json.value("particle_lifetime", DEFAULT_PARTICLE_LIFETIME);

    if (config.weather_update_interval < MIN_WEATHER_UPDATE_INTERVAL) {
        config.weather_update_interval = MIN_WEATHER_UPDATE_INTERVAL;
        std::cout << "Weather update interval too low, set to minimum: " << MIN_WEATHER_UPDATE_INTERVAL << " minute(s)" << std::endl;
    }
}

const Config& ConfigManager::get_config() const {
    return config;
}