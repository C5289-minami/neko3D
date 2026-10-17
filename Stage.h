// =============================
// Gameplay/Actors/Stage.h
// =============================
#pragma once
#include <array>
#include <cstddef>
#include "Vector3.h"
#include "ActorBase.h"
#include "DrawableObject.h"
#include "ResourceManager.h"
#include <vector>
#include "Player.h"

class Stage : public ActorBase
{
public:
	Stage() = default;
	~Stage() = default;

	// accessor
	int GetModelHandle() const { return RM().GetModel(stageModel_.modelKey); }
	ModelObject GetModelObject() const { return stageModel_; }
	std::vector<ModelObject> GetPillarModelObjects() const { return { stagePillars_.begin(), stagePillars_.end() }; }
	std::vector<ModelObject> GetStageModelObjects() const { return { stageModels_.begin(), stageModels_.end() }; }
	ModelObject GetAlphaModelObject() const { return alphaModel_; }

    void GetCollisionModelHandles(std::vector<int>& handles) const;//modelの判定をまとめてやる

    // プレイヤーのカプセルと柱L・M・Sのメッシュを判定する。
    bool IsHitPillar(const Vec3& capsuleStart, const Vec3& capsuleEnd, float radius) const;

    void DrawDebugUI();

	// lifecycle
	void Init() override;
	void Reset()override;
	void Update(float deltaTime) override;
	void Draw() const override;

	void ResetAlphaModel(const Vec3& playerPosition);
	void UpdateAlphaModel(float deltaTime, const Player& player);
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

	ModelObject alphaModel_;
	ModelObject stageModel_;
	bool alphaModelReady_{};
};
