#include "../PresentationClock.h"

#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace
{
    void Check(bool condition, const char* message)
    {
        if (!condition) throw std::runtime_error(message);
        std::cout << "PASS: " << message << '\n';
    }

    bool Near(float actual, float expected)
    {
        return std::abs(actual - expected) < 0.000001f;
    }

    void TestPauseAndResume()
    {
        PresentationClock clock;
        Check(Near(clock.Advance(0.02f), 0.02f), "Running uses elapsed real time");
        clock.SetSpeed(0.25f);
        Check(Near(clock.Advance(0.02f), 0.005f), "Slow motion scales simulation time");
        clock.SetPaused(true);
        float simulationTime = 0.0f;
        for (int frame = 0; frame < 120; ++frame)
            simulationTime += clock.Advance(0.02f);
        Check(clock.IsPaused() && simulationTime == 0.0f,
            "Pause prevents simulation advancing across repeated rendered frames");
        clock.TogglePaused();
        Check(!clock.IsPaused() && Near(clock.Advance(0.02f), 0.005f),
            "Resume retains the selected speed without accumulating paused time");
    }

    void TestSingleStep()
    {
        PresentationClock clock;
        for (float speed : { 0.05f, 0.25f, 1.0f, 2.0f })
        {
            clock.SetSpeed(speed);
            clock.Step();
            Check(clock.IsPaused() && Near(clock.Advance(0.5f), 1.0f / 60.0f),
                "A step advances exactly one 60 Hz frame regardless of speed and real time");
            Check(clock.Advance(0.5f) == 0.0f && clock.Advance(0.001f) == 0.0f,
                "A step is consumed once and leaves the simulation paused");
        }
        clock.Step();
        Check(Near(clock.Advance(0.0f), 1.0f / 60.0f),
            "A queued step works without elapsed real time");
        clock.Step();
        clock.SetPaused(false);
        Check(Near(clock.Advance(0.02f), 0.04f),
            "Resuming cancels a queued step instead of unexpectedly stepping");
        clock.Step();
        clock.SetPaused(true);
        Check(clock.Advance(0.02f) == 0.0f,
            "Explicit pause cancels a queued step");
    }

    void TestInvalidTimingAndSpeed()
    {
        PresentationClock clock;
        const float nan = std::numeric_limits<float>::quiet_NaN();
        const float infinity = std::numeric_limits<float>::infinity();
        clock.SetSpeed(0.5f);
        clock.SetSpeed(nan);
        clock.SetSpeed(infinity);
        Check(Near(clock.GetSpeed(), 0.5f), "Nonfinite speed preserves the previous valid speed");
        clock.SetSpeed(-2.0f);
        Check(Near(clock.GetSpeed(), 0.05f), "Negative speed clamps to the minimum");
        clock.SetSpeed(0.0f);
        Check(Near(clock.GetSpeed(), 0.05f), "Zero speed clamps to the minimum; pause is explicit");
        clock.SetSpeed(20.0f);
        Check(Near(clock.GetSpeed(), 2.0f), "Excessive speed clamps to the maximum");
        Check(clock.Advance(-1.0f) == 0.0f && clock.Advance(nan) == 0.0f &&
            clock.Advance(infinity) == 0.0f,
            "Negative and nonfinite elapsed times cannot advance or corrupt simulation time");
        Check(Near(clock.Advance(0.02f), 0.04f), "Valid timing still works after invalid inputs");
    }

    void TestReset()
    {
        PresentationClock clock;
        clock.SetSpeed(0.25f);
        clock.Step();
        clock.Reset();
        Check(!clock.IsPaused() && Near(clock.GetSpeed(), 1.0f) &&
            Near(clock.Advance(0.02f), 0.02f),
            "Reset clears pause and pending step and restores normal speed");
    }
}

int main()
{
    try
    {
        TestPauseAndResume();
        TestSingleStep();
        TestInvalidTimingAndSpeed();
        TestReset();
    }
    catch (const std::exception& error)
    {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
