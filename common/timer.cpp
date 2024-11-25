#include "timer.h"

#include <iterator>
#include <mutex>
#include <thread>

my_timer_t::my_timer_t(duration_t timer_resolution)
: stop_flag_(false)
, internal_sleep_(timer_resolution)
, worker_()
, callbacks_()
{}

my_timer_t::~my_timer_t() {
    finish();
}

void my_timer_t::init() {
    worker_ = std::thread([this]{
        while (!stop_flag_) {
            tick();
            std::this_thread::sleep_for(internal_sleep_);
        }
    });
}

void my_timer_t::finish() {
    if (stop_flag_)
        return;

    stop_flag_ = true;
    worker_.join();

};

void my_timer_t::tick() {
    auto now = clock_t_::now();
    std::vector<cb_t_> expired_cb;

    {
        auto g = std::lock_guard<std::mutex>(m_);
        auto expired_timers = callbacks_.lower_bound(now);
        expired_cb.reserve(std::distance(callbacks_.begin(), expired_timers));

        for (auto i = callbacks_.begin(); i != expired_timers; ++i) {
            expired_cb.emplace_back(std::move(i->second));
        }
        callbacks_.erase(callbacks_.begin(), expired_timers);
    }

    for (auto cb: expired_cb)
        cb();
}

void my_timer_t::add(cb_t cb, duration_t expires_after) {
    auto now = clock_t_::now();
    auto g = std::lock_guard<std::mutex>(m_);
    callbacks_.emplace(now + expires_after, cb);
}


