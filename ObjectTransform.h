#pragma once

#include "Vector3.h"

struct ObjectTransform
{
    Vec3 scale{ 1.0f, 1.0f, 1.0f };
    Vec3 position{};
    Vec3 rotation{};
};
