#include "data/weather.hpp"

#include <gtest/gtest.h>

TEST(WeatherDataTest, HasExpectedDefaults) {
    const WeatherData weather;

    EXPECT_DOUBLE_EQ(weather.temperature, 20.0);
    EXPECT_DOUBLE_EQ(weather.wind_speed, 0.0);
    EXPECT_DOUBLE_EQ(weather.wind_direction, 0.0);
}
