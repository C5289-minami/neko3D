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
#include "CameraShakeType.h"

void GameTestScene::Init()
{
	Collision::ResetTransparentModels();
	gameContext->Reset();
	gameContext->PlacePlayerAtBattleStart();
	gameContext->FaceBossTowardPlayer();

	auto* player = gameContext->GetPlayer();
	auto& boss = gameContext->GetBoss();
	player->GetStateMachine().SetEventCallback({});
	player->SetGravityEnabled(true);
	player->SetDefMove(true);
	player->SetControlLocked(false);
	boss.SetAIActive(true);

	wasTouchingBossBody_ = false;

	tailWhipHitCapsule_ = {};



	attackSphereDebugRemaining_ = 0.0f;
	rightMouseWasDown_ = false;

	playerCamera_.Reset();
	bossDownCamera_.Reset();
	cameraDirector_.Reset();
	playerCamera_.SetFieldOfView(60.0f);
	playerCamera_.Update(player->GetWorldPosition(), boss.GetWorldPosition(), 1.0f);
	cameraDirector_.SwitchCamera(playerCamera_, 0.0f);
	cameraDirector_.Update(0.0f);
#ifndef NDEBUG
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

	// =============================
	// 基本設定・デバッグ入力
	// =============================
	DxLib::SetBackgroundColor(
		gameContext->GetBgRed(),
		gameContext->GetBgGreen(),
		gameContext->GetBgBlue());

#ifdef _DEBUG
	const bool rightMouseDown =
		(DxLib::GetMouseInput() & MOUSE_INPUT_RIGHT) != 0;

	if (rightMouseDown && !rightMouseWasDown_)
		attackSphereDebugRemaining_ = 1.0f;

	rightMouseWasDown_ = rightMouseDown;

	attackSphereDebugRemaining_ = (std::max)(
		0.0f,
		attackSphereDebugRemaining_ - (std::max)(0.0f, deltaTime));
#endif

	// =============================
	// FreeCamera
	// =============================
#ifndef NDEBUG
	const bool toggleDown =
		DxLib::CheckHitKey(KEY_INPUT_LALT) != 0 &&
		DxLib::CheckHitKey(KEY_INPUT_RETURN) != 0;

	if (toggleDown && !wasFreeCameraToggleDown_)
	{
		if (cameraDirector_.GetActiveCamera() == &freeCamera_)
		{
			cameraDirector_.SwitchCamera(playerCamera_);
		}
		else
		{
			freeCamera_.Initialize(
				playerCamera_.GetPosition(),
				playerCamera_.GetTarget());

			cameraDirector_.SwitchCamera(freeCamera_);
		}
	}

	wasFreeCameraToggleDown_ = toggleDown;

	const bool shiftHeld = DxLib::CheckHitKey(KEY_INPUT_LSHIFT) != 0 ||
		DxLib::CheckHitKey(KEY_INPUT_RSHIFT) != 0;
	if (DxLib::CheckHitKey(KEY_INPUT_R) != 0 && !shiftHeld)
		freeCamera_.ResetView();

	if (DxLib::CheckHitKey(KEY_INPUT_F) != 0)
		freeCamera_.FocusAt(gameContext->GetPlayer()->GetPosition());

	DxPlus::Debug::SetString(
		L" R: Transformリセット, F: FreeCameraをプレイヤーにフォーカス");

	if (cameraDirector_.GetActiveCamera() == &freeCamera_)
	{
		gameContext->StopTailWhipSound();
		if (!freeCamera_.IsSceneViewActive() &&
			(DxLib::GetMouseInput() & MOUSE_INPUT_RIGHT) != 0)
		{
			freeCamera_.Update(deltaTime);
		}
		cameraDirector_.Update(deltaTime);
	}
	else
#endif
	{
		auto* player = gameContext->GetPlayer();
		auto& boss = gameContext->GetBoss();
		auto& stage = gameContext->GetStage();

		// =============================
		// Stage更新
		// =============================
		stage.Update(deltaTime);

		// ゲージと飛び道具は本編と同じく、プレイヤー・ボスより先に更新する。
		if (!player->IsDead() && boss.GetHealth() > 0)
			player->AddUltimateGauge(Const::PLAYER_ULTIMATE_GAUGE_PER_SECOND * deltaTime);
		gameContext->UpdateUltimateProjectiles(deltaTime);

		// =============================
		// Player更新・床判定
		// =============================
#ifdef _DEBUG
		const bool debugDash = enableDebugDash_ && (
			DxLib::CheckHitKey(KEY_INPUT_LSHIFT) != 0 ||
			DxLib::CheckHitKey(KEY_INPUT_RSHIFT) != 0);

		player->SetDebugDashEnabled(debugDash);
#endif

		const Vec3 stageCenterPosition = stage.GetPosition();
		player->Update(deltaTime, stage.GetCollision(), stageCenterPosition);

		// ジャンプ接触は一度だけゲージを減らし、二段目だけプレイヤーを戻す。
		const auto state = player->GetCurrentState();
		const bool doubleJump = state == PlayerStateType::DoubleJump;
		const bool jumping = !player->IsGrounded() && (state == PlayerStateType::Jump || doubleJump);
		const bool touchingBody = jumping && stage.GetCollision().IsTouchingCapsule(player->GetHitSphere());

		const bool enteredBossBody = touchingBody && !wasTouchingBossBody_;
		const bool jumpedFromUpperFloor =
			player->GetJumpStartFloor() == Player::GroundType::UpFloor;
		const bool shouldDamageDownGauge = enteredBossBody && jumpedFromUpperFloor;

		if (shouldDamageDownGauge)
		{
			const float damage = doubleJump
				? Const::BOSS_UPPER_DOUBLE_JUMP_DOWN_DAMAGE
				: Const::BOSS_JUMP_DOWN_DAMAGE;
			boss.ApplyDownGaugeDamage(damage);
		}

		if (touchingBody && doubleJump)
			player->ReturnToLastSafePosition(stage.GetCollision());
		wasTouchingBossBody_ = touchingBody;

#ifdef _DEBUG
		player->SetDebugDashEnabled(false);
#endif

		// =============================
		// Boss更新・攻撃判定
		// =============================
		// Clapの目標位置を更新
		boss.UpdateClapTargetPosition(player->GetWorldPosition());
		boss.Update(deltaTime);
		if (boss.IsAIActive())
			boss.TurnToTarget(player->GetWorldPosition(), deltaTime);

		const bool slamHitPlayer = boss.UpdateSlamAttack(
			player->GetWorldPosition(), stage.GetWorldPosition());
		const bool handHitPlayer = boss.UpdateHandAttack(
			deltaTime,
			player->GetWorldPosition(),
			stage.GetWorldPosition());

		UpdateTailWhipGimmick();

		tailWhipHitCapsule_ = {};

		if (boss.IsTailWhipActive() && boss.IsTailWhipHitActive())
		{
			const Vec3 stageCenter = stage.GetWorldPosition();
			const float stageRadius = stage.GetArenaRadius();
			const float attackHeight = stage.GetArenaSurfaceY() + 30.0f;

			tailWhipHitCapsule_ = boss.GetTailWhipHitCapsule(
				stageCenter,
				stageRadius,
				attackHeight);
		}
		gameContext->UpdateTailWhipSound(tailWhipHitCapsule_);

		const Collision::Sphere playerBody = player->GetHitSphere();

		const bool tailWhipPlayerHit =
			boss.IsTailWhipHitActive() &&
			Collision::IsHitCapsuleSphere(tailWhipHitCapsule_, playerBody);

		if (tailWhipPlayerHit)
		{
			player->TakeDamage(Const::BOSS_TAIL_WHIP_DAMAGE);
			Vec3 outward = player->GetWorldPosition() - stage.GetWorldPosition();
			outward.y = 0.0f;
			player->ApplyKnockback(outward, 5000.0f, 0.3f);
		}

		// =============================
		// Playerの攻撃判定（デバッグ）
		// =============================
		auto& playerStateMachine = player->GetStateMachine();

		if (playerStateMachine.IsAttackBallActive())
		{
			const auto& attackBall =
				playerStateMachine.GetAttackBallSphere();

			if (boss.IsHitByAttackSphere(attackBall, tailWhipHitCapsule_, stage.GetBossDownHurtSphere()))
			{
				if (playerStateMachine.TryRegisterAttackBallHit())
					boss.TakeDamage(Const::PLAYER_NORMAL_ATTACK_DAMAGE);
			}
		}

		// プレイヤーが被ダメージ状態に移る前に、攻撃の命中を登録する。
		if (handHitPlayer)
			player->TakeDamage(Const::BOSS_CLAP_DAMAGE);
		else if (slamHitPlayer)
			player->TakeDamage(Const::BOSS_SLAM_DAMAGE);

#ifdef _DEBUG
		DxPlus::Debug::SetFormatString(
			L"Boss HP: %d / %d",
			boss.GetHealth(),
			Boss::MaxHealth);
		DxPlus::Debug::SetFormatString(
			L"Ult: %.1f / %.1f",
			player->GetUltimateGauge(),
			Player::UltimateGaugeMax);

		DxPlus::Debug::SetString(
			L"1: Hand attack test (3 sec follow)");
#endif

		// =============================
		// カメラシェイク
		// =============================
		UpdateCameraShake(boss, *player);

		// =============================
		// カメラ更新・切り替え
		// =============================
		if (boss.IsDowned())
		{
			bossDownCamera_.Update(
				player->GetWorldPosition(),
				boss.GetWorldPosition(),
				deltaTime);

			if (cameraDirector_.GetActiveCamera() != &bossDownCamera_)
			{
				bossDownCamera_.Initialize(cameraDirector_.GetCurrentPose());
				cameraDirector_.SwitchCamera(bossDownCamera_, 1.2f);
			}
		}
		else
		{
			playerCamera_.Update(
				player->GetWorldPosition(),
				boss.GetWorldPosition(),
				deltaTime);

			playerCamera_.SetFieldOfView(60.0f);

			if (cameraDirector_.GetActiveCamera() != &playerCamera_)
			{
				cameraDirector_.SwitchCamera(playerCamera_, 1.2f);
			}
		}

		cameraDirector_.Update(deltaTime);

		// =============================
		// カメラ透過
		// =============================
		Collision::ResetTransparentModels();

		Collision::SetModelsBetweenTransparent(
			cameraDirector_.GetActiveCamera()->GetPosition(),
			player->GetWorldPosition() + Vec3::Up() * 30.0f,
			stage.GetCollision(),
			0.7f);

		Collision::SetModelsBetweenTransparent(
			cameraDirector_.GetActiveCamera()->GetPosition(),
			boss.GetWorldPosition() + Vec3::Up() * 600.0f,
			stage.GetCollision(),
			0.7f);

		// =============================
		// 入力・エフェクト
		// =============================
		if (GetButtonDown(PLAYER1) & BUTTON_L3)
		{
			gameContext->GetEffectManager().PlayHit(
				RM().GetEffect(ResourceKeys::Effect_Test),
				player->GetPosition() + Vec3::Up() * 100.0f,
				player->GetYaw() + DX_PI_F);
		}

		gameContext->GetEffectManager().Update(deltaTime);

		// 本編の勝敗判定と同じタイミングで、テスト用の再開を行う。
		if (!boss.IsActive() || player->GetCurrentState() == PlayerStateType::Die)
		{
			Init();
			return;
		}
	}

#ifndef NDEBUG
	// 数値入力中はシーン切り替えのキーを無視する
	if (ImGui::GetCurrentContext() &&
		ImGui::GetIO().WantCaptureKeyboard)
	{
		return;
	}
#endif

	// =============================
	// シーン遷移
	// =============================
	const int buttonDown = GetButtonDown(PLAYER1);

	if (buttonDown & BUTTON_SELECT)
	{
		Scene* resultScene =
			SceneManager::GetInstance().GetScene(SceneID::Result);

		SetNextScene(resultScene);
		finished = true;
		return;
	}

	// =============================
	// シェーダー切り替え
	// =============================
	static bool shaderToggleWasDown = false;

	const bool shaderToggleDown =
		DxLib::CheckHitKey(KEY_INPUT_F2) != 0;

	if (shaderToggleDown && !shaderToggleWasDown)
		shaderEffects_.SetEnabled(!shaderEffects_.IsEnabled());

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
	cameraDirector_.ApplyRenderPose();
	gameContext->GetBoss().DrawClapAttackDebug();
	gameContext->GetBoss().DrawSlamAttackDebug();

	auto& boss = gameContext->GetBoss();
	auto* player = gameContext->GetPlayer();
	if (showBossBodyCapsule_)
	{
		const auto& bodyCollision = gameContext->GetStage().GetCollision();
		const bool touchingBody = player != nullptr && bodyCollision.IsTouchingCapsule(player->GetHitSphere());
		DxLib::SetUseZBuffer3D(FALSE);
		DxLib::SetWriteZBuffer3D(FALSE);
		Collision::DrawDebugCapsule(bodyCollision.GetContactCapsule(),
			touchingBody ? DxLib::GetColor(255, 220, 60) : DxLib::GetColor(80, 220, 255));
		DxLib::SetUseZBuffer3D(TRUE);
		DxLib::SetWriteZBuffer3D(TRUE);
	}
	// 攻撃中の尻尾カプセルを表示する。黄色は接触中、青は未接触。
	if (boss.IsTailWhipHitActive() && player != nullptr)
	{
		const Collision::Capsule& tail = tailWhipHitCapsule_;
		const auto playerBody = player->GetHitSphere();
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
		ImGui::Checkbox(u8"Shift長押しで移動速度を5倍にする", &enableDebugDash_);
		ImGui::TextUnformatted(u8"Shift + R：プレイヤー位置に毛玉エフェクト再生");
#endif

		ImGui::Text("Pillar contact: %s", player->IsTouchingWall() ? "True" : "False");

		const float avoidTime = player->GetStateMachine().GetAvoidTime();
		if (avoidTime > 0.0f)
		{
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram,
				toImGuiColor(DxLib::GetColor(255, 0, 0)));
			ImGui::ProgressBar(avoidTime / PlayerDamagingState::AvoidTime, ImVec2(-1.0f, 0.0f), "Avoid window");
			ImGui::PopStyleColor();
		}

		const float avoidInvTime = player->GetAvoidInvTime();
		if (avoidInvTime > 0.0f)
		{
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram,
				toImGuiColor(DxLib::GetColor(255, 255, 0)));
			ImGui::ProgressBar(avoidInvTime / Player::AvoidInvTime, ImVec2(-1.0f, 0.0f), "Invincible");
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
		auto& stage = gameContext->GetStage();

		if (ImGui::Button(u8"Boss.Reset"))
		{
			boss.ResetForDebug();
			gameContext->FaceBossTowardPlayer();
			gameContext->StopTailWhipSound();
			tailWhipHitCapsule_ = {};
			wasTouchingBossBody_ = false;
			wasTailWhipActive_ = false;
		}

		if (ImGui::Button(u8"Down"))
			boss.SetDownGauge(0.0f);

		float downHurtRadius = stage.GetBossDownHurtRadius();
		if (ImGui::DragFloat(u8"ダウン時の被ダメージ判定の半径", &downHurtRadius, 1.0f, 1.0f, 5000.0f,
			"%.1f", ImGuiSliderFlags_AlwaysClamp))
			stage.SetBossDownHurtRadius(downHurtRadius);

		if (ImGui::Button(u8"Take Damage (1 HP)"))
			boss.TakeDamage(1);

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
		ImGui::Checkbox(u8"bossAIEnabled", &boss.GetDebugAIEnabled());
		// Stage Gimmick Test
		ImGui::Separator();
		ImGui::TextUnformatted(u8"Stage Gimmick Test");

		ImGui::Separator();
		if (ImGui::CollapsingHeader(u8"ボス本体の当たり判定", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Checkbox(u8"カプセルの枠を表示", &showBossBodyCapsule_);
			auto body = stage.GetBossBodySettings();
			bool changed = ImGui::DragFloat(u8"本体の半径", &body.radius, 1.0f, 1.0f, 5000.0f,
				"%.1f", ImGuiSliderFlags_AlwaysClamp);
			changed |= ImGui::DragFloat(u8"本体の全高", &body.height, 1.0f, body.radius * 2.0f, 10000.0f,
				"%.1f", ImGuiSliderFlags_AlwaysClamp);
			changed |= ImGui::DragFloat(u8"底の高さオフセット", &body.yOffset, 1.0f, -5000.0f, 5000.0f,
				"%.1f", ImGuiSliderFlags_AlwaysClamp);
			if (changed) stage.SetBossBodySettings(body);
			if (ImGui::Button(u8"本体の判定を初期値に戻す")) stage.ResetBossBodySettings();
			const auto& capsule = stage.GetCollision().GetContactCapsule();
			const Vec3 center = (capsule.start + capsule.end) * 0.5f;
			ImGui::Text(u8"中心座標：X %.1f / Y %.1f / Z %.1f", center.x, center.y, center.z);
			ImGui::TextUnformatted(stage.GetCollision().IsTouchingCapsule(player->GetHitSphere())
				? u8"プレイヤー：接触中（黄色）" : u8"プレイヤー：接触なし（青色）");
			ImGui::TextWrapped(u8"全高は上下の丸い部分を含み、半径の2倍以上になります。変更は実行中のみ有効です。");
		}

		if (ImGui::Button(u8"Stage Shake"))
			stage.StartShake();
		if (ImGui::Button(u8"knock Back"))
			player->ApplyKnockback({ 1,0,0 }, 10000, 0.1f);

		// Player Status Test
		ImGui::Separator();
		ImGui::TextUnformatted(u8"Player Status Test");

		ImGui::Text("HP: %d / %d",
			player->GetHealth(),
			Player::MaxHealth);

		if (ImGui::Button(u8"HP MAX"))
			player->SetHealth(Player::MaxHealth);

		ImGui::SameLine();
		if (ImGui::Button(u8"HP 0"))
			player->SetHealth(0);

		if (ImGui::Button(u8"HP -1"))
			player->TakeDamage(1);


		ImGui::Separator();

		ImGui::Text("Ultimate: %.1f / %.1f",
			player->GetUltimateGauge(),
			Player::UltimateGaugeMax);

		float ultimateGauge = player->GetUltimateGauge();

		if (ImGui::SliderFloat(
			u8"必殺ゲージ",
			&ultimateGauge,
			0.0f,
			Player::UltimateGaugeMax))
		{
			player->SetUltimateGauge(ultimateGauge);
		}

		if (ImGui::Button(u8"必殺ゲージ MAX"))
			player->SetUltimateGauge(Player::UltimateGaugeMax);

		ImGui::SameLine();
		if (ImGui::Button(u8"必殺ゲージ 0"))
			player->SetUltimateGauge(0.0f);

		ImGui::TextWrapped(u8"設定は実行中のみ有効です。");

	}
	ImGui::End();
#endif
}

void GameTestScene::End()
{
	gameContext->StopTailWhipSound();
	gameContext->ClearUltimateProjectiles();
	Collision::ResetTransparentModels();
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

		if (player->WasLastOnSecondFloor()
			|| player->GetCurrentState() == PlayerStateType::Climbing)// 柱か二階なら吹き飛ばす
		{
			player->ApplyKnockback(outward, 5000.0f, 0.3f);
		}
	}

	wasTailWhipActive_ = isTailWhipActive;
}
void GameTestScene::RenderDepth() const
{
	cameraDirector_.ApplyRenderPose();

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
	cameraDirector_.ApplyRenderPose();

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

	// Room
	for (const auto* allRoom : room.GetAllModelObjects())
	{
		drawModel(*allRoom);
	}


	// BossとPlayer

	drawModel(player->GetModelObject());


	for (const auto& preview : selectPlayers)
	{
		drawModel(preview.GetModelObject());
	}

	// Stage
	drawModel(stage.GetModelObject());
	// boss
	drawModel(boss.GetModelObject());
	// stageObj
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
	const auto& boss = gameContext->GetBoss();
	boss.DrawSlamAttackDebug();

	if (boss.GetCurrentState() == BossStateType::Down)
	{
		Collision::DrawDebugSphere(gameContext->GetStage().GetBossDownHurtSphere(), DxLib::GetColor(255, 140, 60));
	}

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

void GameTestScene::UpdateCameraShake(Boss& boss, Player& player)
{
	auto applyShake = [this](auto& actor)
		{
			const auto type = actor.GetCameraShakeRequest();
			if (type == CameraShakeType::None) return;

			const float duration = actor.GetCameraShakeDuration();
			const float strength = actor.GetCameraShakeStrength();
			const float frequency = actor.GetCameraShakeFrequency();

			switch (type)
			{
			case CameraShakeType::Horizontal:
				cameraDirector_.StartHorizontalShake(duration, strength, frequency);
				break;
			case CameraShakeType::Vertical:
				cameraDirector_.StartVerticalShake(duration, strength, frequency);
				break;
			case CameraShakeType::Mixed:
				cameraDirector_.StartMixedShake(duration, strength, frequency);
				break;
			}

			actor.ResetCameraShakeRequest();
		};

	applyShake(boss);
	applyShake(player);
}
