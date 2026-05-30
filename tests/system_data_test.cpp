#include "data/system_data.hpp"

#include <gtest/gtest.h>

TEST(SystemDataTest, CpuUsageIsWithinPercentRange) {
    const int cpu_usage = get_cpu_usage();

    EXPECT_GE(cpu_usage, 0);
    EXPECT_LE(cpu_usage, 100);
}

TEST(SystemDataTest, RepeatedCpuUsageCallsStayWithinPercentRange) {
    for (int i = 0; i < 5; ++i) {
        const int cpu_usage = get_cpu_usage();

        EXPECT_GE(cpu_usage, 0);
        EXPECT_LE(cpu_usage, 100);
    }
}

TEST(SystemDataTest, CurrentTimeIsWithinDayRange) {
    const int current_time = get_current_time();

    EXPECT_GE(current_time, 0);
    EXPECT_LT(current_time, 24 * 60 * 60);
}

TEST(SystemDataTest, RepeatedCurrentTimeCallsStayWithinDayRange) {
    for (int i = 0; i < 5; ++i) {
        const int current_time = get_current_time();

        EXPECT_GE(current_time, 0);
        EXPECT_LT(current_time, 24 * 60 * 60);
    }
}
