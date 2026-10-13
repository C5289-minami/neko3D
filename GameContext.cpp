// =============================
// Core/GameContext.cpp
// =============================
#include "GameContext.h"
#include "DxConv.h"
#include "DxPlus/DxPlus.h"
#ifndef NDEBUG
#include "imgui.h"
#endif
//
//DebugSceneControls GameContext::GetDebugControls()
//{
//    return {
//        "game",
//        {
//            MakeDebugTransformTarget("game/player", "Player", player),
//            MakeDebugTransformTarget("game/boss", "Boss", boss)
//        },
//        DebugCameraControls{
//            [this] { return IsSceneViewActive(); },
//            [this] { return GetCameraPosition(); },
//            [this](const Vec3& position) { SetCameraPosition(position); },
//            [this] { ResetSceneCamera(); },
//            [this] { FocusSceneCameraOnPlayer(); }
//        },
//        [this] { return debugActorsPaused_; },
//        [this](bool paused) { debugActorsPaused_ = paused; },
//        transformInitialStatus_
//    };
//}

void GameContext::Init()
{
    stage.Init();
    player.Init();
    boss.Init();

    playerCamera.Reset();
#ifndef NDEBUG
    activeCamera_ = nullptr;
    wasFreeCameraToggleDown_ = false;
#endif
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
    //TransformSettings::ResetToDefaults(GetDebugControls().targets, transformInitialStatus_);
    stage.ResetAlphaModel(player.GetPosition());
    playerCamera.Reset();
#ifndef NDEBUG
    activeCamera_ = nullptr;
    freeCamera.Reset();
    wasFreeCameraToggleDown_ = false;
#endif
}

void GameContext::Update(float deltaTime)
{
    DxLib::SetBackgroundColor(bgRed, bgGreen, bgBlue);

#ifndef NDEBUG
    const bool toggleDown = DxLib::CheckHitKey(KEY_INPUT_LALT) != 0 &&
        DxLib::CheckHitKey(KEY_INPUT_RETURN) != 0;
    if (toggleDown && !wasFreeCameraToggleDown_)
    {
        if (activeCamera_)
            activeCamera_ = nullptr;
        else
        {
            freeCamera.Initialize(playerCamera.GetPosition(), playerCamera.GetTarget());
            activeCamera_ = &freeCamera;
        }
    }
    wasFreeCameraToggleDown_ = toggleDown;
    if (activeCamera_ == &freeCamera)
    {
        freeCamera.InputUpdate(deltaTime);
        return;
    }
#endif

    // 足場の中心 ボスが動いてもこの位置
    const Vec3 stageCenterPosition{ 0.0f, 0.0f, 0.0f };
    player.Update(deltaTime, stage, stageCenterPosition);
    stage.UpdateAlphaModel(deltaTime, player);
    boss.Update(deltaTime);
    Collision::UpdateModelContact(player.GetModelObject(), boss.GetModelObject());
    playerCamera.Update(player.GetPosition(), boss.GetPosition(), deltaTime);


    // Effect
	using namespace DxPlus::Input;
    if (GetButtonDown(PLAYER1) & BUTTON_L3)
    {
        effectManager.PlayHit(RM().GetEffect(ResourceKeys::Effect_Test),
            player.GetPosition() + Vec3::Up() * 100.0f, 
            player.GetYaw() + DX_PI_F);
    }
    effectManager.Update(deltaTime);
}

void GameContext::Draw() const
{
    //DxLib::SetCameraPositionAndTargetAndUpVec(
    //    DxConv::ToVECTOR(GetCameraPosition()), // カメラの位置
    //    DxConv::ToVECTOR(GetCameraTarget()),   // カメラの注視点
    //    DxConv::ToVECTOR(GetCameraUp()));      //ちょっと上から

    DxLib::ClearDrawScreen();
#ifndef NDEBUG
    (activeCamera_ ? activeCamera_ : &playerCamera)->ApplyCamera();
#else
    playerCamera.ApplyCamera();
#endif
    grid.Draw();
    stage.Draw();
    boss.Draw();
    player.Draw();
    stage.DrawAlphaModel();


    effectManager.Draw();

}
