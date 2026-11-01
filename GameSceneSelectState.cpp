#include "GameSceneSelectState.h"
#include "GameScene.h"
#include "GameStateMachine.h"
#include "GameContext.h"
#include "Player.h"
#include "ResourceKeys.h"
#include "DxConv.h"
#include "DxPlus/Debug.h"

#include <array>
#include <cmath>
#include <string>

namespace
{

	// 現在共有されているコードで確認できるモデルキー。
	// 3種類のキャラクターモデルがある場合は、それぞれのキーに変更する。



	constexpr float SelectTargetH = 180.0f;
	constexpr float SelectTargetRightOffset = 150.0f;
	constexpr float SelectCameraLeftOffset = 120.0f;
	constexpr float SelectCameraForwardDistance = 350.0f;
	constexpr float SelectCameraMoveDuration = 0.6f;

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

	auto* context = gameScene_.GetGameContext();

	players_.clear();

	auto& previews = context->GetSelectPreviewPlayers();

	for (std::size_t i = 0; i < previews.size(); ++i)
	{
		auto& preview = previews[i];

		preview.SetGravityEnabled(false);
		preview.SetDefMove(false);
		preview.StopMove();

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

			// マウスでキャラクターを選択した場合は、
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

	selectedIndex_ = index;

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

void GameSceneSelectState::Exit()
{
	DxPlus::Debug::SetFormatString(
		L"GameSceneSelectState::Exit");

	DxLib::SetMouseDispFlag(FALSE);
}