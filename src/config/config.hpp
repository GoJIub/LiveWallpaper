#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>

struct Config {
    double latitude, longitude;
    int weather_update_interval;
    int fps;
    int particle_count;
    int particle_lifetime;
};

class ConfigManager {
public:

    ConfigManager(const std::string& file_path);

    const Config& get_config() const;

    ~ConfigManager() = default;

private:
    std::string config_file_path;
    Config config;
};

#endif // CONFIG_HPP