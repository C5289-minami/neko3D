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
}
