#pragma once
#include <string>
#include "DxPlus/Vector2.h"
#include "Vector3.h"

struct ImageObject
{
    std::wstring imageKey;
    DxPlus::Vec2 position{};
    DxPlus::Vec2 scale{ 1.0f, 1.0f };
	DxPlus::Vec2 center{ 0.5f, 0.5f };
    float rotation{};

    void Draw() const;
};

struct ModelObject
{
    std::wstring modelKey;
    Vec3 position{};
    Vec3 scale{ 1.0f, 1.0f, 1.0f };
	Vec3 center{ 0.0f, 0.0f, 0.0f };
    Vec3 rotation{};

    void Draw() const;
};
