#include "system_data.hpp"

#include <cstdio>
#include <ctime>
#include <fstream>
#include <string>

int get_current_time() {
    std::time_t t = std::time(nullptr);
    std::tm* local_time = std::localtime(&t);
    return local_time->tm_hour * 3600 + local_time->tm_min * 60 + local_time->tm_sec;
}

int get_cpu_usage() {
    struct CpuTimes {
        unsigned long long idle = 0;
        unsigned long long total = 0;
    };

    auto read_cpu_times = []() -> CpuTimes {
        std::ifstream stat_file("/proc/stat");
        std::string line;
        if (!std::getline(stat_file, line)) {
            return {};
        }

        unsigned long long user = 0;
        unsigned long long nice = 0;
        unsigned long long system = 0;
        unsigned long long idle = 0;
        unsigned long long iowait = 0;
        unsigned long long irq = 0;
        unsigned long long softirq = 0;
        unsigned long long steal = 0;

        int parsed = std::sscanf(
            line.c_str(),
            "cpu %llu %llu %llu %llu %llu %llu %llu %llu",
            &user,
            &nice,
            &system,
            &idle,
            &iowait,
            &irq,
            &softirq,
            &steal
        );

        if (parsed < 4) {
            return {};
        }

        unsigned long long idle_time = idle + iowait;
        unsigned long long active_time = user + nice + system + irq + softirq + steal;
        return {idle_time, idle_time + active_time};
    };

    static CpuTimes previous = {};
    static bool has_previous = false;
    static int last_usage = 0;

    CpuTimes current = read_cpu_times();
    if (current.total == 0) {
        return last_usage;
    }

    if (!has_previous) {
        previous = current;
        has_previous = true;
        return last_usage;
    }

    unsigned long long total_diff = current.total - previous.total;
    unsigned long long idle_diff = current.idle - previous.idle;
    previous = current;

    if (total_diff == 0) {
        return last_usage;
    }

    last_usage = static_cast<int>((total_diff - idle_diff) * 100 / total_diff);
    return last_usage;
}
