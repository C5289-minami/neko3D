#pragma once
#pragma once
#include "Boss.h"
#include "Scene.h"
#include "DrawableObject.h"
#include "Vector3.h"

class BossTestScene final : public Scene
{
public:
    explicit BossTestScene(class GameContext* context) : Scene(context) {}
    void Init() override;
    void Update(float deltaTime) override;
    void Render() const override;

private:
    ModelObject stage{};
    Boss boss{};
    int fontHandle{ -1 };
    Vec3 cameraEye{ 0.0f, 250.0f, -650.0f };
    float yaw{};
    float pitch{};
};

