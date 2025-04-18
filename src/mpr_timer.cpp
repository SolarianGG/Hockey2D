// clang-format off
#include "mpr_timer.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
// clang-format on

namespace mpr {

Timer::Timer() { 
  __int64 countsPerSec;
  QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&countsPerSec));
  secondsPerCount_ = 1.0 / static_cast<double>(countsPerSec);
}

Timer& Timer::GetTimer() {
  static Timer time{};
  return time;
}

float Timer::TotalTime() const { 
  if (bIsStopped_) [[unlikely]] {
    return static_cast<float>(((stopTime_ - pausedTime_) - baseTime_) *
                              secondsPerCount_);
  } else [[likely]] {
    return static_cast<float>(((currTime_ - pausedTime_) - baseTime_) *
                              secondsPerCount_);  
  }

}


float Timer::DeltaTime() const { return static_cast<float>(deltaTime_); }

void Timer::Reset() {
  QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currTime_));
  baseTime_ = currTime_;
  prevTime_ = currTime_;
  stopTime_ = 0;
  bIsStopped_ = false;
}

void Timer::Start() {
  if (!bIsStopped_) return;
  __int64 startTime;
  QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&startTime));

  pausedTime_ += (startTime - stopTime_);
  prevTime_ = startTime;
  stopTime_ = 0;
  bIsStopped_ = false;
}

void Timer::Stop() {
  if (bIsStopped_) return;

  QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currTime_));
  stopTime_ = currTime_;
  bIsStopped_ = true;
}

void Timer::Tick() {
  if (bIsStopped_) {
    deltaTime_ = 0.0;
    return;
  }
  QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currTime_));
  deltaTime_ = (currTime_ - prevTime_) * secondsPerCount_;
  prevTime_ = currTime_;
  if (deltaTime_ < 0.0) [[unlikely]] {
    deltaTime_ = 0.0;
  }
}
}  // namespace mpr
