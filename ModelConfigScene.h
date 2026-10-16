#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "Camera.h"
#include "DrawableObject.h"
#include "Scene.h"

#include <string>

class ModelConfigScene final : public Scene
{
public:
    explicit ModelConfigScene(GameContext* context) : Scene(context) {}
    void Init() override;
    void Update(float deltaTime) override;
    void Render() const override;
    void DrawImGui() override;

private:
    Camera::FreeCamera camera_;
    ModelObject* selected_{};
    std::string status_;
};
