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
/*
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
        Collision::UpdateModelContact(player.GetModelObject(), boss.GetModelObject());
        stage.UpdateAlphaModel(deltaTime, player);
        return;
    }
#endif

    // ����̒��S �{�X�������Ă����̈ʒu
    const Vec3 stageCenterPosition{ 0.0f, 0.0f, 0.0f };
    player.Update(deltaTime, stage, stageCenterPosition);
    stage.UpdateAlphaModel(deltaTime, player);
    boss.Update(deltaTime);
    const bool touching = Collision::UpdateModelContact(player.GetModelObject(), boss.GetModelObject());
    if (touching) player.TakeDamage();
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
    DxLib::SetCameraPositionAndTargetAndUpVec(
        DxConv::ToVECTOR(GetCameraPosition()), // �J�����̈ʒu
        DxConv::ToVECTOR(GetCameraTarget()),   // �J�����̒����_
        DxConv::ToVECTOR(GetCameraUp()));      //������Əォ��

    DxLib::ClearDrawScreen();
    grid.Draw();
    stage.Draw();
    boss.Draw();
    player.Draw();
    stage.DrawAlphaModel();


    effectManager.Draw();
    player.DrawStatus();

*/
}
