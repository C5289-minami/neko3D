// =============================
// 15_System/Player_camera.h
// =============================
#pragma once
#include "../Vector3.h"

class Player_camera
{
public:
    Player_camera() = default;

    const Vec3& GetPosition() const { return eye; }
    const Vec3& GetEye() const { return eye; }
    const Vec3& GetTarget() const { return target; }
    const Vec3& GetUp() const { return up; }

    void SetPosition(const Vec3& position) { eye = position; }
    void SetTarget(const Vec3& lookAt) { target = lookAt; }
    void Reset();

private:
    Vec3 eye{ 400.0f, 400.0f, 400.0f };
    Vec3 target{ 0.0f, 0.0f, 0.0f };
    Vec3 up{ 0.0f, 1.0f, 0.0f };
};
