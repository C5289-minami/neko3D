#pragma once

#include "Camera.h"
#include "Vector3.h"
#include "CameraPose.h"
#include "CameraShakeType.h"

class CameraShake
{
public:
    // ‰¡—h‚ê
    void StartHorizontal(
        float duration = 0.25f,
        float strength = 20.0f,
        float frequency = 20.0f);

    // c—h‚ê
    void StartVertical(
        float duration = 0.25f,
        float strength = 20.0f,
        float frequency = 20.0f);

    // ‰¡{c‚Ì¬‡—h‚ê
    void StartMixed(
        float duration = 0.25f,
        float strength = 20.0f,
        float frequency = 20.0f);

    void Stop();
    bool IsActive() const { return active_; }

    // Œ»İ‚ÌƒJƒƒ‰p¨‚©‚çA¡‰ñ‚Ì—h‚ê—Ê‚ğæ“¾
    Vec3 Update(float deltaTime, const Camera::CameraPose& pose);

private:

    void Start(CameraShakeType type, float duration, float strength, float frequency);
    float GetWave(float time, float phase) const;

    CameraShakeType type_{ CameraShakeType::Mixed };

    float elapsed_{ 0.0f };
    float duration_{ 0.0f };
    float strength_{ 0.0f };
    float frequency_{ 20.0f };

    bool active_{ false };
};