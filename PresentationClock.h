#pragma once
#include <algorithm>
#include <cmath>

// UI and camera use real time; only this clock advances the simulation.
class PresentationClock
{
public:
    void Reset() { paused_ = false; step_ = false; speed_ = 1.0f; }
    bool IsPaused() const { return paused_; }
    float GetSpeed() const { return speed_; }
    void SetPaused(bool paused) { paused_ = paused; step_ = false; }
    void TogglePaused() { SetPaused(!paused_); }
    void SetSpeed(float speed) { if (std::isfinite(speed)) speed_ = std::clamp(speed, 0.05f, 2.0f); }
    void Step() { paused_ = true; step_ = true; }
    float Advance(float realDelta)
    {
        if (step_) { step_ = false; return 1.0f / 60.0f; }
        if (paused_ || !std::isfinite(realDelta)) return 0.0f;
        return (std::max)(0.0f, realDelta) * speed_;
    }
private:
    bool paused_{};
    bool step_{};
    float speed_{ 1.0f };
};
