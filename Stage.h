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
#include <vector>

class Stage
{
public:
	Stage() = default;

	// accessor
	int GetModelHandle() const { return RM().GetModel(stageModel_.modelKey); }
	ModelObject GetModelObject() const { return stageModel_; }
	ModelObject GetAlphaModelObject() const { return alphaModel_.model; }

    void GetCollisionModelHandles(std::vector<int>& handles) const;//modelの判定をまとめてやる

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
    void SetupCollisionInfo();
    void RefreshCollisionInfo();

    // 柱と台座をまとめて管理

    std::array<ModelObject, 5> stageModels_{};
    // 当たり判定に使えるモデルハンドル。未準備・失敗時は -1。
    std::array<int, 5> stageCollisionHandles_{ -1, -1, -1, -1, -1 };
    int groundCollisionHandle_{ -1 };
    std::string modelSettingsStatus_;

	ModelAlpha alphaModel_;
	ModelObject stageModel_; // 仮でデフォ
	bool alphaModelReady_{};
};
