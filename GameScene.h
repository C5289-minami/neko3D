// =============================
// Scenes/Game/GameScene.h
// =============================
#pragma once
#include "Scene.h"

class GameScene final : public Scene
{
public:
    explicit GameScene(GameContext* context) : Scene(context) {}
    void Init() override {}
};
