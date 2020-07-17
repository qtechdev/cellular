#include "timer.hpp"

timing::Clock::Clock() {
  epoch = clock::now();
}

timing::seconds timing::Clock::get() const {
  time_point now = clock::now();
  return seconds(now - epoch);
}

timing::Timer::Timer() : previous_time(0.0), current_time(0.0), delta_time(0.0) {}

timing::seconds timing::Timer::getDelta() const {
  return delta_time;
}

void timing::Timer::tick(seconds now) {
  current_time = now;
  delta_time = current_time - previous_time;
  previous_time = current_time;
}
