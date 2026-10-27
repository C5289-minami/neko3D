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
	const std::array<std::wstring, 3> playerModelKeys
	{
		ResourceKeys::Model_Player,
		ResourceKeys::Model_Player,
		ResourceKeys::Model_Player
	};

	constexpr float SelectTargetHeight = 100.0f;
	constexpr int MouseHitHalfWidth = 130;
	constexpr int MouseHitHalfHeight = 200;

	// 選択中のキャラクターの前方にカメラを移動させる距離と時間
	constexpr float SelectCameraForwardDistance = 500.0f;// 手前に200
	constexpr float SelectCameraMoveDuration = 0.5f;

	Camera::CameraPose MakeCameraPoseInFrontOfPlayer(
		Camera::CameraPose pose,
		const Player& player)
	{
		const Vec3 playerPos = player.GetWorldPosition();
		const float yaw = player.GetWorldRotation().y;

		const Vec3 forward{
			std::sin(yaw),
			0.0f,
			std::cos(yaw)
		};

		// Playerの正面にカメラを配置
		pose.position = playerPos + forward * SelectCameraForwardDistance;
		pose.position.y = playerPos.y + SelectTargetHeight;

		// Playerの上半身付近を注視する
		const Vec3 target =
			playerPos + Vec3::Up() * SelectTargetHeight;

		const Vec3 direction = target - pose.position;

		// 注視点の方向からカメラの角度を計算
		pose.yaw = std::atan2(direction.x, direction.z);
		pose.pitch = std::atan2(
			direction.y,
			std::sqrt(
				direction.x * direction.x +
				direction.z * direction.z));

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

		preview.SetModelKey(playerModelKeys[i]);
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

	context->SetSelectPreviewVisible(true);

	auto& camera = gameScene_.GetCameraDirector();

	// 横揺れ
	camera.StartMixedShake(50.2f, 525.0f, 50.0f);

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
	// マウスで選択・決定
	// =========================================

	if (mousePressed)
	{
		int mouseX = 0;
		int mouseY = 0;

		DxLib::GetMousePoint(&mouseX, &mouseY);

		const int hitIndex =
			FindPlayerByMouse(mouseX, mouseY);

		if (hitIndex >= 0)
		{
			// すでに選択中のキャラクターなら即決定
			if (hitIndex == selectedIndex_)
			{
				ConfirmSelection();
				return;
			}

			// 選択対象を変更し、移動完了後に決定
			confirmAfterCameraMove_ = true;
			SelectIndex(hitIndex);
			return;
		}
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

int GameSceneSelectState::FindPlayerByMouse(
	int mouseX,
	int mouseY) const
{
	for (std::size_t i = 0; i < players_.size(); ++i)
	{
		const Vec3 target =
			players_[i]->GetWorldPosition() +
			Vec3::Up() * SelectTargetHeight;

		const VECTOR screenPosition =
			DxLib::ConvWorldPosToScreenPos(
				DxConv::ToVECTOR(target));

		// カメラの描画範囲外ならスキップ
		if (screenPosition.z <= 0.0f ||
			screenPosition.z >= 1.0f)
		{
			continue;
		}

		const int screenX =
			static_cast<int>(screenPosition.x);

		const int screenY =
			static_cast<int>(screenPosition.y);

		if (std::abs(mouseX - screenX) <= MouseHitHalfWidth &&
			std::abs(mouseY - screenY) <= MouseHitHalfHeight)
		{
			return static_cast<int>(i);
		}
	}

	return -1;
}

void GameSceneSelectState::ConfirmSelection()
{
	if (players_.empty()) return;

	auto* context = gameScene_.GetGameContext();

	const Player& selected = *players_[selectedIndex_];
	Player& player = context->GetPlayer();

	// 選んだキャラクターの情報を本編Playerへ反映
	player.SetModelKey(selected.GetModelObject().GetModelKey());
	player.SetCharData(selected.GetCharData());

	context->SetSelectPreviewVisible(false);

	stateMachine_.ChangeState(GameStateType::Tutorial);
}

void GameSceneSelectState::Exit()
{
	DxPlus::Debug::SetFormatString(
		L"GameSceneSelectState::Exit");

	gameScene_.GetGameContext()->SetSelectPreviewVisible(false);

	DxLib::SetMouseDispFlag(FALSE);
}