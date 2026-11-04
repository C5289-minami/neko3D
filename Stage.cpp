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

    for (const auto& model : stagePillars_)
        collision_.AddModel(model, ModelCollision::Surface::Pillar);

    for (const auto& model : stageModels_)
        collision_.AddModel(model, ModelCollision::Surface::Stand);

    collision_.Refresh();

}
void Stage::Update(float deltaTime)
{
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



void Stage::DrawDebugUI()
{

}
