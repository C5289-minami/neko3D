#pragma once
#pragma once

#include "Boss.h"
#include "BossDemoSkill.h"
#include "Scene.h"
#include "DrawableObject.h"
#include "Vector3.h"
#include "Stage.h"

class BossTestScene final : public Scene
{
public:
    explicit BossTestScene(class GameContext* context) : Scene(context), demoSkill_(boss) {}
    void Init() override;
    void Update(float deltaTime) override;
    void Render() const override;

private:

    Stage stage{};
    Boss boss{};
    BossDemoSkill demoSkill_;
    int fontHandle{ -1 };
    Vec3 cameraEye{ 0.0f, 250.0f, -650.0f };
    float yaw{};
    float pitch{};
    bool skillKeyWasDown_{};
};

