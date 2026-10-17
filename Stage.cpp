// =============================
// Gameplay/Actors/Stage.cpp
// =============================
#include "Stage.h"
#include "ResourceManager.h"
#include "ResourceKeys.h"
#include "DxConv.h"
#include "Consts.h"
#include "imgui.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include <stdexcept>

namespace
{
    // 透過表示するステージモデルの大きさと、プレイヤー頭上の余白。
    float AlphaStageScale = 0.15f;
    float AlphaStageHeadClearance = 40.0f;

    float AlphaStageFadeSpeed = 2.4f;
}

void Stage::Init()
{
    //int modelHandle = DxLib::MV1DuplicateModel(RM().GetModel(ResourceKeys::Model_Stage));
    stagePillars_[0].modelKey = ResourceKeys::Model_hashira_l;
    stagePillars_[1].modelKey = ResourceKeys::Model_hashira_m;
    stagePillars_[2].modelKey = ResourceKeys::Model_hashira_s;

    stageModels_[0].modelKey = ResourceKeys::Model_stand_l;
    stageModels_[1].modelKey = ResourceKeys::Model_stand_m;

    // 地面の当たり判定に使うモデル
    stageModel_.modelKey = ResourceKeys::Model_StageFbx;

    // 編集対象に設定
    for (auto& model : stagePillars_)
    {
		model.Load();
    }
    for (auto& model : stageModels_)
    {
        model.Load(); 
    }
    stageModel_.Load(); 

    SetupCollisionInfo();
    Reset();
}
void Stage::SetupCollisionInfo()//当たり判定のセットアップ
{
    const auto setup = [](const ModelObject& model)//ラムダ式で書き換えれるように
    {
        int handle = RM().GetModel(model.modelKey);
        if (handle < 0) return -1;

        // 描画前でも、保存済みの配置・回転・拡大率で判定できるようにする。
        model.Draw();

        int result = MV1SetupCollInfo(handle, -1, 8, 8, 8);
        if (result < 0) return -1;

        return handle;
    };

    groundCollisionHandle_ = setup(stageModel_);
    for (int i = 0; i < stagePillars_.size(); ++i)
    {
        pillarCollisionHandles_[i] = setup(stagePillars_[i]);
    }
    for (int i = 0; i < stageModels_.size(); ++i)
    {
        stageCollisionHandles_[i] = setup(stageModels_[i]);
    }
    // コリジョン情報はRMによるMV1DeleteModel時にモデルと一緒に解放される。
}

void Stage::RefreshCollisionInfo()
{
    const auto refresh = [](const ModelObject& model, int handle)
    {
        if (handle < 0) return -1;
        // モデルが解放・差し替えされていたら、古いハンドルは使わない。
        if (RM().GetModel(model.modelKey) != handle) return -1;

        model.Draw();

		int result = MV1RefreshCollInfo(handle, -1);// コリジョン情報を更新する
        if (result < 0) return -1;

        return handle;
    };

    groundCollisionHandle_ = refresh(stageModel_, groundCollisionHandle_);
    for (int i = 0; i < stagePillars_.size(); ++i)
    {
        pillarCollisionHandles_[i] = refresh(stagePillars_[i], pillarCollisionHandles_[i]);
    }
    for (int i = 0; i < stageModels_.size(); ++i)
    {
        stageCollisionHandles_[i] = refresh(stageModels_[i], stageCollisionHandles_[i]);
    }
}

void Stage::GetCollisionModelHandles(std::vector<int>& handles) const
{
    // 呼び出し元の配列を、現在使えるハンドルの一覧に更新する。
    handles.clear();
    handles.reserve(pillarCollisionHandles_.size() + stageCollisionHandles_.size() + 1);

    if (groundCollisionHandle_ >= 0)
    {
        handles.emplace_back(groundCollisionHandle_);
    }
    for (int handle : pillarCollisionHandles_)
    {
        if (handle >= 0)
        {
            handles.emplace_back(handle);
        }
    }
    for (int handle : stageCollisionHandles_)
    {
        if (handle >= 0)
        {
            handles.emplace_back(handle);
        }
    }
}

bool Stage::IsHitPillar(const Vec3& capsuleStart, const Vec3& capsuleEnd, float radius) const
{
    if (radius <= 0.0f) return false;

    for (int handle : pillarCollisionHandles_)
    {
        if (handle < 0) continue;

        const auto result = MV1CollCheck_Capsule(
            handle, -1, DxConv::ToVECTOR(capsuleStart), DxConv::ToVECTOR(capsuleEnd), radius);
        const bool hit = result.HitNum > 0;
        // 判定結果の配列は、当たった場合も当たらない場合も解放する。
        MV1CollResultPolyDimTerminate(result);
        if (hit) return true;
    }
    return false;
}

void Stage::Reset()
{
    RefreshCollisionInfo();
    // 透過モデルの配置情報は毎回リセット
    alphaModelReady_ = false;

    // リセット直後は透過効果が残らないように不透明度を初期値へ
    alphaModel_.SetAlpha(1.0f);
}

void Stage::Update(float deltaTime)
{
}

void Stage::Draw() const
{
    // 柱と台座をそれぞれ描画
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
void Stage::ResetAlphaModel(const Vec3& playerPosition)
{

    alphaModelReady_ = false;

    // ResourceKeys
    alphaModel_.modelKey = ResourceKeys::Model_StageAlpha;
    alphaModel_.scale = { AlphaStageScale, AlphaStageScale, AlphaStageScale };
    alphaModel_.rotation = {};
    alphaModel_.center = {};
    alphaModel_.SetAlpha(1.0f);

    const int handle = RM().GetModel(ResourceKeys::Model_StageAlpha);
    if (handle < 0) return;

    MV1SetPosition(handle, VGet(0.0f, 0.0f, 0.0f));
    MV1SetRotationXYZ(handle, VGet(0.0f, 0.0f, 0.0f));
    MV1SetScale(handle, DxConv::ToVECTOR(alphaModel_.scale));// スケールを適用してから範囲を取得するため、先にスケールをセットする。
    if (MV1SetupReferenceMesh(handle, -1, TRUE, TRUE) < 0) return;

    MV1RefreshReferenceMesh(handle, -1, TRUE, TRUE);
    const auto mesh = MV1GetReferenceMesh(handle, -1, TRUE, TRUE);
    const bool hasVertices = mesh.VertexNum > 0;

    //スケール適用後のモデルの範囲として保存する。
    const Vec3 minimum{ mesh.MinPosition.x, mesh.MinPosition.y, mesh.MinPosition.z };
    const Vec3 maximum{ mesh.MaxPosition.x, mesh.MaxPosition.y, mesh.MaxPosition.z };

    // 参照メッシュは範囲を取得したら解放する。
	MV1TerminateReferenceMesh(handle, -1, TRUE, TRUE);// 参照メッシュを解放しないと、モデルの範囲が正しく取得できない場合がある。 へぇ
    if (!hasVertices) return;

   //Playerの頭の上に出てくる
    alphaModel_.position = {
        playerPosition.x - (minimum.x + maximum.x) * 0.5f,
        playerPosition.y + Const::PLAYER_HEIGHT + AlphaStageHeadClearance - minimum.y,
        playerPosition.z - (minimum.z + maximum.z) * 0.5f
    };

    alphaModelReady_ = true;
}

void Stage::UpdateAlphaModel(float deltaTime, const Player& player)
{
    if (!alphaModelReady_) return;

    alphaModel_.FadeTo(player, AlphaStageFadeSpeed, deltaTime);
}

void Stage::DrawAlphaModel() const
{
    if (alphaModelReady_) alphaModel_.Draw();
}

void Stage::DrawDebugUI()
{
 
}
