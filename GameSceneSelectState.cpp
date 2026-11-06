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
		const wchar_t* uiKey;
		const wchar_t* spriteKey;
		const wchar_t* soundKey;
		const char* statusName;
		const wchar_t* trait;
	};

	constexpr std::array<SelectCard, 3> SelectCards{ {
		{ L"Select.Mike", ResourceKeys::SelectMike, ResourceKeys::Sound_SelectTaka, u8"たか", L"バランス型" },
		{ L"Select.Sabi", ResourceKeys::SelectSabi, ResourceKeys::Sound_SelectGishi, u8"ぎし", L"走るのが得意" },
		{ L"Select.Kuro", ResourceKeys::SelectKuro, ResourceKeys::Sound_SelectYu, u8"ゆう", L"登るのが得意" }
	} };

	class SelectCardUI : public UIBase
	{
	public:
		SelectCardUI(const SelectCard& card, DxPlus::Vec2 position)
			: UIBase(card.spriteKey, position, { 0, -1 }, SelectCameraMoveDuration)
		{
			trait_.SetText(card.trait);
			trait_.SetFontHandle(RM().GetFont(ResourceKeys::Font_Game));
			trait_.SetColor(DxLib::GetColor(64, 64, 64));
			trait_.SetOffset({ 0.0f, -202.0f });
		}

		void Draw() override
		{
			if (!IsActive()) return;

			UIBase::Draw();
			trait_.Draw(position, GetScale(), GetAlpha());
		}

	private:
		ButtonLabel trait_;
	};


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

		// カード3枚を登録
		for (const auto& card : SelectCards)
		{
			if (!UIM().GetUI(card.uiKey))
			{
				UIM().CreateImageUI(
					card.uiKey,
					card.spriteKey,
					position,
					{ 0, -1 },
					SelectCameraMoveDuration);
			}

			UIM().GetUI(card.uiKey)->SetFalse();
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

	// カードと共通UIを一度非表示にする
	for (const auto& card : SelectCards)
	{
		if (auto* ui = UIM().GetUI(card.uiKey))
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

void GameSceneSelectState::UpdateSelectedUI()
{
	for (std::size_t i = 0; i < SelectCards.size(); ++i)
	{
		if (auto* ui = UIM().GetUI(SelectCards[i].uiKey))
		{
			if (i == static_cast<std::size_t>(selectedIndex_))
				ui->SetTrue();
			else
				ui->SetFalse();
		}
	}

	// 共通UIも表示
	if (auto* ui = UIM().GetUI(L"Select.Common"))
		ui->SetTrue();
}

void GameSceneSelectState::Exit()
{
	DxPlus::Debug::SetFormatString(
		L"GameSceneSelectState::Exit");

	SoundManager::StopSE(RM().GetSound(SelectCards[selectedIndex_].soundKey));

	for (const auto& card : SelectCards)
	{
		if (auto* ui = UIM().GetUI(card.uiKey))
			ui->SetFalse();
	}

	if (auto* ui = UIM().GetUI(L"Select.Common"))
		ui->SetFalse();

	DxLib::SetMouseDispFlag(FALSE);
}
