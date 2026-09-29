// =============================
// Core/Camera/OrbitCamera.h
// =============================
#pragma once
#include "Vector3.h"
#include "DxPlus/DxPlus.h"

class OrbitCamera
{
public:
    OrbitCamera() = default;

    // accessor
    const Vec3& GetEye() const { return eye; }
    const Vec3& GetTarget() const { return target; }
    const Vec3& GetUp() const { return up; }

    float GetMoveSpeed() const { return moveSpeed; }
    void SetMoveSpeed(float s) { moveSpeed = s; }

    void Reset();
    void BeginControl();
    void Update(float deltaTime);

    void SetFromLookAt(const Vec3& lookFrom, const Vec3& lookAt);

private:
    static constexpr float ROTATE_RAD_PER_PIXEL = DxPlus::PI * 2 / DxPlus::CLIENT_WIDTH;
    static constexpr float PITCH_MIN = DxPlus::Deg2Rad * -89;
    static constexpr float PITCH_MAX = DxPlus::Deg2Rad *  89;

    static constexpr float ZOOM_SPEED_PER_WHEEL = 4000.0f;
    static constexpr float DISTANCE_MIN = 50.0f;
    static constexpr float DISTANCE_MAX = 2000.0f;

    //static constexpr float MOVE_SPEED = 400.0f;

    Vec3 eye{ 600, 600, -600 };
    Vec3 target{ 0, 0, 0 };
    Vec3 up{ 0, 1, 0 };

    float yaw{ 0.0f };
    float pitch{ 0.0f };
    float distance{ 400.0f };

    float moveSpeed = 400.0f;
};
