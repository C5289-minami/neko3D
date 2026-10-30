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
#include "CharSettings.h"

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
	modelObjects.Add(room_.GetRoomObject(), room_);

	for (auto& pillar : room_.GetPillars())
	{
		modelObjects.Add(*pillar, room_);
	}

	for (auto& jumpStand : room_.GetJumpStands())
	{
		modelObjects.Add(jumpStand->GetJumpStandModel(), *jumpStand);
		modelObjects.Add(jumpStand->GetPillarModel(), *jumpStand);

	}

	modelObjects.Add(room_.GetBasket().GetModelObject(), room_.GetBasket());

	CharSettingsManager::Load();

	for (std::size_t i = 0; i < selectPreviewPlayers_.size(); ++i)
	{
		auto& selectPlayer = selectPreviewPlayers_[i];
		const std::string suffix = std::to_string(i);

		// Player Actor‚Ìe‚ðBasket‚É‚·‚é
		selectPlayer.SetOwner(room_.GetBasket());

		selectPlayer.SetIdentifier("select_player_" + suffix);
		selectPlayer.GetModelObject().SetObjectId("select_player_" + suffix);
		selectPlayer.GetModelObject().SetObjectName("SelectPlayer" + suffix);

		// ModelObject‚Ìe‚ðPlayer Actor‚É‚·‚é
		modelObjects.Add(selectPlayer.GetModelObject(), selectPlayer);
	}

	stage.Init();
	player.Init();
	boss.Init();
	room_.Init();

	for (auto& preview : selectPreviewPlayers_)
	{
		preview.Init();
		preview.SetGravityEnabled(false);
		preview.SetDefMove(false);
	}
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
