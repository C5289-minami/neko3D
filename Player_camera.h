// =============================
// Player_camera.h
// =============================
#pragma once
#include "Vector3.h"

// ボス戦カメラの調整値。値を変えるとカメラの見え方が変わる。
struct PlayerCameraSettings
{
    float cameraDistance{ 900.0f }; // カメラまでの距離
    float cameraHeight{ 420.0f };   // 見たカメラの高さ
    float targetHeight{ 110.0f };   // ちょっと上のほうにカメラを挙げるやつ
    float followSpeed{ 5.0f };      // 1秒あたりの追従の強さ。大きいほど早く近づく
};

class Player_camera
{
public:
    Player_camera() = default;

    const Vec3& GetPosition() const { return cameraPosition; }
    const Vec3& GetEye() const { return cameraPosition; }
    const Vec3& GetTarget() const { return lookAtPosition; }
    const Vec3& GetUp() const { return upDirection; }

    void SetPosition(const Vec3& position) { cameraPosition = position; }
    void SetTarget(const Vec3& newLookAtPosition) { lookAtPosition = newLookAtPosition; }
    void Update(const Vec3& playerPosition, const Vec3& bossPosition, float deltaTime);
    void Reset();
    PlayerCameraSettings& GetSettings() { return settings; }
    const PlayerCameraSettings& GetSettings() const { return settings; }

private:
    Vec3 cameraPosition{ 400.0f, 400.0f, 400.0f };
    Vec3 lookAtPosition{ 0.0f, 0.0f, 0.0f };
    Vec3 upDirection{ 0.0f, 1.0f, 0.0f };
    PlayerCameraSettings settings{};
};
