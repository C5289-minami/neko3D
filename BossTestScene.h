#pragma once

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


    // シェーダー
    int pixelShader{};
	int vertexShader{};
    int outlinePixelShader{};
    int outlineVertexShader{};
    bool isShaderEnabled_ = true;     // シェーダーのON/OFFフラグ
    bool shaderKeyWasDown_ = false;    // F2キーの入力判定用
	ModelObject testModel_{ };
    int outlineConstantBuffer_ = -1;
};

