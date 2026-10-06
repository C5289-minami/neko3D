// =============================
// 15_System/Player_camera.cpp
// =============================
#include "Player_camera.h"

void Player_camera::Update( const Vec3& playerPosition, const Vec3& bossPosition, float deltaTime)
{
    //差分を求めます。
    Vec3 directionFromBossToPlayer = playerPosition - bossPosition;
    directionFromBossToPlayer.y = 0.0f;

    //仮の向きを決める
    if (directionFromBossToPlayer.LengthSq() < 0.001f)
    {
        directionFromBossToPlayer = { 0.0f, 0.0f, 1.0f };
    }
    else
    {
        directionFromBossToPlayer = directionFromBossToPlayer.Normalized();
    }
	//カメラの目標位置を計算する
    const Vec3 targetCameraPosition = playerPosition
        + directionFromBossToPlayer * settings.cameraDistance
        + Vec3(0.0f, settings.cameraHeight, 0.0f);

    //カメラの注視点の目標位置を計算する
    const Vec3 targetLookAtPosition = (bossPosition + playerPosition) * 0.5f
        + Vec3(0.0f, settings.targetHeight, 0.0f);
    //カメラの追従
	const float movementRate = settings.followSpeed * deltaTime;// 1byou
	const float movementAmount = movementRate < 1.0f ? movementRate : 1.0f;// 1秒あたりの追従の強さ。
    cameraPosition += (targetCameraPosition - cameraPosition) * movementAmount;
    lookAtPosition += (targetLookAtPosition - lookAtPosition) * movementAmount;
}

void Player_camera::Reset()
{
    cameraPosition = { 400.0f, 400.0f, 400.0f };
    lookAtPosition = { 0.0f, 0.0f, 0.0f };
    upDirection = { 0.0f, 1.0f, 0.0f };
}
