#include "timer.hpp"

Timer::Timer() : is_running(false) {}

void Timer::reset() { is_running = false; }

void Timer::start() {
    if (!is_running) {
        last_tick_time = Clock::now();
        is_running = true;
    }
}

double Timer::tick() {
    if (!is_running) {
        return 0.0;
    }

    TimePoint current_time = Clock::now();

    Seconds delta_time =
        std::chrono::duration_cast<Seconds>(current_time - last_tick_time);

    last_tick_time = current_time;

    return delta_time.count();
}
