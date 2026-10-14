// =============================
// Gameplay/Actors/Stage.h
// =============================
#pragma once
#include <array>
#include <cstddef>
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

    void DrawDebugUI();
    bool SaveModelSettings();
    bool LoadModelSettings();

	// lifecycle
	void Init();
	void Reset();
	void ResetAlphaModel(const Vec3& playerPosition);
	void UpdateAlphaModel(float deltaTime, const Player& player);
	void Draw() const;
	void DrawAlphaModel() const;
	float GetAlphaModelOpacity() const { return alphaModel_.GetAlpha(); }

private:
    // íåÇ∆ë‰ç¿ÇÇ‹Ç∆ÇﬂÇƒä«óù

    std::array<ModelObject, 5> stageModels_{};
    std::string modelSettingsStatus_;

	ModelAlpha alphaModel_;
	ModelObject stageModel_; // âºÇ≈ÉfÉtÉH
	bool alphaModelReady_{};
};
