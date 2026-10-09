// =============================
// Core/GameContext.cpp
// =============================
#include "GameContext.h"
#include "DxConv.h"
#include "DxPlus/DxPlus.h"
#ifndef NDEBUG
#include "imgui.h"
#endif

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

DebugSceneControls GameContext::GetDebugControls()
{
    return {
        "game",
        {
            MakeDebugTransformTarget("game/player", "Player", player),
            MakeDebugTransformTarget("game/boss", "Boss", boss)
        },
        DebugCameraControls{
            [this] { return IsSceneViewActive(); },
            [this] { return GetCameraPosition(); },
            [this](const Vec3& position) { SetCameraPosition(position); },
            [this] { ResetSceneCamera(); },
            [this] { FocusSceneCameraOnPlayer(); }
        },
        [this] { return debugActorsPaused_; },
        [this](bool paused) { debugActorsPaused_ = paused; },
        transformInitialStatus_
    };
}

void GameContext::Init()
{
    stage.Init();
    player.Init();
    boss.Init();

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
    boss.Reset();
    debugActorsPaused_ = false;
    TransformSettings::ResetToDefaults(GetDebugControls().targets, transformInitialStatus_);
    stage.ResetAlphaModel(player.GetPosition());
    playerCamera.Reset();
    Debug_camera.Initialize(playerCamera.GetEye(), playerCamera.GetTarget());
}

void GameContext::Update(float deltaTime)
{
    DxLib::SetBackgroundColor(bgRed, bgGreen, bgBlue);

#ifndef NDEBUG
    Debug_camera.Update(deltaTime, player.GetPosition(),
        playerCamera.GetEye(), playerCamera.GetTarget());
    if (Debug_camera.IsSceneViewActive() || debugActorsPaused_ ||
        (ImGui::GetCurrentContext() && ImGui::GetIO().WantCaptureKeyboard))
    {
        stage.UpdateAlphaModel(deltaTime, player);
        return;
    }
#endif

    player.Update(deltaTime, stage);
    stage.UpdateAlphaModel(deltaTime, player);
    boss.Update(deltaTime);
    playerCamera.Update(player.GetPosition(), boss.GetPosition(), deltaTime);
}

void GameContext::Draw() const
{
    DxLib::SetCameraPositionAndTargetAndUpVec(
        DxConv::ToVECTOR(GetCameraPosition()), // カメラの位置
        DxConv::ToVECTOR(GetCameraTarget()),   // カメラの注視点
        DxConv::ToVECTOR(GetCameraUp()));      //ちょっと上から

    DxLib::ClearDrawScreen();
    grid.Draw();
    stage.Draw();
    boss.Draw();
    player.Draw();
    stage.DrawAlphaModel();
}
