// =============================
// Core/GameContext.cpp
// =============================
#include "GameContext.h"
#include "DxConv.h"
#include "DxPlus/DxPlus.h"

const Vec3& GameContext::GetCameraPosition() const
{
#ifndef NDEBUG
    if (Debug_camera.IsSceneViewActive()) return Debug_camera.GetEye();
#endif
    return playerCamera.GetPosition();
}

const Vec3& GameContext::GetCameraTarget() const
{
#ifndef NDEBUG
    if (Debug_camera.IsSceneViewActive()) return Debug_camera.GetTarget();
#endif
    return playerCamera.GetTarget();
}

const Vec3& GameContext::GetCameraUp() const
{
#ifndef NDEBUG
    if (Debug_camera.IsSceneViewActive()) return Debug_camera.GetUp();
#endif
    return playerCamera.GetUp();
}

bool GameContext::IsSceneViewActive() const
{
#ifndef NDEBUG
    return Debug_camera.IsSceneViewActive();
#else
    return false;
#endif
}

void GameContext::SetCameraPosition(const Vec3& position)
{
#ifndef NDEBUG
    if (Debug_camera.IsSceneViewActive())
    {
        Debug_camera.SetPosition(position);
        return;
    }
#endif
    playerCamera.SetPosition(position);
}

void GameContext::ResetSceneCamera()
{
    if (IsSceneViewActive())
    {
        Debug_camera.ResetView();
    }
    else
    {
        playerCamera.Reset();
        Debug_camera.Initialize(playerCamera.GetEye(), playerCamera.GetTarget());
    }
}

void GameContext::FocusSceneCameraOnPlayer()
{
#ifndef NDEBUG
    if (!Debug_camera.IsSceneViewActive())
    {
        Debug_camera.Initialize(playerCamera.GetEye(), playerCamera.GetTarget());
        Debug_camera.Begin();
    }
    Debug_camera.FocusAt(player.GetPosition());
#endif
}

void GameContext::Init()
{
    stage.Init();
    player.Init();

    playerCamera.Reset();
    Debug_camera.Initialize(playerCamera.GetEye(), playerCamera.GetTarget());
}

void GameContext::Reset()
{
    DxLib::SetDrawScreen(DX_SCREEN_BACK);
    DxLib::SetBackgroundColor(bgRed, bgGreen, bgBlue);

    const Vec3 lightDir{ -0.3f, -1.0f, -0.5f };
    DxLib::SetLightDirection(DxConv::ToVECTOR(lightDir));
    DxLib::SetGlobalAmbientLight(DxLib::GetColorF(0.35f, 0.35f, 0.35f, 1.0f));

    stage.Reset();
    player.Reset();
    playerCamera.Reset();
    Debug_camera.Initialize(playerCamera.GetEye(), playerCamera.GetTarget());
}

void GameContext::Update(float deltaTime)
{
    DxLib::SetBackgroundColor(bgRed, bgGreen, bgBlue);

#ifndef NDEBUG
    // 切り替え時は通常カメラの現在の位置・向きからデバッグ操作を開始する。
    Debug_camera.Update(deltaTime, player.GetPosition(),
        playerCamera.GetEye(), playerCamera.GetTarget());
    if (Debug_camera.IsSceneViewActive()) return;
#endif

    // 通常カメラは定点のまま。プレイヤーの座標・移動処理は維持する。
    player.Update(deltaTime, stage);
}

void GameContext::Draw() const
{
    DxLib::SetCameraPositionAndTargetAndUpVec(
        DxConv::ToVECTOR(GetCameraPosition()),
        DxConv::ToVECTOR(GetCameraTarget()),
        DxConv::ToVECTOR(GetCameraUp()));

    DxLib::ClearDrawScreen();
    grid.Draw();
    stage.Draw();
    player.Draw();
}
