#pragma once

#include "DrawableObject.h"

// Use a separate resource key/handle for models whose opacity can change.
class ModelAlpha
{
public:
    ModelObject model;

    float GetAlpha() const { return alpha_; }
    void SetAlpha(float alpha);
    void FadeTo(float targetAlpha, float speed, float deltaTime);
    void Draw() const;

private:
    float alpha_{ 1.0f };
};
