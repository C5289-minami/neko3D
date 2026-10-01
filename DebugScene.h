#pragma once
#pragma once
#include "Scene.h"
#include "SSPlayer/SS6Player.h"

class TestScene final : public Scene
{
public:
    explicit TestScene(GameContext* context) : Scene(context) {}
    void Init() override;
    void Update(float deltaTime) override;
    void Render() const override;

private:
    ss::Player* m_ssPlayer = nullptr;
    int flipTestMode = 0;
};

