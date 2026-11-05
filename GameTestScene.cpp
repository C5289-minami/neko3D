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

	tailWhipHitCapsule_ = {};


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

	wasTailWhipActive_ = false;

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
		stage.Update(deltaTime);

		// player.Update(deltaTime, stage, stageCenterPosition);

#ifdef _DEBUG// Shift長押しで通常移動の速度を5倍にする（GameTest限定）
		const bool debugDash = DxLib::CheckHitKey(KEY_INPUT_LSHIFT) != 0 ||
			DxLib::CheckHitKey(KEY_INPUT_RSHIFT) != 0;
		player->SetDebugDashEnabled(debugDash);
#endif

		player->Update(deltaTime, stage.GetCollision(), stageCenterPosition);
		for (auto& obj : stage.GetStageModelObjects())
		{
			player->CheckFloor(obj, stage.GetModelObject());
		}
		for (auto& pillar : stage.GetPillarModelObjects())
		{
			ModelObject deltaPillar = pillar;
			deltaPillar.SetPosition({ 0.0f,deltaPillar.GetPosition().y * 2,0.0f }); // 無理やりだけど、Centerが中心なのでその分あげて上の判定あげてる
			player->CheckFloor(deltaPillar, stage.GetModelObject());

		}
	
#ifdef _DEBUG
		// この更新だけに適用し、共有するPlayerを他のシーンへ持ち越さない。
		player->SetDebugDashEnabled(false);
#endif
		// カメラ透過

		Collision::ResetTransparentModels();// まずリセットしてから、透過対象を追加する

		Collision::SetModelsBetweenTransparent(
			playerCamera_.GetPosition(),
			player->GetWorldPosition() + Vec3::Up() * 30.0f,
			gameContext->GetStage().GetCollision(),
			0.7f
		);

		Collision::SetModelsBetweenTransparent(
			playerCamera_.GetPosition(),
			boss.GetWorldPosition() + Vec3::Up() * 600.0f,// 顔が見たいので少し上
			gameContext->GetStage().GetCollision(),
			0.7f
		);



		boss.Update(deltaTime);
		boss.TurnToTarget(player->GetWorldPosition(), deltaTime);

		if (boss.UpdateSlamAttack(player->GetWorldPosition(), stage.GetWorldPosition()))
			player->TakeDamage();
		UpdateTailWhipGimmick();

		tailWhipHitCapsule_ = {};
		if (boss.IsTailWhipHitActive())
		{
			const Vec3 stageCenter = stage.GetWorldPosition();
			const float stageRadius = stage.GetArenaRadius();
			const float attackHeight = stage.GetArenaSurfaceY() + 30.0f;//床から＋30の高さで攻撃判定を作る

			tailWhipHitCapsule_ = boss.GetTailWhipHitCapsule(
				stageCenter,
				stageRadius,
				attackHeight);
		}

		const Collision::Sphere playerBody{ player->GetWorldPosition() + Vec3::Up() * 30.0f, 30.0f };

		bool tailWhipPlayerHit = boss.IsTailWhipHitActive() &&
			Collision::IsHitCapsuleSphere(tailWhipHitCapsule_, playerBody);

		if (tailWhipPlayerHit)
			player->TakeDamage();
		if (boss.UpdateHandAttack(deltaTime,
			player->GetWorldPosition(),
			stage.GetWorldPosition()))
		{
			player->TakeDamage();
		}

#ifdef _DEBUG
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
			gameContext->GetEffectManager().PlayHit(RM().GetEffect(ResourceKeys::Effect_PlayerAttack),
				player->GetPosition() + Vec3::Up() * 15.0f,
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
	
	


	// --- F2 でシェーダー全体のON/OFFを切り替える ---
	static bool shaderToggleWasDown = false;
	const bool shaderToggleDown =
		DxLib::CheckHitKey(KEY_INPUT_F2) != 0;

	if (shaderToggleDown && !shaderToggleWasDown)
	{
		shaderEffects_.SetEnabled(
			!shaderEffects_.IsEnabled()
		);
	}

	shaderToggleWasDown = shaderToggleDown;
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
	gameContext->GetBoss().DrawClapAttackDebug();
	gameContext->GetBoss().DrawSlamAttackDebug();

	auto& boss = gameContext->GetBoss();
	auto* player = gameContext->GetPlayer();
	// 攻撃中の尻尾カプセルを表示する。黄色は接触中、青は未接触。
	if (boss.IsTailWhipHitActive() && player != nullptr)
	{
		const Collision::Capsule& tail = tailWhipHitCapsule_;
		const Collision::Sphere playerBody{ player->GetWorldPosition() + Vec3::Up() * 30.0f, 30.0f };
		if (tail.radius > 0.0f)
		{
			const bool touching = Collision::IsHitCapsuleSphere(tail, playerBody);
			const unsigned int capsuleColor = touching ? DxLib::GetColor(255, 220, 60) : DxLib::GetColor(80, 220, 255);
			const unsigned int playerColor = touching ? capsuleColor : DxLib::GetColor(255, 120, 80);
			DxPlus::Debug::SetFormatString(L"TailWhip capsule hit: %s", touching ? L"true" : L"false");

			DxLib::SetUseZBuffer3D(FALSE);
			DxLib::SetWriteZBuffer3D(FALSE);
			Collision::DrawDebugCapsule(tail, capsuleColor);
			Collision::DrawDebugSphere(playerBody, playerColor);
			DxLib::SetUseZBuffer3D(TRUE);
			DxLib::SetWriteZBuffer3D(TRUE);
		}
		else
		{
			DxPlus::Debug::SetString(L"TailWhip capsule: inactive height-limited hitbox");
		}
	}
#endif
}

void GameTestScene::DrawImGui()
{
#ifndef NDEBUG

	auto toImGuiColor = [](unsigned int dxColor)
		{
			return ImVec4(
				static_cast<float>((dxColor >> 16) & 0xFF) / 255.0f,
				static_cast<float>((dxColor >> 8) & 0xFF) / 255.0f,
				static_cast<float>(dxColor & 0xFF) / 255.0f,
				1.0f
			);
		};

	if (ImGui::Begin("Debug"))

	{
		auto* player = gameContext->GetPlayer();

#ifdef _DEBUG
		ImGui::TextUnformatted(u8"Shift長押し：通常移動の速度を5倍（GameTest限定）");
#endif

		ImGui::Text("Pillar contact: %s", player->IsTouchingWall() ? "True" : "False");

		const float avoidTime = player->GetStateMachine().GetAvoidTime();
		if (avoidTime > 0.0f)
		{
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram,
				toImGuiColor(DxLib::GetColor(255, 0, 0)));
			ImGui::ProgressBar(avoidTime / PlayerDamagingState::AvoidTime, ImVec2(-1.0f, 0.0f), "Avoid 0.3s");
			ImGui::PopStyleColor();
		}

		const float avoidInvTime = player->GetAvoidInvTime();
		if (avoidInvTime > 0.0f)
		{
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram,
				toImGuiColor(DxLib::GetColor(255, 255, 0)));
			ImGui::ProgressBar(avoidInvTime / Player::AvoidInvTime, ImVec2(-1.0f, 0.0f), "Invincible 0.2s");
			ImGui::PopStyleColor();
		}

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

		if (ImGui::Button(u8"Boss.Reset"))
			boss.Reset();

		if (ImGui::Button(u8"Down (Flinch)"))
			boss.SetDownGauge(0.0f);

		if (ImGui::Button(u8"Die (成仏)"))
			boss.TakeDamage(999); // HPを0にして死亡させる

		if (ImGui::Button(u8"Clap"))
			boss.UseSkill(BossAttackType::Clap);

		if (ImGui::Button(u8"Slam"))
			boss.UseSkill(BossAttackType::Slam);

		if (ImGui::Button(u8"TailWhip"))
			boss.UseSkill(BossAttackType::TailWhip);


		auto& camera = playerCamera_;
		auto& settings = camera.GetSettings();
		if (ImGui::TreeNode(u8"TargetCamera Settings"))
		{
			ImGui::DragFloat(
				u8"カメラ距離",
				&settings.cameraDistance,
				10.0f,
				100.0f,
				3000.0f
			);

			ImGui::DragFloat(
				u8"カメラ高さ",
				&settings.cameraHeight,
				10.0f,
				-1000.0f,
				1000.0f
			);
			ImGui::TreePop();
		}
		ImGui::Checkbox(u8"bossAIEnabled", &boss.IsAIActive());
		// Stage Gimmick Test
		ImGui::Separator();
		ImGui::TextUnformatted(u8"Stage Gimmick Test");

		auto& stage = gameContext->GetStage();

		if (ImGui::Button(u8"Stage Shake"))
			stage.StartShake();
		if (ImGui::Button(u8"knock Back"))
			player->ApplyKnockback({ 1,0,0 }, 10000, 0.1f);

		ImGui::TextWrapped(u8"設定は実行中のみ有効です。");

		ImGui::End();
	}
#endif
}

void GameTestScene::End()
{
}

void GameTestScene::UpdateTailWhipGimmick()
{
	auto& boss = gameContext->GetBoss();
	auto& stage = gameContext->GetStage();
	auto* player = gameContext->GetPlayer();

	const bool isTailWhipActive = boss.IsTailWhipHitActive();

	if (isTailWhipActive && !wasTailWhipActive_)
	{
		stage.StartShake();

		Vec3 outward = player->GetWorldPosition() - stage.GetWorldPosition();
		outward.y = 0.0f;

		player->ApplyKnockback(outward, 5000.0f, 0.3f);
	}

	wasTailWhipActive_ = isTailWhipActive;
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

	// Room
	for (const auto* allRoom : room.GetAllModelObjects())
	{
		drawModel(*allRoom);
	}

	// Stage
	drawModel(stage.GetModelObject());

	for (const auto& pillar : stage.GetPillarModelObjects())
	{
		drawModel(pillar);
	}

	for (const auto& model : stage.GetStageModelObjects())
	{
		drawModel(model);
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

