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
#include "UIManager.h"
#include "TwoFrameUIAnimator.h"
#include <cmath>

namespace
{
	struct PlayerHandUISetting
	{
		const wchar_t* normalUIId;
		const wchar_t* pressedUIId;

		DxPlus::Vec2 normalCenter;
		DxPlus::Vec2 pressedCenter;
	};

	constexpr std::array<PlayerHandUISetting, 3>
		PlayerHandUISettings{ {
		{
			L"MikeNormalHand",
			L"MikePressedHand",
			{ 0.5f, 0.5f },
			{ 0.5f, 0.5f }
		},
		{
			L"SabiNormalHand",
			L"SabiPressedHand",
			{ 0.5f, 0.5f },
			{ 0.5f, 0.5f }
		},
		{
			L"KuroNormalHand",
			L"KuroPressedHand",
			{ 0.5f, 0.5f },
			{ 0.5f, 0.5f }
		}
	} };

	std::size_t ToHandIndex(
		GameContext::PlayerHandType handType)
	{
		switch (handType)
		{
		case GameContext::PlayerHandType::Mike:
			return 0;

		case GameContext::PlayerHandType::Sabi:
			return 1;

		case GameContext::PlayerHandType::Kuro:
			return 2;

		default:
			return 0;
		}
	}
}
	

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
		ResourceKeys::Model_Player.mike,
		ResourceKeys::Model_Player.sabi,
		ResourceKeys::Model_Player.kuro
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

	// UIの初期化
	HandUIInit();
	
}

void GameContext::HandUIInit()
{
	const auto createImageUIIfNeeded =
		[](const wchar_t* id, const std::wstring& resourceKey)
		{
			if (UIM().GetUI(id))
			{
				return;
			}

			UIBase* ui = UIM().CreateImageUI(
				id,
				resourceKey,
				{ 0.0f, 0.0f }
			);

			// AnimatorがStartするまで非表示にする。
			if (ui)
			{
				ui->SetFalse();
			}
		};

	createImageUIIfNeeded(
		L"MikeNormalHand",
		ResourceKeys::PlayerHand.normal.mike
	);

	createImageUIIfNeeded(
		L"MikePressedHand",
		ResourceKeys::PlayerHand.pressed.mike
	);

	createImageUIIfNeeded(
		L"SabiNormalHand",
		ResourceKeys::PlayerHand.normal.sabi
	);

	createImageUIIfNeeded(
		L"SabiPressedHand",
		ResourceKeys::PlayerHand.pressed.sabi
	);

	createImageUIIfNeeded(
		L"KuroNormalHand",
		ResourceKeys::PlayerHand.normal.kuro
	);

	createImageUIIfNeeded(
		L"KuroPressedHand",
		ResourceKeys::PlayerHand.pressed.kuro
	);
}
TwoFrameUIAnimator*
GameContext::StartPlayerHandAnimation(
	const std::wstring& animatorId,
	PlayerHandType handType,
	const DxPlus::Vec2& anchorPosition,
	bool flipX)
{
	if (animatorId.empty())
	{
		return nullptr;
	}

	HandUIInit();

	const std::size_t handIndex =
		ToHandIndex(handType);

	if (handIndex >= PlayerHandUISettings.size())
	{
		return nullptr;
	}

	const auto& setting =
		PlayerHandUISettings[handIndex];

	UIBase* normalHand =
		UIM().GetUI(setting.normalUIId);

	UIBase* pressedHand =
		UIM().GetUI(setting.pressedUIId);

	if (!normalHand || !pressedHand)
	{
		return nullptr;
	}

	// ボタンや通常UIより手前に描画する。
	constexpr int HandZOrder = 100;

	normalHand->SetZOrder(HandZOrder);
	pressedHand->SetZOrder(HandZOrder);

	TwoFrameUIAnimator* animator =
		UIM().CreateTwoFrameAnimator(animatorId);

	if (!animator)
	{
		return nullptr;
	}

	animator->Stop();

	animator->SetUI(
		normalHand,
		pressedHand
	);

	animator->SetAnchorPosition(
		anchorPosition
	);

	TwoFrameUIAnimator::FrameSetting normalFrame;

	normalFrame.offset = {
		0.0f,
		0.0f
	};

	normalFrame.angle = 0.0f;
	normalFrame.scale = 1.0f;
	normalFrame.center = setting.normalCenter;
	normalFrame.flipX = flipX;

	TwoFrameUIAnimator::FrameSetting pressedFrame;

	pressedFrame.offset = {
		flipX ? -25.0f : 25.0f,
		-2.0f
	};

	pressedFrame.angle =
		(flipX ? -8.0f : 8.0f) *
		DxPlus::Deg2Rad;

	pressedFrame.scale = 1.0f;
	pressedFrame.center = setting.pressedCenter;
	pressedFrame.flipX = flipX;

	animator->SetFirstFrame(normalFrame);
	animator->SetSecondFrame(pressedFrame);

	animator->SetFirstHoldDuration(0.6f);
	animator->SetForwardDuration(0.15f);
	animator->SetSecondHoldDuration(0.12f);
	animator->SetBackwardDuration(0.2f);

	animator->Start();

	return animator;
}
TwoFrameUIAnimator*
GameContext::StartActivePlayerHandAnimation(
	const std::wstring& animatorId,
	const DxPlus::Vec2& anchorPosition)
{
	if (!activePlayer)
	{
		return nullptr;
	}

	const std::wstring& modelKey =
		activePlayer
		->GetModelObject()
		.GetModelKey();

	PlayerHandType handType =
		PlayerHandType::Mike;

	if (modelKey ==
		ResourceKeys::Model_Player.sabi)
	{
		handType = PlayerHandType::Sabi;
	}
	else if (modelKey ==
		ResourceKeys::Model_Player.kuro)
	{
		handType = PlayerHandType::Kuro;
	}

	return StartPlayerHandAnimation(
		animatorId,
		handType,
		anchorPosition
	);
}
void GameContext::StopPlayerHandAnimation(
	const std::wstring& animatorId)
{
	UIM().RemoveTwoFrameAnimator(
		animatorId
	);
}

void GameContext::PlacePlayerAtBattleStart()
{
	constexpr Vec3 TutorialSkipBattleOffset{ 0.0f, 0.0f, -1000.0f };
	Vec3 position = stage.GetWorldPosition() + TutorialSkipBattleOffset;
	position.y = stage.GetArenaSurfaceY() + Const::PLAYER_SKIN;
	activePlayer->SetStartPosition(position);
	activePlayer->ResetPosition();

	const Vec3 toBoss = boss.GetWorldPosition() - position;
	activePlayer->SetYaw(std::atan2(toBoss.x, toBoss.z));
}

void GameContext::FaceBossTowardPlayer()
{
	const Vec3 toPlayer = activePlayer->GetWorldPosition() - boss.GetWorldPosition();
	boss.SetYaw(std::atan2(toPlayer.x, toPlayer.z) + DX_PI_F - stage.GetWorldRotation().y);
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
