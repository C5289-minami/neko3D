#pragma once

#include <string>

#include "Boss.h"
#include "BossBangSkill.h"
#include "BossBiteSkill.h"
#include "BossClapSkill.h"
#include "BossHairBallSkill.h"
#include "BossTailWhipSkill.h"
#include "Scene.h"
#include "GameContext.h"
#include "DrawableObject.h"
#include "Vector3.h"
#include "Stage.h"
#include "Player.h"
#include "ModelToonRenderer.h"
#include "ShaderEffectController.h"
#include "Camera.h"

class ShaderTest final : public Scene
{
public:
    explicit ShaderTest(class GameContext* context)
        : Scene(context),
          bangSkill_(context->GetBoss()),
          tailWhipSkill_(context->GetBoss()),
          clapSkill_(context->GetBoss()),
          hairBallSkill_(context->GetBoss()),
          biteSkill_(context->GetBoss())
    {
    }

    void Init() override;
    void Update(float deltaTime) override;
    void Render() const override;
	void DrawImGui() override;

private:
    BossBangSkill bangSkill_;
    BossTailWhipSkill tailWhipSkill_;
    BossClapSkill clapSkill_;
    BossHairBallSkill hairBallSkill_;
    BossBiteSkill biteSkill_;

  
    // Debug
    bool skillKeyWasDown_{};
    Camera::FreeCamera camera_{};
    bool debugActorsPaused_{};
    std::string transformInitialStatus_;


    // モデルのトゥーン描画を管理する
    ModelToonRenderer modelToonRenderer_{};
    // 共通のスクリーン空間アウトライン描画
    ShaderEffectController shaderEffects_{};
    void DrawScene(bool toonEnabled) const;
    void RenderDepth() const;

};
