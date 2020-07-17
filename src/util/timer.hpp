#ifndef __MODULE_TIMER_HPP__
#define __MODULE_TIMER_HPP__
#include <chrono>

namespace timing {
  using clock = std::chrono::steady_clock;
  using time_point = clock::time_point;
  using seconds = std::chrono::duration<double>;

  class Clock{
  public:
    Clock();
    seconds get() const;
  private:
    time_point epoch;
  };

  class Timer {
  public:
    Timer();

    seconds getDelta() const;

    void tick(seconds now);
  protected:
    seconds previous_time;
    seconds current_time;
    seconds delta_time;
  };

}


#endif // __MODULE_TIMER_HPP__
