// =============================
// Gameplay/Actors/Stage.cpp
// =============================
#include "Stage.h"
#include "Player.h"
#include "ResourceManager.h"
#include "ResourceKeys.h"
#include "DxConv.h"
#include "Consts.h"
#include "imgui.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <algorithm>
#include <cmath>

#include "ModelObjectManager.h"

namespace
{
	float AlphaStageScale = 0.15f;
	float AlphaStageHeadClearance = 40.0f;

	float AlphaStageFadeSpeed = 2.4f;
}

Stage::Stage() : ActorBase("Stage")
{
	stagePillars_[0].SetModelKey(ResourceKeys::Model_PillarL);
	stagePillars_[1].SetModelKey(ResourceKeys::Model_PillarM);
	stagePillars_[2].SetModelKey(ResourceKeys::Model_PillarS);

	stageModels_[0].SetModelKey(ResourceKeys::Model_stand_l);
	stageModels_[1].SetModelKey(ResourceKeys::Model_stand_m);
	stageModels_[2].SetModelKey(ResourceKeys::Model_stand_s);
	// 地面の当たり判定に使うモデル
	stageModel_.SetModelKey(ResourceKeys::Model_Stage);

	for (auto& model : stagePillars_) model.SetOwner(*this);
	for (auto& model : stageModels_) model.SetOwner(*this);
	stageModel_.SetOwner(*this);

	stagePillars_[0].SetObjectId("PillarL");
	stagePillars_[0].SetObjectName("PillarL");
	stagePillars_[1].SetObjectId("PillarM");
	stagePillars_[1].SetObjectName("PillarM");
	stagePillars_[2].SetObjectId("PillarS");
	stagePillars_[2].SetObjectName("PillarS");

	stageModels_[0].SetObjectId("StageStandL");
	stageModels_[0].SetObjectName("StageStandL");

	stageModels_[1].SetObjectId("StageStandM");
	stageModels_[1].SetObjectName("StageStandM");

	stageModels_[2].SetObjectId("StageStandS");
	stageModels_[2].SetObjectName("StageStandS");

	stageModel_.SetObjectId("StageGround");
	stageModel_.SetObjectName("StageGround");


}

// accessor
int Stage::GetModelHandle() const { return RM().GetModel(stageModel_.GetModelKey()); }


void Stage::Init()
{
	Reset();
}


void Stage::UpdateArenaBounds()
{
	const Collision::Ellipsoid arenaBounds = stageModel_.GetHitEllipsoid();
	const Vec3 arenaCenter = GetWorldPosition();
	arenaRadius_ = (std::max)(
		std::abs(arenaBounds.center.x - arenaCenter.x) + arenaBounds.radii.x,
		std::abs(arenaBounds.center.z - arenaCenter.z) + arenaBounds.radii.z);
	arenaSurfaceY_ = arenaBounds.center.y + arenaBounds.radii.y;
}

void Stage::Reset()
{
	MObjs().LoadActorTransform(*this);
	for (auto& model : stagePillars_)
	{
		model.Load();
	}
	for (auto& model : stageModels_)
	{
		model.Load();
	}
	stageModel_.Load();

	UpdateArenaBounds();



	// RefreshCollisionInfo();

	// モデルごとに判定の役割を登録する。形・大きさはモデルから取得される。
	collision_.Clear();
	collision_.AddModel(stageModel_, ModelCollision::Surface::Floor);

    // モデルごとに判定の役割を登録する。形・大きさはモデルから取得される。
    collision_.Clear();
    collision_.AddFallRecoveryFloor(stageModel_);

	// 当たり判定を登録
	collision_.Clear();

	collision_.AddFallRecoveryFloor(stageModel_);

	for (const auto& model : stageModels_)
		collision_.AddModel(model, ModelCollision::Surface::Stand);

	for (const auto& model : stagePillars_)
		collision_.AddModel(model, ModelCollision::Surface::Pillar);

	collision_.Refresh();

	// 初期回転を保持

	for (int i = 0; i < stagePillars_.size(); ++i)
	{
		pillarBaseRotations_[i] = stagePillars_[i].GetRotation();
		pillarBasePositions_[i] = stagePillars_[i].GetPosition();
	}

	for (int i = 0; i < stageModels_.size(); ++i)
	{
		stageBaseRotations_[i] = stageModels_[i].GetRotation();
		stageBasePositions_[i] = stageModels_[i].GetPosition();
	}

	isShaking_ = false;
	isReturning_ = false;
}
void Stage::Update(float deltaTime)
{
	UpdateShake(deltaTime);
}

void Stage::Draw() const
{
	for (const auto& model : stagePillars_)
	{
		model.Draw();
	}
	for (const auto& model : stageModels_)
	{
		model.Draw();
	}

	stageModel_.Draw();
}


void Stage::StartShake()
{
	if (isShaking_) return;

	constexpr float MaxTiltAngle = DX_PI_F / 6.0f;

	tiltMotion_.Start(
		0.0f,
		MaxTiltAngle,
		0.2f,
		Motion::Easing::EaseOut
	);

	isShaking_ = true;
	isReturning_ = false;
}

void Stage::UpdateShake(float deltaTime)
{
	if (!isShaking_) return;

	tiltMotion_.Update(deltaTime);

	if (tiltMotion_.IsFinished() && !isReturning_)
	{
		tiltMotion_.SetDuration(tiltMotion_.GetDuration() * 3.0f); // 戻りは遅くする
		tiltMotion_.StartReverse();
		isReturning_ = true;
	}

	const float angle = tiltMotion_.GetValue();

	// StageのローカルY座標を支点の高さにする
	const float pivotY = 0.0f;

	// 柱
	for (int i = 0; i < stagePillars_.size(); ++i)
	{
		const Vec3 basePosition = pillarBasePositions_[i];

		// Stage中心から外側への方向
		Vec3 outward = basePosition - Vec3{};
		outward.y = 0.0f;
		outward = outward.Normalized();

		const float tiltX = outward.z * angle;
		const float tiltZ = -outward.x * angle;

		const Matrix4 rotation =
			Matrix4::CreateRotationX(tiltX) *
			Matrix4::CreateRotationZ(tiltZ);

		// 対象のXZ位置を中心に、StageのYを支点として回転
		const Vec3 pivot{
			basePosition.x,
			pivotY,
			basePosition.z
		};

		const Vec3 offset{
			0.0f,
			basePosition.y - pivot.y,
			0.0f
		};

		const Vec3 rotatedOffset = rotation.TransformVector(offset);

		stagePillars_[i].SetPosition(pivot + rotatedOffset);

		Vec3 modelRotation = pillarBaseRotations_[i];
		modelRotation.x += tiltX;
		modelRotation.z += tiltZ;
		stagePillars_[i].SetRotation(modelRotation);
	}

	// 足場
	for (int i = 0; i < stageModels_.size(); ++i)
	{
		const Vec3 basePosition = stageBasePositions_[i];

		Vec3 outward = basePosition - Vec3{};
		outward.y = 0.0f;
		outward = outward.Normalized();

		const float tiltX = outward.z * angle;
		const float tiltZ = -outward.x * angle;

		const Matrix4 rotation =
			Matrix4::CreateRotationX(tiltX) *
			Matrix4::CreateRotationZ(tiltZ);

		const Vec3 pivot{
			basePosition.x,
			pivotY,
			basePosition.z
		};

		const Vec3 offset{
			0.0f,
			basePosition.y - pivot.y,
			0.0f
		};

		const Vec3 rotatedOffset = rotation.TransformVector(offset);

		stageModels_[i].SetPosition(pivot + rotatedOffset);

		Vec3 modelRotation = stageBaseRotations_[i];
		modelRotation.x += tiltX;
		modelRotation.z += tiltZ;
		stageModels_[i].SetRotation(modelRotation);
	}

	if (isReturning_ && tiltMotion_.IsFinished())
	{
		isShaking_ = false;
		isReturning_ = false;
	}
}