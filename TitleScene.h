// =============================
// Scenes/Title/TitleScene.h
// =============================
#pragma once
#include "Scene.h"
#include "GameContext.h"

class TitleScene final : public Scene
{
public:
    explicit TitleScene(GameContext* context) : Scene(context) {}
    void Init() override;
    void Update(float deltaTime) override;
    void Render() const override;

#ifndef NDEBUG
	void DrawImGui() override;
#endif 
private:
};
