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
//#include <functional>

class Stage
{
public:
	Stage() = default;

	// accessor
	int GetModelHandle() const { return RM().GetModel(stageModel_.modelKey); }
	ModelObject GetModelObject() const { return stageModel_; }
	ModelObject GetAlphaModelObject() const { return alphaModel_.model; }

    void GetCollisionModelHandles(std::vector<int>& handles) const;//modelの判定をまとめてやる

    // プレイヤーのカプセルと柱L・M・Sのメッシュを判定する。
    bool IsHitPillar(const Vec3& capsuleStart, const Vec3& capsuleEnd, float radius) const;

    void DrawDebugUI();
    bool SaveModelSettings();
    bool LoadModelSettings();

	// lifecycle
	void Init();
	void Reset();
	void ResetAlphaModel(const Vec3& playerPosition);
	void UpdateAlphaModel(float deltaTime, const Player& player);
	void Draw() const;
    void ForEachModel(const std::function<void(const ModelObject&)>& draw) const;
	void DrawAlphaModel() const;
	float GetAlphaModelOpacity() const { return alphaModel_.GetAlpha(); }

private:
    void SetupCollisionInfo();
    void RefreshCollisionInfo();


    std::array<ModelObject, 3> stagePillars_{};				
    std::array<int, 3> pillarCollisionHandles_{ -1, -1, -1 };

 
    std::array<ModelObject, 2> stageModels_{};
    // 当たり判定に使えるモデルハンドル。未準備・失敗時は -1。
    std::array<int, 2> stageCollisionHandles_{ -1, -1 };
    int groundCollisionHandle_{ -1 };
    std::string modelSettingsStatus_;

	ModelAlpha alphaModel_;
	ModelObject stageModel_; // 仮でデフォ
	bool alphaModelReady_{};
    Vec3 alphaModelScale_{ 0.15f, 0.15f, 0.15f };
};
