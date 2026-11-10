#include "GameScene.h"
#include "GameContext.h"
#include "ToonSettings.h"
#include "OutlineSettings.h"
#include "CameraObjectManager.h"
#include "UIManager.h"
#include "ResourceKeys.h"
#include "ResourceManager.h"
#include "SoundManager.h"
#include "StatusUI.h"
#include <algorithm>
#include "DxPlus/DxPlus.h"
#include "Collision.h"
#include "DxConv.h"

#ifndef NDEBUG
#include "imgui.h"
#endif

namespace
{
	constexpr float TutorialSkipHoldTime = 1.5f;
	// スキップ後の配置。ステージ中心からの横方向の距離。
	constexpr Vec3 TutorialSkipBattleOffset{ 0.0f, 0.0f, -1000.0f };
}

GameScene::GameScene(GameContext* context)
	: Scene(context), stateMachine_(*this) {
}



void GameScene::Init()
{
	ResetForTitle();
	battleEventActive_ = false;
	battleLetterboxMotion_.Reset();

	// チュートリアルも、使いたいモデルを登録するだけで同じ判定を利用できる。
	tutorialCollision_.Clear();
	tutorialCollision_.AddModel(gameContext->GetRoom().GetRoomObject(), ModelCollision::Surface::Floor);
	tutorialCollision_.AddModel(gameContext->GetRoom().GetRoomObject(), ModelCollision::Surface::Wall);

	// チュートリアルでもStageの柱を判定する。
	for (const auto& pillar : gameContext->GetStage().GetPillarModelObjects())
		tutorialCollision_.AddModel(pillar, ModelCollision::Surface::Pillar);

	for (const auto* pillar : gameContext->GetRoom().GetPillars())
		tutorialCollision_.AddModel(*pillar, ModelCollision::Surface::Pillar);

	// ジャンプ台の床と柱を、それぞれの用途で登録する。
	for (const auto* stand : gameContext->GetRoom().GetJumpStands())
	{
		tutorialCollision_.AddModel(stand->GetJumpStandModel(), ModelCollision::Surface::Floor);
		tutorialCollision_.AddModel(stand->GetPillarModel(), ModelCollision::Surface::Pillar);
	}
	tutorialCollision_.AddModel(gameContext->GetStage().GetModelObject(), ModelCollision::Surface::Floor);

	// トゥーン描画とアウトライン描画を有効にする
	{
		ToonSettingsManager::Load();
		OutlineSettingsManager::Load();

		// トゥーン描画に必要なシェーダーと設定バッファーを初期化
		if (!modelToonRenderer_.Init())
			DxPlus::Utils::FatalError(L"modelToonRendererの初期化失敗");
		if (!shaderEffects_.Init(DxPlus::CLIENT_WIDTH, DxPlus::CLIENT_HEIGHT))
			DxPlus::Utils::FatalError(L"shaderEffectsの初期化失敗");

		// toon描画とアウトライン描画を有効にする
		shaderEffects_.GetSettings().toonEnabled = true;
		shaderEffects_.GetSettings().outlineEnabled = true;
	}

	// camera
	{
		camera_.Reset();
		playerTargetCamera_.Reset();
		bossDieCamera_.Reset();
		bossDownCamera_.Reset();

		cameraDirector_.Reset();
		activeCameraShot_.clear();

		if (!CObjs().LoadCameras())
			DxPlus::Utils::FatalError(L"カメラのロードに失敗");

		for (const auto& shot : CObjs().GetCameras())
		{
			cameraDirector_.RegisterShot(shot.name, shot.pose);
		}

		camera_.Initialize(CObjs().GetCameraPose("Title"));
		cameraDirector_.Update(0);
	}



	stateMachine_.Initialize();

	StartFadeIn();
}

void GameScene::ResetForTitle()
{
	battleCheckpoint_.reset();
	gameContext->Reset();
	gameContext->SetSelectPreviewVisible(true);
	cameraDirector_.StopShake();
}

void GameScene::SaveBattleCheckpoint()
{
	const auto& boss = gameContext->GetBoss();
	battleCheckpoint_ = BattleCheckpoint{
		gameContext->GetPlayer()->GetTransform(),
		boss.GetTransform(),
		boss.GetPosition(),
		boss.GetRotation(),
		cameraDirector_.GetCurrentPose(),
		playerTargetCamera_
	};
}

bool GameScene::RestoreBattleCheckpoint()
{
	if (!battleCheckpoint_) return false;

	const auto& checkpoint = *battleCheckpoint_;
	auto* player = gameContext->GetPlayer();
	auto& boss = gameContext->GetBoss();

	gameContext->GetStage().Reset();
	player->Reset();
	player->SetTransform(checkpoint.playerTransform);
	player->SetStartPosition(checkpoint.playerTransform.position);
	player->ResetCameraShakeRequest();

	boss.Reset();
	boss.SetTransform(checkpoint.bossTransform);
	boss.SetPosition(checkpoint.bossPosition);
	boss.SetRotation(checkpoint.bossRotation);
	boss.ResetCameraShakeRequest();

	Collision::ResetTransparentModels();
	playerTargetCamera_ = checkpoint.targetCamera;
	cameraDirector_.StopShake();
	cameraDirector_.SetPoseImmediate(checkpoint.cameraPose);
	return true;
}

void GameScene::SkipTutorial()
{
	if (GetCurrentGameState() != GameStateType::Tutorial) return;

	auto* player = gameContext->GetPlayer();
	const auto& stage = gameContext->GetStage();
	const auto& boss = gameContext->GetBoss();
	Vec3 position = stage.GetWorldPosition() + TutorialSkipBattleOffset;
	// スキップ後の位置は、ステージの床面に合わせる
	position.y = stage.GetArenaSurfaceY() + Const::PLAYER_SKIN;
	player->SetStartPosition(position);
	player->ResetPosition();
	// スキップ後の向きは、ボスの方を向く
	const Vec3 toBoss = boss.GetWorldPosition() - position;
	player->SetYaw(std::atan2(toBoss.x, toBoss.z));
	// スキップ後のカメラは、ボスを中心にしたターゲットカメラにする
	Collision::ResetTransparentModels();
	cameraDirector_.StopShake();
	stateMachine_.StartBattleWithoutIntro();
	SaveBattleCheckpoint();
}

void GameScene::UpdateDuringFade(float deltaTime)
{
	if (gameContext == nullptr) return;

	auto& previews = gameContext->GetSelectPreviewPlayers();
	previews[0].UpdateAnimation(deltaTime);
	previews[1].UpdateAnimation(deltaTime);
}

bool GameScene::UpdateStateCamera(float deltaTime, const std::string& requestedShot)
{
	const bool transitionCompleted = cameraDirector_.Update(deltaTime);
	if (!requestedShot.empty() && requestedShot != activeCameraShot_ && !cameraDirector_.IsTransitioning())
	{
		if (cameraDirector_.PlayShot(requestedShot, 1.2f, Motion::Easing::EaseInOut))
		{
			activeCameraShot_ = requestedShot;
		}
	}
	return transitionCompleted;
}


void GameScene::Update(float deltaTime)
{
	if (battleEventActive_)
		battleLetterboxMotion_.Update(deltaTime);

	DxLib::SetBackgroundColor(
		gameContext->GetBgRed(),
		gameContext->GetBgGreen(),
		gameContext->GetBgBlue()
	);

	const auto currentState = GetCurrentGameState();
	const bool enterDown = DxLib::CheckHitKey(KEY_INPUT_RETURN) != 0;
	const bool enterWasDown = tutorialSkipKeyWasDown_;
	tutorialSkipKeyWasDown_ = enterDown;

	if (currentState != GameStateType::Tutorial)// Tutorial以外では、スキップ判定をリセットする
		tutorialSkipHoldTime_ = 0.0f;

	// Tutorial・Battle以外
	if (currentState != GameStateType::Tutorial &&
		currentState != GameStateType::Battle)
	{
		stateMachine_.Tick(deltaTime);
		gameContext->GetEffectManager().Update(deltaTime);
		return;
	}

	// Battleの更新はBattleStateに任せる
	if (currentState == GameStateType::Battle)
	{
		const auto& boss = gameContext->GetBoss();
		const auto* player = gameContext->GetPlayer();
		if (boss.IsActive())
			stateMachine_.Tick(deltaTime);

		if (GetCurrentGameState() != GameStateType::Battle)
			return;
		StatusUI::SyncPlayerHealth(player->GetHealth(), deltaTime);

		// 死亡判定

		if (!boss.IsActive())
		{
			stateMachine_.ChangeState(GameStateType::Clear);
		}
		else if (gameContext->GetPlayer()->GetCurrentState() == PlayerStateType::Die)
		{
			stateMachine_.ChangeState(GameStateType::GameOver);
		}
		return;
	}

	// =============================
	// Tutorial更新
	// =============================
	auto* player = gameContext->GetPlayer();
	auto& boss = gameContext->GetBoss();

	player->SetDefMove(true);
	player->Update(deltaTime, tutorialCollision_);

	if (!enterDown || !enterWasDown)
		tutorialSkipHoldTime_ = 0.0f;
	else
		tutorialSkipHoldTime_ += deltaTime;

	if (tutorialSkipHoldTime_ >= TutorialSkipHoldTime)
	{
		SkipTutorial();
		return;
	}

	boss.Update(deltaTime);

	// =============================
	// State更新
	// =============================
	stateMachine_.Tick(deltaTime);

	// =============================
	// カメラ透過
	// =============================
	Collision::ResetTransparentModels();

	Collision::SetModelsBetweenTransparent(
		cameraDirector_.GetCurrentPose().position,
		player->GetWorldPosition() + Vec3::Up() * 30.0f,
		tutorialCollision_
	);

	// =============================
	// チュートリアルタスク
	// =============================
	tutorialChecklist_.Update(deltaTime);

	// =============================
	// エフェクト更新
	// =============================
	gameContext->GetEffectManager().Update(deltaTime);
}

void GameScene::Render() const
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

	if (battleEventActive_)
	{
		const int barHeight = static_cast<int>(DxPlus::CLIENT_HEIGHT / 8.0f *
			battleLetterboxMotion_.GetValue());
		const int color = DxLib::GetColor(0, 0, 0);
		DxLib::DrawBox(0, 0, DxPlus::CLIENT_WIDTH, barHeight, color, TRUE);
		DxLib::DrawBox(0, DxPlus::CLIENT_HEIGHT - barHeight,
			DxPlus::CLIENT_WIDTH, DxPlus::CLIENT_HEIGHT, color, TRUE);
	}
}

void GameScene::DrawImGui()
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

	if (ImGui::Begin("GameScene Debug"))
	{
		auto* player = gameContext->GetPlayer();
		auto& boss = gameContext->GetBoss();
		auto& stage = gameContext->GetStage();

		// =============================
		// Scene Test
		// =============================
		if (ImGui::Button("Clear"))
			stateMachine_.ChangeState(GameStateType::Clear);

		if (ImGui::Button("GameOver"))
			stateMachine_.ChangeState(GameStateType::GameOver);

		// =============================
		// Player
		// =============================
		ImGui::Separator();
		ImGui::TextUnformatted(u8"Player");

		ImGui::BeginDisabled(GetCurrentGameState() != GameStateType::Battle ||
			player->IsDead() || player->IsDamaging() || player->GetAvoidInvTime() > 0.0f);
		if (ImGui::Button("Kill Player"))
			player->TakeDamage(player->GetHealth());
		ImGui::EndDisabled();

		ImGui::Text(
			"Pillar contact: %s",
			player->IsTouchingWall() ? "True" : "False"
		);

		const float avoidTime =
			player->GetStateMachine().GetAvoidTime();

		if (avoidTime > 0.0f)
		{
			ImGui::PushStyleColor(
				ImGuiCol_PlotHistogram,
				toImGuiColor(DxLib::GetColor(255, 0, 0))
			);

			ImGui::ProgressBar(
				avoidTime / PlayerDamagingState::AvoidTime,
				ImVec2(-1.0f, 0.0f),
				"Avoid 0.3s"
			);

			ImGui::PopStyleColor();
		}

		const float avoidInvTime = player->GetAvoidInvTime();

		if (avoidInvTime > 0.0f)
		{
			ImGui::PushStyleColor(
				ImGuiCol_PlotHistogram,
				toImGuiColor(DxLib::GetColor(255, 255, 0))
			);

			ImGui::ProgressBar(
				avoidInvTime / Player::AvoidInvTime,
				ImVec2(-1.0f, 0.0f),
				"Invincible 0.2s"
			);

			ImGui::PopStyleColor();
		}

		ImGui::TextUnformatted(u8"柱のエレベーター");

		float climbingSpeed = player->GetClimbingSpeed();

		if (ImGui::DragFloat(
			u8"上昇速度",
			&climbingSpeed,
			10.0f,
			0.0f,
			5000.0f,
			"%.1f",
			ImGuiSliderFlags_AlwaysClamp))
		{
			player->SetClimbingSpeed(climbingSpeed);
		}

		if (ImGui::Button(u8"上昇速度を初期値に戻す"))
			player->ResetClimbingSpeeds();

		ImGui::TextWrapped(
			u8"柱の上端 - 10で到着し、小さくジャンプします。"
		);

		// =============================
		// Boss Attack Test
		// =============================
		ImGui::Separator();
		ImGui::TextUnformatted(u8"Boss Attack Test");

		if (ImGui::Button(u8"Boss.Reset"))
			boss.Reset();

		if (ImGui::Button(u8"Down (Flinch)"))
			boss.SetDownGauge(0.0f);

		if (ImGui::Button(u8"Die (成仏)"))
			boss.TakeDamage(999);

		if (ImGui::Button(u8"Clap"))
			boss.UseSkill(BossAttackType::Clap);

		if (ImGui::Button(u8"Slam"))
			boss.UseSkill(BossAttackType::Slam);

		if (ImGui::Button(u8"TailWhip"))
			boss.UseSkill(BossAttackType::TailWhip);

		ImGui::Checkbox(
			u8"bossAIEnabled",
			&boss.IsAIActive()
		);

		// =============================
		// Camera Settings
		// =============================
		ImGui::Separator();

		auto& camera = GetTargetCamera();
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

		// =============================
		// Stage Gimmick Test
		// =============================
		ImGui::Separator();
		ImGui::TextUnformatted(u8"Stage Gimmick Test");

		if (ImGui::CollapsingHeader(
			u8"ボス本体の当たり判定",
			ImGuiTreeNodeFlags_DefaultOpen))
		{


			auto body = stage.GetBossBodySettings();

			bool changed = ImGui::DragFloat(
				u8"本体の半径",
				&body.radius,
				1.0f,
				1.0f,
				5000.0f,
				"%.1f",
				ImGuiSliderFlags_AlwaysClamp
			);

			changed |= ImGui::DragFloat(
				u8"本体の全高",
				&body.height,
				1.0f,
				body.radius * 2.0f,
				10000.0f,
				"%.1f",
				ImGuiSliderFlags_AlwaysClamp
			);

			changed |= ImGui::DragFloat(
				u8"底の高さオフセット",
				&body.yOffset,
				1.0f,
				-5000.0f,
				5000.0f,
				"%.1f",
				ImGuiSliderFlags_AlwaysClamp
			);

			if (changed)
				stage.SetBossBodySettings(body);

			if (ImGui::Button(u8"本体の判定を初期値に戻す"))
				stage.ResetBossBodySettings();

			const auto& capsule =
				stage.GetCollision().GetContactCapsule();

			const Vec3 center =
				(capsule.start + capsule.end) * 0.5f;

			ImGui::Text(
				u8"中心座標：X %.1f / Y %.1f / Z %.1f",
				center.x,
				center.y,
				center.z
			);

			ImGui::TextUnformatted(
				stage.GetCollision().IsTouchingCapsule(
					player->GetHitSphere()
				)
				? u8"プレイヤー：接触中（黄色）"
				: u8"プレイヤー：接触なし（青色）"
			);

			ImGui::TextWrapped(
				u8"全高は上下の丸い部分を含み、半径の2倍以上になります。"
			);
		}

		if (ImGui::Button(u8"Stage Shake"))
			stage.StartShake();

		if (ImGui::Button(u8"Knock Back"))
			player->ApplyKnockback({ 1, 0, 0 }, 10000.0f, 0.1f);

		ImGui::TextWrapped(u8"設定は実行中のみ有効です。");
	}

	ImGui::End();

#endif
}
void GameScene::RenderDepth() const
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

void GameScene::DrawTutorialCollisionSpheres() const
{
#ifdef _DEBUG
	const auto& settings = tutorialCollisionSettings_;

	if (!settings.drawSpheres ||
		GetCurrentGameState() != GameStateType::Tutorial)
	{
		return;
	}

	const auto* player = gameContext->GetPlayer();
	const auto& stage = gameContext->GetStage();

	const Vec3 playerCenter =
		player->GetWorldPosition() + settings.playerOffset;

	const Vec3 stageCenter =
		stage.GetWorldPosition() + settings.stageOffset;

	// Collision共通のSphere型を使う
	const Collision::Sphere playerSphere{
		playerCenter, settings.playerRadius
	};

	const Collision::Sphere stageSphere{
		stageCenter, settings.stageRadius
	};

	const Collision::Sphere climbSphere{
		stageCenter, settings.climbRadius
	};

	// Player：緑
	Collision::DrawDebugSphere(
		playerSphere, DxLib::GetColor(0, 255, 0));

	// Battle遷移判定：赤
	Collision::DrawDebugSphere(
		stageSphere, DxLib::GetColor(255, 0, 0));

	// カメラ追従開始判定：青
	Collision::DrawDebugSphere(
		climbSphere, DxLib::GetColor(0, 100, 255));
#endif
}

void GameScene::DrawScene(bool toonEnabled) const
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

		player->Draw();
		boss.Draw();
		stage.Draw();


		for (const auto& preview : selectPlayers)
		{
			preview.Draw();
		}




		effectManager.Draw();
		grid.Draw();
		room.Draw();


		DrawTutorialCollisionSpheres();
#ifdef _DEBUG
		boss.DrawSlamAttackDebug();
#endif
		return;
	}

	modelToonRenderer_.BeginPass();

	// 元のModelObjectを直接描画する
	// 親子変換はModelObject::ApplyTransform()に任せる
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


	for (const auto& preview : selectPlayers)
	{
		drawModel(preview.GetModelObject());
	}




	// Stage
	drawModel(stage.GetModelObject());
	// Boss
	drawModel(boss.GetModelObject());

	if (player->WasLastOnFirstFloor())
	{
		if (drawPlayerShadow_)
			DrawPlayerShadow();
		drawModel(player->GetModelObject());
	}
	// StageObjects
	for (const auto& pillar : stage.GetPillarModelObjects())
	{
		drawModel(pillar);
	}

	for (const auto& model : stage.GetStageModelObjects())
	{
		drawModel(model);
	}
	if (player->WasLastOnSecondFloor())
	{
		if (drawPlayerShadow_)
			DrawPlayerShadow();
		drawModel(player->GetModelObject());
	}



	effectManager.Draw();
	modelToonRenderer_.EndPass();
	effectManager.Draw();
#ifdef _DEBUG
	player->GetStateMachine().Draw();
#endif
	DrawTutorialCollisionSpheres();
#ifdef _DEBUG
	boss.DrawSlamAttackDebug();
#endif
}

void GameScene::End()
{
	SoundManager::StopBGM(RM().GetMusic(ResourceKeys::Music_Title));
	SoundManager::StopBGM(RM().GetMusic(ResourceKeys::Music_Battle));
	gameContext->StopTailWhipSound();
	gameContext->ClearUltimateProjectiles();
	tutorialCollision_.Clear();
}

void GameScene::DrawPlayerShadow() const
{
	const auto* player = gameContext->GetPlayer();
	const auto& stage = gameContext->GetStage();

	// 状態に応じて使用するCollisionを選ぶ
	const ModelCollision* collision = nullptr;

	if (GetCurrentGameState() == GameStateType::Tutorial)
		collision = &tutorialCollision_;
	else if (GetCurrentGameState() == GameStateType::Battle)
		collision = &stage.GetCollision();

	if (!collision) return;

	// 床へのRaycast
	const Vec3 playerPos = player->GetWorldPosition();

	// Playerの後方へ30移動
	const float yaw = player->GetYaw();
	const Vec3 backward(
		-sinf(yaw) * 30.0f,
		0.0f,
		-cosf(yaw) * 30.0f
	);

	const Vec3 shadowPos = playerPos + backward;

	Physics::RayHit hit{};
	if (!collision->RaycastDown(
		shadowPos + Vec3::Up() * 50.0f,
		500.0f,
		hit))
	{
		return;
	}

	// 床の位置を画面座標に変換
	const VECTOR screenPos = ConvWorldPosToScreenPos(
		DxConv::ToVECTOR(hit.point));

	const int x = static_cast<int>(screenPos.x);
	const int y = static_cast<int>(screenPos.y);

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);

	DrawOval(
		x, y,
		80, 18,
		GetColor(0, 0, 0),
		TRUE);

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}
