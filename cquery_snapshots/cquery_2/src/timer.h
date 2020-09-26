#pragma once

#include <chrono>
#include <string>

struct Timer {
  using Clock = std::chrono::high_resolution_clock;

  // Creates a new timer. A timer is always running.
  Timer();

  // Returns elapsed microseconds.
  long long ElapsedMicroseconds() const;
  // Returns elapsed microseconds and restarts/resets the timer.
  long long ElapsedMicrosecondsAndReset();
  // Restart/reset the timer.
  void Reset();
  // Resets timer and prints a message like "<foo> took 5ms"
  void ResetAndPrint(const std::string& message);

  // Raw start time.
  std::chrono::time_point<Clock> start;
};
