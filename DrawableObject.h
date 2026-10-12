#pragma once
#include <string>
#include "DxPlus/Vector2.h"
#include "Vector3.h"
#include "Collision.h"

struct ImageObject
{
	std::wstring imageKey;
	DxPlus::Vec2 position{};
	DxPlus::Vec2 scale{ 1.0f, 1.0f };
	DxPlus::Vec2 center{ 0.5f, 0.5f };
	float rotation{};

    DxPlus::Vec2 GetSize() const;
    void Draw(DxPlus::Vec2 masterPos = { 0.0f,0.0f }) const;
};

struct ModelObject
{
	std::wstring modelKey;
	Vec3 position{};
	Vec3 scale{ 1.0f, 1.0f, 1.0f };
	Vec3 center{ 0.0f, 0.0f, 0.0f }; // スケールと同期はできないが回転の中心をずらすために使用する
	Vec3 rotation{};

    bool collisionHighlighted{ false };

	//センターやスケールを考慮して、楕円体の当たり判定を返す　回転は無視
   Collision::Ellipsoid GetHitEllipsoid() const;
    Vec3 GetSize() const { return GetHitEllipsoid().radii * 2.0f; }
    void ApplyCollisionColor() const;
    void Draw(Vec3 masterPos = { 0.0f,0.0f,0.0f }) const;
};
