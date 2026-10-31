#pragma once
#include "TutorialType.h"

struct TutorialObjective
{
    TutorialType type{};

    float progress{};
    float requiredProgress{ 1.0f };

    bool completed{};
};