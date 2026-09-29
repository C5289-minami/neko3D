// =============================
// Gameplay/Actors/Stage.h
// =============================
#pragma once
#include "Vector3.h"

class Stage
{
public:
	Stage() = default;

	// accessor
	int GetModelHandle() const { return modelHandle; }

	// lifecycle
	void Init();
	void Reset();
	void Draw() const;

private:
	int modelHandle{ -1 };
	Vec3 scale{ 1.0f, 1.0f, 1.0f };
};
