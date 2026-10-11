// =============================
// Gameplay/Actors/Stage.h
// =============================
#pragma once
#include "Vector3.h"
#include "ModelAlpha.h"
#include "DrawableObject.h"
#include "ResourceManager.h"

class Stage
{
public:
	Stage() = default;

	// accessor
	int GetModelHandle() const { return RM().GetModel(stageModel_.modelKey); }
	ModelObject GetModelObject() const { return stageModel_; }

	// lifecycle
	void Init();
	void Reset();
	void ResetAlphaModel(const Vec3& playerPosition);
	void UpdateAlphaModel(float deltaTime, const Player& player);
	void Draw() const;
	void DrawAlphaModel() const;
	float GetAlphaModelOpacity() const { return alphaModel_.GetAlpha(); }

private:
	ModelAlpha alphaModel_;
	ModelObject stageModel_; // ‰¼‚ÅƒfƒtƒH
	bool alphaModelReady_{};
};
