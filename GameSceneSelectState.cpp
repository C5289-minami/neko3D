#include "GameSceneSelectState.h"
#include "GameScene.h"
#include "GameStateMachine.h"
#include "GameContext.h"
#include "Player.h"
#include "ResourceKeys.h"
#include "DxConv.h"
#include "DxPlus/Debug.h"
#include "UIManager.h"
#include "ButtonLabel.h"
#include "ResourceManager.h"
#include "SoundManager.h"

#include <array>
#include <cmath>
#include <string>

namespace
{
	constexpr float SelectTargetH = 180.0f;
	constexpr float SelectTargetRightOffset = 150.0f;
	constexpr float SelectCameraLeftOffset = 120.0f;
	constexpr float SelectCameraForwardDistance = 350.0f;
	constexpr float SelectCameraMoveDuration = 0.6f;

	struct SelectCard
	{
		const wchar_t* centerUIKey;
		const wchar_t* leftUIKey;
		const wchar_t* rightUIKey;

		const wchar_t* centerSpriteKey;
		const wchar_t* leftSpriteKey;
		const wchar_t* rightSpriteKey;

		const wchar_t* soundKey;
		const char* statusName;
	};

	constexpr std::array<SelectCard, 3> SelectCards{ {
		{
			L"Select.Mike",
			L"Select.Left.Mike",
			L"Select.Right.Mike",
			ResourceKeys::SelectPlayers.mike,
			ResourceKeys::SelectAKeys.mike,
			ResourceKeys::SelectDKeys.mike,
			ResourceKeys::SoundSelectPlayers.mike,
			u8"たか"
		},
		{
			L"Select.Sabi",
			L"Select.Left.Sabi",
			L"Select.Right.Sabi",
			ResourceKeys::SelectPlayers.sabi,
			ResourceKeys::SelectAKeys.sabi,
			ResourceKeys::SelectDKeys.sabi,
			ResourceKeys::SoundSelectPlayers.sabi,
			u8"ぎし"
		},
		{
			L"Select.Kuro",
			L"Select.Left.Kuro",
			L"Select.Right.Kuro",
			ResourceKeys::SelectPlayers.kuro,
			ResourceKeys::SelectAKeys.kuro,
			ResourceKeys::SelectDKeys.kuro,
			ResourceKeys::SoundSelectPlayers.kuro,
			u8"ゆう"
		}
	} };
	class SelectCommonUI : public UIBase
	{
	public:
		SelectCommonUI(DxPlus::Vec2 position)
			: UIBase(ResourceKeys::SelectCommon, position, { 0, -1 }, SelectCameraMoveDuration)
		{
			this->SetCenter({ 0.5f, 0.5f });
		}
	};

	Camera::CameraPose MakeCameraPoseInFrontOfPlayer(
		Camera::CameraPose pose,
		const Player& player)
	{
		const Vec3 playerPos = player.GetWorldPosition();
		const float yaw = player.GetYaw();

		const Vec3 forward{
			std::sin(yaw),
			0.0f,
			std::cos(yaw)
		};

		const Vec3 right{
			std::cos(yaw),
			0.0f,
			-std::sin(yaw)
		};

		// Targetを画面右側に配置するため、注視点を右へずらす
		const Vec3 target =
			playerPos
			+ Vec3::Up() * SelectTargetH
			+ right * SelectTargetRightOffset;

		// キャラクターの正面側にカメラを配置
		pose.position = target + forward * SelectCameraForwardDistance;

		// カメラを少し左へ移動
		pose.position -= right * SelectCameraLeftOffset;

		// Targetを注視
		const Vec3 direction = target - pose.position;

		pose.yaw = std::atan2(direction.x, direction.z);
		pose.pitch = std::atan2(
			direction.y,
			std::sqrt(
				direction.x * direction.x +
				direction.z * direction.z
			)
		);

		return pose;
	}

}


GameSceneSelectState::GameSceneSelectState(
	GameStateMachine& gameStateMachine,
	GameScene& gameScene)
	: GameBaseState(gameStateMachine, gameScene)
{
}

void GameSceneSelectState::Enter()
{
	DxPlus::Debug::SetFormatString(L"GameSceneSelectState::Enter");

	// UI
	{
		const DxPlus::Vec2 position{
			DxPlus::CLIENT_WIDTH * 0.25f,
			DxPlus::CLIENT_HEIGHT * 0.5f
		};

		for (const auto& card : SelectCards)
		{
			if (!UIM().GetUI(card.centerUIKey))
			{
				UIM().CreateImageUI(
					card.centerUIKey,
					card.centerSpriteKey,
					position,
					{ 0.0f, -1.0f },
					SelectCameraMoveDuration
				);
			}
			if (auto* ui = UIM().GetUI(card.centerUIKey))
				ui->SetFalse();

		}
		// 描画順が追加順なので、左と右のUIは後に追加する
		// 中央の画像の幅を取得して、左右のUIの位置を決定する
		float offset{};
		auto* centerSprite = RM().GetSprite(SelectCards[0].centerSpriteKey);
		if (centerSprite)
		{
			DxLib::GetGraphSizeF(centerSprite->GetID(), &offset, {});
			offset *= 0.5f;
		}
		else
		{
			offset = 100.0f; // デフォルトのオフセット値
		}

		DxPlus::Vec2 rightPosition{
			position.x + offset,
			position.y
		};
		DxPlus::Vec2 leftPosition{
			position.x - offset,
			position.y
		};

		for (const auto& card : SelectCards)
		{
			if (!UIM().GetUI(card.leftUIKey))
			{
				UIM().CreateImageUI(
					card.leftUIKey,
					card.leftSpriteKey,
					leftPosition,
					{ 1.0f, 0.0f },
					SelectCameraMoveDuration
				);
			}

			if (!UIM().GetUI(card.rightUIKey))
			{
				UIM().CreateImageUI(
					card.rightUIKey,
					card.rightSpriteKey,
					rightPosition,
					{ -1.0f, 0.0f },
					SelectCameraMoveDuration
				);
			}

			if (auto* ui = UIM().GetUI(card.leftUIKey))
				ui->SetFalse();

			if (auto* ui = UIM().GetUI(card.rightUIKey))
				ui->SetFalse();
		}

		// 共通UIを登録
		if (!UIM().GetUI(L"Select.Common"))
		{
			UIM().RegisterUI(
				L"Select.Common",
				std::make_unique<SelectCommonUI>(
					position + DxPlus::Vec2{ 0.0f, 220.0f }));
		}

		UIM().GetUI(L"Select.Common")->SetFalse();
	}

	auto* context = gameScene_.GetGameContext();
	players_.clear();
	auto& previews = context->GetSelectPreviewPlayers();


	for (std::size_t i = 0; i < previews.size(); ++i)
	{
		auto& preview = previews[i];

		preview.SetGravityEnabled(false);
		preview.SetDefMove(false);
		preview.StopMove();

		if (i == 0)
			preview.PlayAnimationFromModel(ResourceKeys::Model_PlayerAnim_SelectMike);
		else if (i == 1)
			preview.PlayAnimationFromModel(ResourceKeys::Model_PlayerAnim_SelectSabi);

		players_.push_back(&preview);
	}

	selectedIndex_ = 1;
	cameraState_ = SelectCameraState::WaitSelectShot;
	confirmAfterCameraMove_ = false;

	aWasDown_ = false;
	dWasDown_ = false;
	enterWasDown_ = false;
	mouseWasDown_ = false;

	DxLib::SetMouseDispFlag(TRUE);
}

void GameSceneSelectState::Tick(float deltaTime)
{
	DxPlus::Debug::SetFormatString(
		L"GameSceneSelectState::Tick");


	if (players_.empty()) return;

	for (auto* player : players_)
		player->UpdateAnimation(deltaTime);

	auto& director = gameScene_.GetCameraDirector();

	// カメラ更新は毎フレーム1回だけ
	const bool transitionCompleted =
		gameScene_.UpdateStateCamera(deltaTime, "Select");

	// =========================================
	// カメラ状態
	// =========================================

	switch (cameraState_)
	{
	case SelectCameraState::WaitSelectShot:
	{
		// Selectショットの完了を待つ
		if (transitionCompleted && !director.IsTransitioning())
		{
			SoundManager::PlaySE(RM().GetSound(SelectCards[selectedIndex_].soundKey));//選択時のSEを再生

			const auto pose = MakeCameraPoseInFrontOfPlayer(
				director.GetCurrentPose(),
				*players_[selectedIndex_]);

			director.MoveTo(
				pose,
				SelectCameraMoveDuration,
				Motion::Easing::EaseInOut);

			cameraState_ = SelectCameraState::MoveToSelectedPlayer;
		}
		break;
	}

	case SelectCameraState::MoveToSelectedPlayer:
	{
		// カメラ移動の完了を待つ
		if (transitionCompleted && !director.IsTransitioning())
		{
			cameraState_ = SelectCameraState::Ready;

			UpdateSelectedUI();

			// カメラ移動完了後に決定する
			if (confirmAfterCameraMove_)
			{
				confirmAfterCameraMove_ = false;
				ConfirmSelection();
				return;
			}
		}
		break;
	}

	case SelectCameraState::Ready:
		break;
	}

	// =========================================
	// 入力取得
	// =========================================

	const bool aDown =
		DxLib::CheckHitKey(KEY_INPUT_A) != 0;

	const bool dDown =
		DxLib::CheckHitKey(KEY_INPUT_D) != 0;

	const bool enterDown =
		DxLib::CheckHitKey(KEY_INPUT_RETURN) != 0;

	const bool mouseDown =
		(DxLib::GetMouseInput() & MOUSE_INPUT_LEFT) != 0;

	const bool aPressed = aDown && !aWasDown_;
	const bool dPressed = dDown && !dWasDown_;
	const bool enterPressed = enterDown && !enterWasDown_;
	const bool mousePressed = mouseDown && !mouseWasDown_;

	// 移動中に入力を受け付けなくても、押下状態は更新する
	aWasDown_ = aDown;
	dWasDown_ = dDown;
	enterWasDown_ = enterDown;
	mouseWasDown_ = mouseDown;

	// カメラ移動中は選択操作を受け付けない
	if (cameraState_ != SelectCameraState::Ready)
	{
		return;
	}

	// =========================================
	// A / Dで選択
	// =========================================

	if (aPressed)
	{
		SelectIndex(selectedIndex_ - 1);
		return;
	}

	if (dPressed)
	{
		SelectIndex(selectedIndex_ + 1);
		return;
	}

	// =========================================
	// Enterで決定
	// =========================================

	if (enterPressed)
	{
		ConfirmSelection();
	}
}

void GameSceneSelectState::SelectIndex(int index)
{
	if (players_.empty()) return;

	const int count = static_cast<int>(players_.size());
	index = (index % count + count) % count;

	if (index == selectedIndex_) return;

	SoundManager::StopSE(RM().GetSound(SelectCards[selectedIndex_].soundKey));
	selectedIndex_ = index;
	SoundManager::PlaySE(RM().GetSound(SelectCards[selectedIndex_].soundKey));
	for (const auto& card : SelectCards)
	{
		if (auto* ui = UIM().GetUI(card.centerUIKey))
			ui->Hide();

		if (auto* ui = UIM().GetUI(card.leftUIKey))
			ui->Hide();

		if (auto* ui = UIM().GetUI(card.rightUIKey))
			ui->Hide();
	}

	if (auto* ui = UIM().GetUI(L"Select.Common"))
		ui->Hide();

	auto& director = gameScene_.GetCameraDirector();

	auto pose = MakeCameraPoseInFrontOfPlayer(
		director.GetCurrentPose(),
		*players_[selectedIndex_]);

	director.MoveTo(
		pose,
		SelectCameraMoveDuration,
		Motion::Easing::EaseInOut);

	cameraState_ = SelectCameraState::MoveToSelectedPlayer;
}

void GameSceneSelectState::ConfirmSelection()
{
	if (players_.empty()) return;

	auto* context = gameScene_.GetGameContext();

	Player& selected = *players_[selectedIndex_];
	const auto& card = SelectCards[selectedIndex_];
	// キャラ名の処理
	const auto* status = CharSettingsManager::GetByName(card.statusName);
	selected.SetCharData(status ? *status : CharData{});//もし設定が無い場合はデフォルトのCharDataを使う

	// Basketとの親子関係を解除
	selected.ClearOwner();

	// 本編で使用するPlayerを切り替える
	context->SetActivePlayer(selected);

	auto* player = context->GetPlayer();

	// 選択画面で無効にした重力を有効に戻す
	player->SetGravityEnabled(true);

	// Play用のサイズと回転にする
	auto& selectModel = selected.GetModelObject();
	player->GetModelObject().SetRotation({});
	player->GetModelObject().SetScale({ 1.0f, 1.0f, 1.0f });
	player->GetModelObject().SetPosition({});

	stateMachine_.ChangeState(GameStateType::Tutorial);
}

void GameSceneSelectState::UpdateSelectedUI() const
{
	const int count = static_cast<int>(SelectCards.size());

	const int leftIndex =
		(selectedIndex_ - 1 + count) % count;

	const int rightIndex =
		(selectedIndex_ + 1) % count;

	for (std::size_t i = 0; i < SelectCards.size(); ++i)
	{
		if (auto* ui = UIM().GetUI(SelectCards[i].centerUIKey))
		{
			if (i == static_cast<std::size_t>(selectedIndex_))
				ui->Show();
			else
				ui->SetFalse();
		}

		if (auto* ui = UIM().GetUI(SelectCards[i].leftUIKey))
		{
			if (i == static_cast<std::size_t>(leftIndex))
				ui->Show();
			else
				ui->SetFalse();
		}

		if (auto* ui = UIM().GetUI(SelectCards[i].rightUIKey))
		{
			if (i == static_cast<std::size_t>(rightIndex))
				ui->Show();
			else
				ui->SetFalse();
		}
	}

	if (auto* ui = UIM().GetUI(L"Select.Common"))
		ui->Show();
}

void GameSceneSelectState::Exit()
{
	DxPlus::Debug::SetFormatString(
		L"GameSceneSelectState::Exit");

	SoundManager::StopSE(RM().GetSound(SelectCards[selectedIndex_].soundKey));

	for (const auto& card : SelectCards)
	{
		if (auto* ui = UIM().GetUI(card.centerUIKey))
			ui->SetFalse();

		if (auto* ui = UIM().GetUI(card.leftUIKey))
			ui->SetFalse();

		if (auto* ui = UIM().GetUI(card.rightUIKey))
			ui->SetFalse();
	}

	if (auto* ui = UIM().GetUI(L"Select.Common"))
		ui->SetFalse();

	DxLib::SetMouseDispFlag(FALSE);
}
