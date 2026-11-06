
#pragma once

#include <array>
#include <string>

#include "Vector3.h"
#include "ActorBase.h"
#include "DrawableObject.h"
#include "ModelCollision.h"
#include "Motion.h"
#include "Consts.h"

class Stage : public ActorBase
{
public:
    Stage();
    ~Stage() = default;

    // Accessor
    int GetModelHandle() const;

    ModelObject& GetModelObject() { return stageModel_; }
    const ModelObject& GetModelObject() const { return stageModel_; }

    const ModelObject& GetModelSize() const { return stageModel_; }

    std::array<ModelObject, 3>& GetPillarModelObjects() { return stagePillars_; }
    const std::array<ModelObject, 3>& GetPillarModelObjects() const { return stagePillars_; }

    std::array<ModelObject, 3>& GetStageModelObjects() { return stageModels_; }
    const std::array<ModelObject, 3>& GetStageModelObjects() const { return stageModels_; }

    ModelCollision& GetCollision() { return collision_; }
    const ModelCollision& GetCollision() const { return collision_; }

    float GetArenaRadius() const { return arenaRadius_; }
    float GetArenaSurfaceY() const { return arenaSurfaceY_; }

    Collision::Sphere GetBossDownHurtSphere() const;
    float GetBossDownHurtRadius() const { return bossDownHurtRadius_; }
    void SetBossDownHurtRadius(float radius);

    // ボス本体のカプセル。全高は上下の丸い部分も含む。
    struct BossBodySettings
    {
        float radius{ Const::BOSS_BODY_RADIUS };
        float height{ Const::BOSS_BODY_HEIGHT };
        float yOffset{ Const::BOSS_BODY_Y_OFFSET };
    };
    const BossBodySettings& GetBossBodySettings() const { return bossBodySettings_; }
    void SetBossBodySettings(const BossBodySettings& settings);
    void ResetBossBodySettings() { SetBossBodySettings({}); }

    // Lifecycle
    void Init() override;
    void Reset() override;
    void Update(float deltaTime) override;
    void Draw() const override;

    // Gimmick
    void StartShake();

private:
    void UpdateArenaBounds();
    void UpdateBossBodyCollision();
    void UpdateShake(float deltaTime);

    ModelCollision collision_;
    BossBodySettings bossBodySettings_{};
    float bossDownHurtRadius_{ Const::BOSS_DOWN_HURT_RADIUS };

    ModelObject stageModel_;
    std::array<ModelObject, 3> stagePillars_{};
    std::array<ModelObject, 3> stageModels_{};

    // 揺れ処理
    bool isShaking_{ false };
    bool isReturning_{ false };

    Motion tiltMotion_;

    // 初期回転を保持
    std::array<Vec3, 3> pillarBaseRotations_{};
    std::array<Vec3, 3> stageBaseRotations_{};
    std::array<Vec3, 3> pillarBasePositions_{};
    std::array<Vec3, 3> stageBasePositions_{};

    std::string modelSettingsStatus_;

    float HightSize{ 0.0f };
    float arenaRadius_{};
    float arenaSurfaceY_{};
};
