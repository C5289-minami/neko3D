
#pragma once

#include <array>
#include <string>

#include "Vector3.h"
#include "ActorBase.h"
#include "DrawableObject.h"
#include "ModelCollision.h"
#include "Motion.h"

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

    // Lifecycle
    void Init() override;
    void Reset() override;
    void Update(float deltaTime) override;
    void Draw() const override;

    // Gimmick
    void StartShake();

private:
    void UpdateArenaBounds();
    void UpdateShake(float deltaTime);

    ModelCollision collision_;

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
