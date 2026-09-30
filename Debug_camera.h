#pragma once
#include "Vector3.h"
class Debug_camera {
public:
 void Initialize(const Vec3& eye, const Vec3& target);
 void Begin();
 void Update(float deltaTime, const Vec3& focusPoint);
 void ResetView();
 void FocusAt(const Vec3& point, float distance = 350.0f);
 bool IsSceneViewActive() const { return active; }
 float GetMoveSpeed() const { return moveSpeed; }
 void SetMoveSpeed(float speed) { moveSpeed = speed; }
 const Vec3& GetEye() const { return eye; }
 const Vec3& GetTarget() const { return target; }
 const Vec3& GetUp() const { return up; }
private:
 void RebuildTarget();
 bool active{false}, wasToggleDown{false}, wasResetDown{false}, wasFocusDown{false};
 Vec3 eye{}, target{}, homeEye{}, homeTarget{}, up{0,1,0};
 float yaw{}, pitch{};
 float moveSpeed{500.0f};
};
