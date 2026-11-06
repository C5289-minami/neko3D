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
		const char* statusName;
		const wchar_t* trait;
	};

	constexpr std::array<SelectCard, 3> SelectCards{ {
		{ L"Select.Mike", ResourceKeys::SelectMike, u8"����", L"�o�����X�^" },
		{ L"Select.Sabi", ResourceKeys::SelectSabi, u8"����", L"����̂�����" },
		{ L"Select.Kuro", ResourceKeys::SelectKuro, u8"�䂤", L"�o��̂�����" }
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

		// Target����ʉE���ɔz�u���邽�߁A�����_���E�ւ��炷
		const Vec3 target =
			playerPos
			+ Vec3::Up() * SelectTargetH
			+ right * SelectTargetRightOffset;

		// �L�����N�^�[�̐��ʑ��ɃJ������z�u
		pose.position = target + forward * SelectCameraForwardDistance;

		// �J�������������ֈړ�
		pose.position -= right * SelectCameraLeftOffset;

		// Target�𒍎�
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

		// �J�[�h3����o�^
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

		// ����UI��o�^
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

	// �J�����X�V�͖��t���[��1�񂾂�
	const bool transitionCompleted =
		gameScene_.UpdateStateCamera(deltaTime, "Select");

	// =========================================
	// �J�������
	// =========================================

	switch (cameraState_)
	{
	case SelectCameraState::WaitSelectShot:
	{
		// Select�V���b�g�̊�����҂�
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
		// �J�����ړ��̊�����҂�
		if (transitionCompleted && !director.IsTransitioning())
		{
			cameraState_ = SelectCameraState::Ready;

			UpdateSelectedUI();

			// �J�����ړ�������Ɍ��肷��
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
	// ���͎擾
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

	// �ړ����ɓ��͂��󂯕t���Ȃ��Ă��A������Ԃ͍X�V����
	aWasDown_ = aDown;
	dWasDown_ = dDown;
	enterWasDown_ = enterDown;
	mouseWasDown_ = mouseDown;

	// �J�����ړ����͑I�𑀍���󂯕t���Ȃ�
	if (cameraState_ != SelectCameraState::Ready)
	{
		return;
	}

	// =========================================
	// A / D�őI��
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
	// Enter�Ō���
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

	// �J�[�h�Ƌ���UI����x��\���ɂ���
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
	// �L�������̏���
	const auto* status = CharSettingsManager::GetByName(card.statusName);
	selected.SetCharData(status ? *status : CharData{});//�����ݒ肪�����ꍇ�̓f�t�H���g��CharData���g��

	// Basket�Ƃ̐e�q�֌W������
	selected.ClearOwner();

	// �{�҂Ŏg�p����Player��؂�ւ���
	context->SetActivePlayer(selected);

	auto* player = context->GetPlayer();

	// �I����ʂŖ����ɂ����d�͂�L���ɖ߂�
	player->SetGravityEnabled(true);

	// Play�p�̃T�C�Y�Ɖ�]�ɂ���
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

	// ����UI���\��
	if (auto* ui = UIM().GetUI(L"Select.Common"))
		ui->SetTrue();
}

void GameSceneSelectState::Exit()
{
	DxPlus::Debug::SetFormatString(
		L"GameSceneSelectState::Exit");

	for (const auto& card : SelectCards)
	{
		if (auto* ui = UIM().GetUI(card.uiKey))
			ui->SetFalse();
	}

	if (auto* ui = UIM().GetUI(L"Select.Common"))
		ui->SetFalse();

	DxLib::SetMouseDispFlag(FALSE);
}