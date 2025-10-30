#include <chrono>

class Timer {
    public:
        Timer();

        void start();
        void reset();
        double tick();

    private:
        using Clock = std::chrono::high_resolution_clock;
        using TimePoint = Clock::time_point;
        using Seconds = std::chrono::duration<double>;

        TimePoint last_tick_time;
        bool is_running;
};
