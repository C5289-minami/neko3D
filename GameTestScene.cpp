// =============================
// Scenes/Game/GameTestScene.cpp
// =============================
#include "GameTestScene.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include "ResourceKeys.h"
#include "Collision.h"
#include "Consts.h"
#include "DxConv.h"
#include "DxLib.h"
#ifndef NDEBUG
#include "imgui.h"
#endif
#include "ToonSettings.h"
#include "OutlineSettings.h"

void GameTestScene::Init()
{
    gameContext->Reset();

#ifdef _DEBUG
    Collision::RegisterDebugCanWatch(
        this, *gameContext->GetPlayer(), gameContext->GetBoss());
#endif

    playerCamera_.Reset();
    attackSphereDebugRemaining_ = 0.0f;
    rightMouseWasDown_ = false;

#ifndef NDEBUG
    activeCamera_ = nullptr;
    freeCamera_.Reset();
    wasFreeCameraToggleDown_ = false;
#endif

    ToonSettingsManager::Load();
    OutlineSettingsManager::Load();

    if (!modelToonRenderer_.Init())
        DxPlus::Utils::FatalError(L"modelToonRendererの初期化失敗");

    if (!shaderEffects_.Init(
        DxPlus::CLIENT_WIDTH, DxPlus::CLIENT_HEIGHT))
    {
        DxPlus::Utils::FatalError(L"shaderEffectsの初期化失敗");
    }

    shaderEffects_.GetSettings().toonEnabled = true;
    shaderEffects_.GetSettings().outlineEnabled = true;

    StartFadeIn();
}
void GameTestScene::Update(float deltaTime)
{
	using namespace DxPlus::Input;

	DxLib::SetBackgroundColor(gameContext->GetBgRed(), gameContext->GetBgGreen(), gameContext->GetBgBlue());

#ifdef _DEBUG
	const bool rightMouseDown = (DxLib::GetMouseInput() & MOUSE_INPUT_RIGHT) != 0;
	
	if (rightMouseDown && !rightMouseWasDown_)
		attackSphereDebugRemaining_ = 1.0f;
	rightMouseWasDown_ = rightMouseDown;
	
	attackSphereDebugRemaining_ = (std::max)(0.0f,
		attackSphereDebugRemaining_ - (std::max)(0.0f, deltaTime));
#endif

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
		freeCamera_.FocusAt(gameContext->GetPlayer()->GetPosition());
	}
	DxPlus::Debug::SetString(L" R: Transformリセット, F: FreeCameraをプレイヤーにフォーカス");



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
		auto* player = gameContext->GetPlayer();
		auto& boss = gameContext->GetBoss();
		auto& stage = gameContext->GetStage();
		const Vec3 stageCenterPosition = stage.GetPosition();


		// player.Update(deltaTime, stage, stageCenterPosition);
		
#ifdef _DEBUG// Shift長押しで通常移動の速度を5倍にする（GameTest限定）
		const bool debugDash = DxLib::CheckHitKey(KEY_INPUT_LSHIFT) != 0 ||
			DxLib::CheckHitKey(KEY_INPUT_RSHIFT) != 0;
		player->SetDebugDashEnabled(debugDash);
#endif

		player->Update(deltaTime, stage.GetCollision(), stageCenterPosition);

#ifdef _DEBUG
		// この更新だけに適用し、共有するPlayerを他のシーンへ持ち越さない。
		player->SetDebugDashEnabled(false);
#endif

		stage.UpdateAlphaModel(deltaTime, *player);
		boss.Update(deltaTime);

#ifdef _DEBUG
		// 1を押したら、両手攻撃を最初から試す。
		if (GetButtonDown(PLAYER1) & BUTTON_1)
		{
			boss.StartHandAttack();
		}
		if (boss.UpdateHandAttack(deltaTime,
			player->GetWorldPosition(),
			stage.GetWorldPosition()))
		{
			player->TakeDamage();
		}
		auto& playerStateMachine = player->GetStateMachine();
		if (playerStateMachine.IsAttackBallActive()) // 攻撃球が飛んでいるか
		{
			const auto& attackBall = playerStateMachine.GetAttackBallSphere();
			const bool hitBossHand = boss.IsHitByAttackSphere(attackBall); // 左右どちらかの手に当たったか
			if (hitBossHand) // 当たった場合だけダメージ処理へ
			{
				// 同じ攻撃球で何度もHPが減らないよう、命中を一度だけ記録する。
				const bool firstHitForThisBall = playerStateMachine.TryRegisterAttackBallHit(); // この球の初回命中か
				if (firstHitForThisBall)
					boss.TakeDamage();
			}
		}
		DxPlus::Debug::SetFormatString(L"Boss HP: %d / %d", boss.GetHealth(), Boss::MaxHealth);
		DxPlus::Debug::SetString(L"1: Hand attack test (3 sec follow)");
#endif


		playerCamera_.Update(player->GetWorldPosition(), boss.GetWorldPosition(), deltaTime);

		if (GetButtonDown(PLAYER1) & BUTTON_L3)
		{
			gameContext->GetEffectManager().PlayHit(RM().GetEffect(ResourceKeys::Effect_Test),
				player->GetPosition() + Vec3::Up() * 100.0f,
				player->GetYaw() + DX_PI_F);
		}
		gameContext->GetEffectManager().Update(deltaTime);
	}

#ifndef NDEBUG
	// 数値入力中はシーン切り替えのキーを無視する
	if (ImGui::GetCurrentContext() && ImGui::GetIO().WantCaptureKeyboard) return;
#endif

	int buttonDown = GetButtonDown(PLAYER1);
	if (buttonDown & BUTTON_SELECT)
	{
		Scene* resultScene = SceneManager::GetInstance().GetScene(SceneID::Result);
		SetNextScene(resultScene);
		finished = true;    // フェード無しの場合は finished を true にしておく必要あり
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

#ifdef _DEBUG
	(activeCamera_ ? activeCamera_ : &playerCamera_)->ApplyCamera();
	gameContext->GetBoss().DrawHandAttackDebug();
#endif
}

void GameTestScene::DrawImGui()
{
#ifndef NDEBUG
    if (ImGui::Begin("Player Pillar Elevator"))
    {
        auto* player = gameContext->GetPlayer();

#ifdef _DEBUG
        ImGui::TextUnformatted(u8"Shift長押し：通常移動の速度を5倍（GameTest限定）");
#endif

        ImGui::Text("Pillar contact: %s", player->IsTouchingWall() ? "True" : "False");

        ImGui::TextUnformatted(u8"柱のエレベーター");
        float climbingSpeed = player->GetClimbingSpeed();
        if (ImGui::DragFloat(u8"上昇速度", &climbingSpeed, 10.0f, 0.0f, 5000.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp))
            player->SetClimbingSpeed(climbingSpeed);
        if (ImGui::Button(u8"上昇速度を初期値に戻す"))
            player->ResetClimbingSpeeds();
        ImGui::TextWrapped(u8"柱の上端 - 10で到着し、小さくジャンプします。");
   

		// Boss攻撃テスト
		ImGui::Separator();
		ImGui::TextUnformatted(u8"Boss Attack Test");

		auto& boss = gameContext->GetBoss();

		if (ImGui::Button(u8"HairBall"))
			boss.UseSkill(BossAttackType::HairBall);

		if (ImGui::Button(u8"Clap"))
			boss.UseSkill(BossAttackType::Clap);

		if (ImGui::Button(u8"Bite"))
			boss.UseSkill(BossAttackType::Bite);

		if (ImGui::Button(u8"TailWhip"))
			boss.UseSkill(BossAttackType::TailWhip);

        ImGui::TextWrapped(u8"設定は実行中のみ有効です。");
    }
    ImGui::End();
#endif
}

void GameTestScene::End()
{
#ifdef _DEBUG
    Collision::UnregisterDebugCanWatch(this);
#endif
}

void GameTestScene::RenderDepth() const
{
#ifndef NDEBUG
	(activeCamera_ ? activeCamera_ : &playerCamera_)->ApplyCamera();
#else
	playerCamera_.ApplyCamera();
#endif

	const auto& boss = gameContext->GetBoss();
	const auto* player = gameContext->GetPlayer();
	const auto& stage = gameContext->GetStage();
	const auto& room = gameContext->GetRoom();

	// RoomObj
	for (const auto* allRoom : room.GetAllModelObjects())
	{
		modelToonRenderer_.DrawDepth(*allRoom);
	}
	// Stage
	modelToonRenderer_.DrawDepth(stage.GetModelObject());
	modelToonRenderer_.DrawDepth(stage.GetAlphaModelObject());

	for (const auto& pillar : stage.GetPillarModelObjects())
	{
		modelToonRenderer_.DrawDepth(pillar);
	}

	for (const auto& model : stage.GetStageModelObjects())
	{
		modelToonRenderer_.DrawDepth(model);
	}

	// PlayerとBoss
	for (const auto& preview :
		gameContext->GetSelectPreviewPlayers())
	{
		modelToonRenderer_.DrawDepth(preview.GetModelObject());
	}

	modelToonRenderer_.DrawDepth(boss.GetModelObject());
}

void GameTestScene::DrawScene(bool toonEnabled) const
{
#ifndef NDEBUG
	(activeCamera_ ? activeCamera_ : &playerCamera_)->ApplyCamera();
#else
	playerCamera_.ApplyCamera();
#endif

	const auto& boss = gameContext->GetBoss();
	const auto* player = gameContext->GetPlayer();
	const auto& stage = gameContext->GetStage();
	const auto& effectManager = gameContext->GetEffectManager();
	const auto& grid = gameContext->GetGrid();
	const auto& room = gameContext->GetRoom();
	const auto& selectPlayers =
		gameContext->GetSelectPreviewPlayers();

	// トゥーンシェーダーを使わない場合
	if (!toonEnabled)
	{
		SetUsePixelShader(-1);
		SetUseVertexShader(-1);
		boss.Draw();
		stage.Draw();
		player->Draw();
		for (const auto& preview : selectPlayers)
		{
			preview.Draw();
		}
		stage.DrawAlphaModel();
		effectManager.Draw();
		grid.Draw();
		room.Draw();
		DrawAttackSphereDebug();
		return;
	}

	// トゥーン描画開始
	modelToonRenderer_.BeginPass();

	const auto drawModel = [this](const ModelObject& model)
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

	// BossとPlayer
	drawModel(boss.GetModelObject());
	drawModel(player->GetModelObject());


	for (const auto& preview : selectPlayers)
	{
		drawModel(preview.GetModelObject());
	}



	// Stage
	drawModel(stage.GetModelObject());
	drawModel(stage.GetAlphaModelObject());

	for (const auto& pillar : stage.GetPillarModelObjects())
	{
		drawModel(pillar);
	}

	for (const auto& model : stage.GetStageModelObjects())
	{
		drawModel(model);
	}

	// Room
	for (const auto* allRoom : room.GetAllModelObjects())
	{
		drawModel(*allRoom);
	}

	effectManager.Draw();
	grid.Draw();

	modelToonRenderer_.EndPass();
	player->GetStateMachine().Draw();
	DrawAttackSphereDebug();
}

void GameTestScene::DrawAttackSphereDebug() const
{
#ifdef _DEBUG
	if (attackSphereDebugRemaining_ <= 0.0f) return;

	const auto* player = gameContext->GetPlayer();
	const unsigned int color = DxLib::GetColor(0, 0, 0);
	DxLib::DrawSphere3D(
		DxConv::ToVECTOR(player->GetAttackBallSpawnPosition()),
		Const::PLAYER_ATTACK_BALL_RADIUS,
		6,
		color,
		color,
		false);
#endif
}

