#include "signal_handler.hpp"

#include <csignal>

std::atomic<bool> running(true);

static void signal_handler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        running = false;
    }
}

void setup_signal_handlers() {
    struct sigaction sa{};
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);
}