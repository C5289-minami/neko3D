// =============================
// Gameplay/Actors/Stage.h
// =============================
#pragma once
#include "Vector3.h"
#include "ModelAlpha.h"

class Stage
{
public:
	Stage() = default;

	// accessor
	int GetModelHandle() const { return modelHandle; }

	// lifecycle
	void Init();
	void Reset();
	void ResetAlphaModel(const Vec3& playerPosition);
	void UpdateAlphaModel(float deltaTime, const Player& player);
	void Draw() const;
	void DrawAlphaModel() const;
	float GetAlphaModelOpacity() const { return alphaModel_.GetAlpha(); }

private:
	int modelHandle{ -1 };
	Vec3 scale{ 1.0f, 1.0f, 1.0f };
	ModelAlpha alphaModel_;
	bool alphaModelReady_{};
};
