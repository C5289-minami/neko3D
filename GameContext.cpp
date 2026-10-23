// =============================
// Core/GameContext.cpp
// =============================
#include "GameContext.h"
#include "DxConv.h"
#include "DxPlus/DxPlus.h"
#include "ModelObjectManager.h"
#ifndef NDEBUG
#include "imgui.h"
#endif


void GameContext::Init()
{
    auto& modelObjects = MObjs();
    modelObjects.Add(player.GetModelObject(), player);
    modelObjects.Add(stage.GetModelObject(), stage);
    for (auto& model : stage.GetPillarModelObjects())
        modelObjects.Add(model, stage);
    for (auto& model : stage.GetStageModelObjects())
        modelObjects.Add(model, stage);
    modelObjects.Add(boss.GetModelObject(), boss);
    modelObjects.Add(room_.GetModelObject(), room_);

    for (auto& jumpStand : room_.GetJumpStands())
    {
        modelObjects.Add(jumpStand->jumpStandModel_, room_);
        modelObjects.Add(jumpStand->pillarModel_, room_);
	}

    stage.Init();
    player.Init();
    boss.Init();
    room_.Init();
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
	room_.Reset();

    debugActorsPaused_ = false;
    //TransformSettings::ResetToDefaults(GetDebugControls().targets, transformInitialStatus_);
    stage.ResetAlphaModel(player.GetPosition());
}
