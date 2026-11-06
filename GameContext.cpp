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
#include "ResourceKeys.h"
#include "ResourceManager.h"
#include "Sound3D.h"
#include "SoundManager.h"

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

 for (std::size_t i = 0; i < selectPreviewPlayers_.size(); ++i)
	{
     auto& preview = selectPreviewPlayers_[i];
		preview.Init();
		preview.SetGravityEnabled(false);
		preview.SetDefMove(false);

		if (i == 0)
			preview.PlayAnimationFromModel(ResourceKeys::Model_PlayerAnim_SelectMike);
		else if (i == 1)
			preview.PlayAnimationFromModel(ResourceKeys::Model_PlayerAnim_SelectSabi);
	}

	boss.SetPlayer(*activePlayer);
}

void GameContext::UpdateUltimateProjectiles(float deltaTime)
{
	playerUltimateEffect_.Update(*activePlayer, boss, stage, deltaTime);
}

void GameContext::ClearUltimateProjectiles()
{
	playerUltimateEffect_.Clear(*activePlayer);
}

void GameContext::UpdateTailWhipSound(const Collision::Capsule& hitCapsule)
{
	if (hitCapsule.radius <= 0.0f)
	{
		tailWhipSweepActive_ = false;
		return;
	}

	const int sound = RM().GetSound(ResourceKeys::Sound_TailWhip);
	if (!tailWhipSweepActive_)
	{
		const float hearingRadius = stage.GetArenaRadius() * 2.0f;
		SoundManager::PlaySE3D(sound, hitCapsule.end, hearingRadius);
		tailWhipSweepActive_ = true;
	}
	else
	{
		Sound3D::SetPosition(sound, hitCapsule.end);
	}
}

void GameContext::StopTailWhipSound()
{
	const int sound = RM().GetSound(ResourceKeys::Sound_TailWhip);
	if (sound >= 0 && DxLib::CheckSoundMem(sound) == 1)
		SoundManager::StopSE(sound);
	tailWhipSweepActive_ = false;
}

void GameContext::Reset()
{
	StopTailWhipSound();
	ClearUltimateProjectiles();
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
	boss.SetPlayer(*activePlayer);
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
