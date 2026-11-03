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
	modelObjects.Add(defaultPlayer.GetModelObject(), defaultPlayer);
	modelObjects.Add(stage.GetModelObject(), stage);
	for (auto& model : stage.GetPillarModelObjects())
		modelObjects.Add(model, stage);
	for (auto& model : stage.GetStageModelObjects())
		modelObjects.Add(model, stage);

	boss.SetOwner(stage);
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
	modelObjects.Add(room_.GetChairObject(), room_);
	modelObjects.Add(room_.GetBasket().GetModelObject(), room_.GetBasket());

	CharSettingsManager::Load();

	const std::array<std::wstring, 3> playerModelKeys
	{
		ResourceKeys::Model_PlayerMike,
		ResourceKeys::Model_PlayerSabi,
		ResourceKeys::Model_PlayerKuro
	};
	for (std::size_t i = 0; i < selectPreviewPlayers_.size(); ++i)
	{
		auto& selectPlayer = selectPreviewPlayers_[i];
		const std::string suffix = std::to_string(i);

		// Player Actorの親をBasketにする
		selectPlayer.SetOwner(room_.GetBasket());

		selectPlayer.GetModelObject().SetModelKey(playerModelKeys[i]);
		selectPlayer.SetIdentifier("select_player_" + suffix);
		selectPlayer.GetModelObject().SetObjectId("select_player_" + suffix);
		selectPlayer.GetModelObject().SetObjectName("SelectPlayer" + suffix);

		// ModelObjectの親をPlayer Actorにする
		modelObjects.Add(selectPlayer.GetModelObject(), selectPlayer);
	}

	stage.Init();
	boss.Init();
	room_.Init();
	activePlayer->Init();

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
	boss.Reset();
	room_.Reset();

	// 本編Playerを初期Playerに戻す
	activePlayer = &defaultPlayer;
	activePlayer->Reset();

	// プレビューPlayerを初期状態に戻す
	for (auto& preview : selectPreviewPlayers_)
	{
		preview.ResetOwner();
		preview.Reset();
		preview.SetGravityEnabled(false);
		preview.SetDefMove(false);
	}

	debugActorsPaused_ = false;
}
