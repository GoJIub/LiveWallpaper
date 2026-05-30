#include "config/config.hpp"

#include <gtest/gtest.h>

#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

namespace {

namespace fs = std::filesystem;

class ScopedCurrentPath {
public:
    explicit ScopedCurrentPath(const fs::path& path)
        : original_path(fs::current_path())
    {
        fs::current_path(path);
    }

    ~ScopedCurrentPath() {
        fs::current_path(original_path);
    }

private:
    fs::path original_path;
};

class ScopedEnvVar {
public:
    ScopedEnvVar(const char* name, const std::string& value)
        : name(name)
    {
        const char* existing = std::getenv(name);
        if (existing) {
            old_value = existing;
            had_value = true;
        }
        set(value);
    }

    ~ScopedEnvVar() {
        if (had_value) {
            set(old_value);
        } else {
            unset();
        }
    }

private:
    void set(const std::string& value) const {
#ifdef _WIN32
        _putenv_s(name.c_str(), value.c_str());
#else
        setenv(name.c_str(), value.c_str(), 1);
#endif
    }

    void unset() const {
#ifdef _WIN32
        _putenv_s(name.c_str(), "");
#else
        unsetenv(name.c_str());
#endif
    }

    std::string name;
    std::string old_value;
    bool had_value = false;
};

class ScopedUnsetEnvVar {
public:
    explicit ScopedUnsetEnvVar(const char* name)
        : name(name)
    {
        const char* existing = std::getenv(name);
        if (existing) {
            old_value = existing;
            had_value = true;
        }
        unset();
    }

    ~ScopedUnsetEnvVar() {
        if (had_value) {
            set(old_value);
        }
    }

private:
    void set(const std::string& value) const {
#ifdef _WIN32
        _putenv_s(name.c_str(), value.c_str());
#else
        setenv(name.c_str(), value.c_str(), 1);
#endif
    }

    void unset() const {
#ifdef _WIN32
        _putenv_s(name.c_str(), "");
#else
        unsetenv(name.c_str());
#endif
    }

    std::string name;
    std::string old_value;
    bool had_value = false;
};

fs::path unique_temp_path(const std::string& name) {
    const auto* test_info = ::testing::UnitTest::GetInstance()->current_test_info();
    return fs::temp_directory_path() / fs::path(
        "livewallpaper_" +
        std::string(test_info->test_suite_name()) + "_" +
        std::string(test_info->name()) + "_" +
        name
    );
}

void write_file(const fs::path& path, const std::string& content) {
    fs::create_directories(path.parent_path());
    std::ofstream file(path);
    ASSERT_TRUE(file.is_open());
    file << content;
}

} // namespace

TEST(ConfigManagerTest, ReadsAllFieldsFromValidJson) {
    const fs::path config_path = unique_temp_path("valid") / "config.json";
    write_file(config_path, R"({
        "latitude": 12.5,
        "longitude": 45.75,
        "weather_update_interval": 15,
        "fps": 60,
        "particle_count": 250,
        "particle_lifetime": 3000
    })");

    ConfigManager manager(config_path.string());
    const Config& config = manager.get_config();

    EXPECT_DOUBLE_EQ(config.latitude, 12.5);
    EXPECT_DOUBLE_EQ(config.longitude, 45.75);
    EXPECT_EQ(config.weather_update_interval, 15);
    EXPECT_EQ(config.fps, 60);
    EXPECT_EQ(config.particle_count, 250);
    EXPECT_EQ(config.particle_lifetime, 3000);
}

TEST(ConfigManagerTest, ClampsWeatherUpdateIntervalBelowMinimum) {
    const fs::path config_path = unique_temp_path("clamp") / "config.json";
    write_file(config_path, R"({
        "weather_update_interval": 0
    })");

    ConfigManager manager(config_path.string());

    EXPECT_EQ(manager.get_config().weather_update_interval, 1);
}

TEST(ConfigManagerTest, KeepsWeatherUpdateIntervalAtMinimum) {
    const fs::path config_path = unique_temp_path("min_interval") / "config.json";
    write_file(config_path, R"({
        "weather_update_interval": 1
    })");

    ConfigManager manager(config_path.string());

    EXPECT_EQ(manager.get_config().weather_update_interval, 1);
}

TEST(ConfigManagerTest, ClampsFpsBelowMinimum) {
    const fs::path config_path = unique_temp_path("fps_clamp") / "config.json";
    write_file(config_path, R"({
        "fps": 0
    })");

    ConfigManager manager(config_path.string());

    EXPECT_EQ(manager.get_config().fps, 1);
}

TEST(ConfigManagerTest, ClampsParticleLifetimeBelowMinimum) {
    const fs::path config_path = unique_temp_path("particle_lifetime_clamp") / "config.json";
    write_file(config_path, R"({
        "particle_lifetime": 0
    })");

    ConfigManager manager(config_path.string());

    EXPECT_EQ(manager.get_config().particle_lifetime, 1);
}

TEST(ConfigManagerTest, ThrowsWhenConfigAndDefaultConfigAreMissing) {
    const fs::path temp_dir = unique_temp_path("missing");
    const fs::path home_dir = temp_dir / "home";
    fs::create_directories(home_dir);

    ScopedCurrentPath current_path(temp_dir);
    ScopedEnvVar home("HOME", home_dir.string());
    ScopedEnvVar data_dir("LIVEWALLPAPER_DATA_DIR", temp_dir.string());

    EXPECT_THROW(ConfigManager("missing.json"), std::exception);
}

TEST(ConfigManagerTest, ThrowsWhenHomeIsMissingAndConfigIsMissing) {
    const fs::path temp_dir = unique_temp_path("missing_home");
    fs::create_directories(temp_dir);

    ScopedCurrentPath current_path(temp_dir);
    ScopedUnsetEnvVar home("HOME");
    ScopedEnvVar data_dir("LIVEWALLPAPER_DATA_DIR", temp_dir.string());

    EXPECT_THROW(ConfigManager("missing.json"), std::runtime_error);
}

TEST(ConfigManagerTest, CopiesDefaultConfigToUserConfigWhenConfigIsMissing) {
    const fs::path temp_dir = unique_temp_path("fallback");
    const fs::path home_dir = temp_dir / "home";
    fs::create_directories(home_dir);
    write_file(temp_dir / "config.default.json", R"({
        "latitude": 51.5,
        "longitude": -0.12,
        "weather_update_interval": 8,
        "fps": 24,
        "particle_count": 42,
        "particle_lifetime": 750
    })");

    ScopedCurrentPath current_path(temp_dir);
    ScopedEnvVar home("HOME", home_dir.string());
    ScopedEnvVar data_dir("LIVEWALLPAPER_DATA_DIR", temp_dir.string());

    ConfigManager manager("missing.json");
    const Config& config = manager.get_config();
    const fs::path user_config_path = home_dir / ".config" / "livewallpaper" / "config.json";

    EXPECT_TRUE(fs::exists(user_config_path));
    EXPECT_DOUBLE_EQ(config.latitude, 51.5);
    EXPECT_DOUBLE_EQ(config.longitude, -0.12);
    EXPECT_EQ(config.weather_update_interval, 8);
    EXPECT_EQ(config.fps, 24);
    EXPECT_EQ(config.particle_count, 42);
    EXPECT_EQ(config.particle_lifetime, 750);
}

TEST(ConfigManagerTest, ThrowsForInvalidJson) {
    const fs::path config_path = unique_temp_path("invalid_json") / "config.json";
    write_file(config_path, R"({
        "latitude": 55.0,
    })");

    EXPECT_THROW(ConfigManager(config_path.string()), std::exception);
}

TEST(ConfigManagerTest, UsesDefaultsForMissingJsonFields) {
    const fs::path config_path = unique_temp_path("defaults") / "config.json";
    write_file(config_path, R"({
        "latitude": 1.25
    })");

    ConfigManager manager(config_path.string());
    const Config& config = manager.get_config();

    EXPECT_DOUBLE_EQ(config.latitude, 1.25);
    EXPECT_DOUBLE_EQ(config.longitude, 37.0);
    EXPECT_EQ(config.weather_update_interval, 10);
    EXPECT_EQ(config.fps, 30);
    EXPECT_EQ(config.particle_count, 100);
    EXPECT_EQ(config.particle_lifetime, 1000);
}
