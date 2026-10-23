// =============================
// Scenes/Game/GameTestScene.cpp
// =============================
#include "GameTestScene.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include "ResourceKeys.h"
#include "Collision.h"
#ifndef NDEBUG
#include "imgui.h"
#endif
#include "ToonSettings.h"
#include "OutlineSettings.h"
void GameTestScene::Init()
{


	gameContext->Reset();
	playerCamera_.Reset();
#ifndef NDEBUG
	activeCamera_ = nullptr;
	freeCamera_.Reset();
	wasFreeCameraToggleDown_ = false;
#endif


	ToonSettingsManager::Load();
	OutlineSettingsManager::Load();

	// �g�D�[���`��ɕK�v�ȃV�F�[�_�[�Ɛݒ�o�b�t�@�[��������
	if (!modelToonRenderer_.Init())
		DxPlus::Utils::FatalError(L"modelToonRenderer�̏��������s");
	if (!shaderEffects_.Init(DxPlus::CLIENT_WIDTH, DxPlus::CLIENT_HEIGHT))
		DxPlus::Utils::FatalError(L"shaderEffects�̏��������s");

	// toon�`��ƃA�E�g���C���`���L���ɂ���
	shaderEffects_.GetSettings().toonEnabled = true;
	shaderEffects_.GetSettings().outlineEnabled = true;



	StartFadeIn();
}

void GameTestScene::Update(float deltaTime)
{
	DxLib::SetBackgroundColor(gameContext->GetBgRed(), gameContext->GetBgGreen(), gameContext->GetBgBlue());

#ifndef NDEBUG
	const bool toggleDown = DxLib::CheckHitKey(KEY_INPUT_LALT) != 0 &&
		DxLib::CheckHitKey(KEY_INPUT_RETURN) != 0;
	if (toggleDown && !wasFreeCameraToggleDown_)
	{
		if (activeCamera_)
			activeCamera_ = nullptr;
		else
		{
			freeCamera_.Initialize(playerCamera_.GetPosition(), playerCamera_.GetTarget());
			activeCamera_ = &freeCamera_;
		}
	}
	wasFreeCameraToggleDown_ = toggleDown;

	if (DxLib::CheckHitKey(KEY_INPUT_R) != 0)
	{
		freeCamera_.ResetView();
	}
	if (DxLib::CheckHitKey(KEY_INPUT_F) != 0)
	{
		freeCamera_.FocusAt(gameContext->GetPlayer().GetPosition());
	}
	DxPlus::Debug::SetString(L" R: Transform���Z�b�g, F: FreeCamera���v���C���[�Ƀt�H�[�J�X");



	if (activeCamera_ == &freeCamera_)
	{
		if (!freeCamera_.IsSceneViewActive() && (DxLib::GetMouseInput() & MOUSE_INPUT_RIGHT) != 0)
		{
			freeCamera_.Update(deltaTime);
		}
	}
	else
#endif
	{
		auto& player = gameContext->GetPlayer();
		auto& boss = gameContext->GetBoss();
		auto& stage = gameContext->GetStage();
		const Vec3 stageCenterPosition = stage.GetPosition();


		// player.Update(deltaTime, stage, stageCenterPosition);
		
		player.Update(deltaTime, stage.GetCollision(), stageCenterPosition);
		stage.UpdateAlphaModel(deltaTime, player);

		boss.Update(deltaTime);
		ModelObject playerModel = player.GetModelObject().WithParentTransform(
			player.GetPosition(), player.GetRotation());
		const bool touching = Collision::UpdateModelContact(playerModel, boss.GetModelObject());
		player.GetModelObject().collisionHighlighted = playerModel.collisionHighlighted;
		if (touching)
		{
			player.TakeDamage();
		}


		playerCamera_.Update(player.GetPosition(), boss.GetPosition(), deltaTime);

		using namespace DxPlus::Input;
		if (GetButtonDown(PLAYER1) & BUTTON_L3)
		{
			gameContext->GetEffectManager().PlayHit(RM().GetEffect(ResourceKeys::Effect_Test),
				player.GetPosition() + Vec3::Up() * 100.0f,
				player.GetYaw() + DX_PI_F);
		}
		gameContext->GetEffectManager().Update(deltaTime);
	}

#ifndef NDEBUG
	// ���l���͒��̓V�[���؂�ւ��̃L�[�𖳎�����
	if (ImGui::GetCurrentContext() && ImGui::GetIO().WantCaptureKeyboard) return;
#endif

	using namespace DxPlus::Input;
	int buttonDown = GetButtonDown(PLAYER1);
	if (buttonDown & BUTTON_SELECT)
	{
		Scene* resultScene = SceneManager::GetInstance().GetScene(SceneID::Result);
		SetNextScene(resultScene);
		finished = true;    // �t�F�[�h�����̏ꍇ�� finished �� true �ɂ��Ă����K�v����
		return;
	}
}

void GameTestScene::Render() const
{

	DxLib::ClearDrawScreen();
	if (shaderEffects_.IsEnabled())
	{
		shaderEffects_.Render(
			[this](bool toonEnabled) {
				DrawScene(toonEnabled);
			},
			[this] {
				RenderDepth();
			}
		);
	}
	else
	{
		DrawScene(false);
	}
}

void GameTestScene::End()
{
}

void GameTestScene::RenderDepth() const
{
#ifndef NDEBUG
	(activeCamera_ ? activeCamera_ : &playerCamera_)->ApplyCamera();
#else
	playerCamera_.ApplyCamera();
#endif

	const auto& boss = gameContext->GetBoss();
	const auto& player = gameContext->GetPlayer();
	const auto& stage = gameContext->GetStage();
	const auto& room = gameContext->GetRoom();

	const ModelObject playerModel = player.GetModelObject().WithParentTransform(
		player.GetPosition(), player.GetRotation());
	const ModelObject bossModel = boss.GetModelObject().WithParentTransform(
		boss.GetPosition(), boss.GetRotation());
	const ModelObject stageModel = stage.GetModelObject().WithParentTransform(
		stage.GetPosition(), stage.GetRotation());

	modelToonRenderer_.DrawDepth(bossModel);
	modelToonRenderer_.DrawDepth(stageModel);
	modelToonRenderer_.DrawDepth(stage.GetAlphaModelObject());

	for (const auto& pillar : stage.GetPillarModelObjects())
	{
		modelToonRenderer_.DrawDepth(
			pillar.WithParentTransform(stage.GetPosition(), stage.GetRotation()));
	}

	for (const auto& model : stage.GetStageModelObjects())
	{
		modelToonRenderer_.DrawDepth(
			model.WithParentTransform(stage.GetPosition(), stage.GetRotation()));
	}

	modelToonRenderer_.DrawDepth(playerModel);
	modelToonRenderer_.DrawDepth(room.GetModelObject());
}

void GameTestScene::DrawScene(bool toonEnabled) const
{
#ifndef NDEBUG
	(activeCamera_ ? activeCamera_ : &playerCamera_)->ApplyCamera();
#else
	playerCamera_.ApplyCamera();
#endif

	const auto& boss = gameContext->GetBoss();
	const auto& player = gameContext->GetPlayer();
	const auto& stage = gameContext->GetStage();
	const auto& effectManager = gameContext->GetEffectManager();
	const auto& grid = gameContext->GetGrid();
	const auto& room = gameContext->GetRoom();

	// �g�D�[���V�F�[�_�[���g��Ȃ��ꍇ
	if (!toonEnabled)
	{
		SetUsePixelShader(-1);
		SetUseVertexShader(-1);
		boss.Draw();
		stage.Draw();
		player.Draw();
		stage.DrawAlphaModel();
		effectManager.Draw();
		grid.Draw();
		room.Draw();
		return;
	}

	// �g�D�[���`��J�n
	modelToonRenderer_.BeginPass();

	const auto drawActor = [this](const ModelObject& model)
		{
			if (model.collisionHighlighted)
			{
				SetUsePixelShader(-1);
				SetUseVertexShader(-1);
				MV1SetUseOrigShader(FALSE);
				model.Draw();
				MV1SetUseOrigShader(TRUE);
			}
			else
			{
				modelToonRenderer_.Draw(model);
			}
		};
	const ModelObject playerModel = player.GetModelObject().WithParentTransform(
		player.GetPosition(), player.GetRotation());
	drawActor(playerModel);
	// boss�̃��f����boss���̂̈ʒu�Ɖ�]��K�p���ĕ`�悷��
	drawActor(boss.GetModelObject().WithParentTransform(
		boss.GetPosition(), boss.GetRotation()));
	drawActor(stage.GetModelObject().WithParentTransform(
		stage.GetPosition(), stage.GetRotation()));
	drawActor(stage.GetAlphaModelObject());
	for (const auto& pillar : stage.GetPillarModelObjects())
	{
		drawActor(pillar.WithParentTransform(stage.GetPosition(), stage.GetRotation()));
	}
	for (const auto& model : stage.GetStageModelObjects())
	{
		drawActor(model.WithParentTransform(stage.GetPosition(), stage.GetRotation()));
	}
	effectManager.Draw();
	grid.Draw();

	drawActor(room.GetModelObject());
	modelToonRenderer_.EndPass();
}

