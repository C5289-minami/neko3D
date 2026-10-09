#pragma once

#include "DrawableObject.h"

class Player;

// Use a separate resource key/handle for models whose opacity can change.
class ModelAlpha
{
public:
    ModelObject model;

    float GetAlpha() const { return alpha_; }
    void SetAlpha(float alpha);
    void FadeTo(const Player& player, float speed, float deltaTime);
    void Draw() const;

private:
    float alpha_{ 1.0f };
};
