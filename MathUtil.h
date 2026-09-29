#pragma once
#include "DxPlus/DxPlus.h"

namespace MathUtil
{
    inline float NormalizeAngle(float angle)
    {
        while (angle >  DxPlus::PI) angle -= DxPlus::PI * 2.0f;
        while (angle < -DxPlus::PI) angle += DxPlus::PI * 2.0f;
        return angle;
    }

    inline float MoveAngleToward(float current, float target, float maxStep)
    {
        float diff = target - current;
        diff = NormalizeAngle(diff);
        diff = std::clamp(diff, -maxStep, maxStep);
        return NormalizeAngle(current + diff);
    }
}
