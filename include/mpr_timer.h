#pragma once

namespace mpr {

class Timer {
 public:
  [[nodiscard]]
  static Timer& GetTimer();

  [[nodiscard]] float TotalTime() const;
  [[nodiscard]] float DeltaTime() const;
  void Reset();
  void Start();
  void Stop();
  void Tick();

 private:
  Timer();

  double secondsPerCount_{0};
  double deltaTime_{-1.0};
  __int64 baseTime_{0};
  __int64 pausedTime_{0};
  __int64 stopTime_{0};
  __int64 prevTime_{0};
  __int64 currTime_{0};

  bool bIsStopped_{false};
};
}  // namespace mpr
#define TIMER mpr::Timer::GetTimer()
