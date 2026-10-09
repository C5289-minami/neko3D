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
#include "Debug_camera.h"
#include "ShaderEffectController.h"

class BossTestScene final : public Scene
{
public:
    explicit BossTestScene(class GameContext* context)
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

    int fontHandle{ -1 };
    Vec3 cameraEye{ 0.0f, 250.0f, -650.0f };
    float yaw{};
    float pitch{};
    bool skillKeyWasDown_{};
    Debug_camera debugCamera_;
    bool debugActorsPaused_{};
    std::string transformInitialStatus_;

    // シーン固有のトゥーン描画
    int toonPixelShader_{ -1 };
    int toonVertexShader_{ -1 };
    ModelObject testModel_{};
    int toonConstantBuffer_{ -1 };
    Player test;
    ModelToonRenderer modelToonRenderer_{};

    // 共通のスクリーン空間アウトライン描画
  ShaderEffectController shaderEffects_{};

    void RenderSceneBuffer(bool toonEnabled) const;
    void RenderDepth() const;


    void ApplyCamera() const;
    void DrawToonCharacters(bool depthPass) const;
};
