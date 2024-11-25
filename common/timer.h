#pragma once

#include <chrono>
#include <functional>
#include <mutex>
#include <map>
#include <thread>

class my_timer_t {
    using cb_t_ = std::function<void()>;
    using clock_t_ = std::chrono::steady_clock;
    using float_duration_t_ = std::chrono::duration<float>;
    using time_point_t_ = std::chrono::time_point<clock_t_, float_duration_t_>;

    std::mutex m_;
    bool stop_flag_;
    float_duration_t_ internal_sleep_;
    std::thread worker_;

    std::multimap<time_point_t_, cb_t_> callbacks_;

public:
    using duration_t = float_duration_t_;
    using cb_t = cb_t_;

    my_timer_t(duration_t timer_resolution);
    ~my_timer_t();

    void init();
    void finish();
    void tick();
    void add(cb_t cb, duration_t expires_after);

};



