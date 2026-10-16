#pragma once

#include <string>

#include "Boss.h"
#include "BossBangSkill.h"
#include "BossBiteSkill.h"
#include "BossClapSkill.h"
#include "BossHairBallSkill.h"
#include "BossTailWhipSkill.h"
#include "Scene.h"
#include "DrawableObject.h"
#include "Vector3.h"
#include "Stage.h"
#include "Player.h"
#include "ModelToonRenderer.h"
#include "ShaderEffectController.h"
#include "Camera.h"

class ShaderTestScene final : public Scene
{
public:
    explicit ShaderTestScene(class GameContext* context)
        : Scene(context),
          bangSkill_(boss),
          tailWhipSkill_(boss),
          clapSkill_(boss),
          hairBallSkill_(boss),
          biteSkill_(boss)
    {
    }

    void Init() override;
    void Update(float deltaTime) override;
    void Render() const override;
    DebugSceneControls GetDebugControls() override;

private:
    Stage stage{};
    Boss boss{};
    BossBangSkill bangSkill_;
    BossTailWhipSkill tailWhipSkill_;
    BossClapSkill clapSkill_;
    BossHairBallSkill hairBallSkill_;
    BossBiteSkill biteSkill_;

  
    Vec3 cameraEye{ 0.0f, 250.0f, -650.0f };
    float yaw{};
    float pitch{};
    // Debug
    bool skillKeyWasDown_{};
  Camera::DebugCamera debugCamera_;
    bool debugActorsPaused_{};
    std::string transformInitialStatus_;

    // プレイヤー
    Player player_;

    // モデルのトゥーン描画を管理する
    ModelToonRenderer modelToonRenderer_{};
    // 共通のスクリーン空間アウトライン描画
    ShaderEffectController shaderEffects_{};
    void DrawScene(bool toonEnabled) const;
    void RenderDepth() const;

	Camera::FreeCamera camera_{};
};
