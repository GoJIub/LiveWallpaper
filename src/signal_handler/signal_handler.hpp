#ifndef SIGNAL_HANDLER_HPP
#define SIGNAL_HANDLER_HPP

#include <atomic>

extern std::atomic<bool> running;
void setup_signal_handlers();

#endif // SIGNAL_HANDLER_HPP