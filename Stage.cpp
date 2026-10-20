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
#include <algorithm>
#include <cmath>

#include "ModelObjectManager.h"

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

    for (auto& model : stagePillars_) model.SetOwner(*this);
    for (auto& model : stageModels_) model.SetOwner(*this);
    stageModel_.SetOwner(*this);

    SetupCollisionInfo();
    Reset();
}
void Stage::SetupCollisionInfo()//当たり判定のセットアップ
{
    const auto setup = [this](const ModelObject& model)//ラムダ式で書き換えれるように
    {
        int handle = RM().GetModel(model.modelKey);
        if (handle < 0) return -1;

        // 描画前でも、保存済みの配置・回転・拡大率で判定できるようにする。
        model.WithParentTransform(GetPosition(), GetRotation()).Draw();

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
    const auto refresh = [this](const ModelObject& model, int handle)
    {
        if (handle < 0) return -1;
        // モデルが解放・差し替えされていたら、古いハンドルは使わない。
        if (RM().GetModel(model.modelKey) != handle) return -1;

        model.WithParentTransform(GetPosition(), GetRotation()).Draw();

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

bool Stage::FindPillarWall(const Vec3& feetPosition, Vec3& wallNormal, const Vec3& moveDirection,
    float* wallTopY, bool keepTopContact) const
{
    wallNormal = {};
    if (wallTopY) *wallTopY = 0.0f;

    // 移動の衝突判定より少し広い範囲で側面を拾う。体の高さは変えない。
    const float detectionRadius = Const::PLAYER_RADIUS + Const::PLAYER_CLIMB_DETECTION_MARGIN;
    // Climbingの時足元が上にあるかの判定「
    const float lowerReach = keepTopContact ? Const::PLAYER_CLIMB_TOP_HOLD_DISTANCE : 0.0f;
    const Vec3 start = feetPosition + Vec3::Up() * (Const::PLAYER_RADIUS - lowerReach);
    const Vec3 end = feetPosition + Vec3::Up() * (Const::PLAYER_HEIGHT - Const::PLAYER_RADIUS);
    const float headY = feetPosition.y + Const::PLAYER_HEIGHT;

    const Vec3 direction = Vec3(moveDirection.x, 0.0f, moveDirection.z).Normalized();
    bool foundWall = false;
    float bestDirection = 2.0f;
    float bestDistance = detectionRadius;

    for (int handle : pillarCollisionHandles_)
    {
        if (handle < 0) continue;

        const auto result = MV1CollCheck_Capsule(handle, -1,
            DxConv::ToVECTOR(start),
            DxConv::ToVECTOR(end),
            detectionRadius);

        for (int i = 0; i < result.HitNum; ++i)
        {
            const auto& poly = result.Dim[i];
            const Vec3 normal = DxConv::ToVec3(poly.Normal).Normalized();

            // 向きがない面や、上下を向いた面・斜面は使わない。
            if (normal.LengthSq() == 0.0f || std::abs(normal.y) > 0.01f)
            {
                continue;
            }

            float top = (std::max)({ poly.Position[0].y, poly.Position[1].y, poly.Position[2].y });
            float bottom = (std::min)({ poly.Position[0].y, poly.Position[1].y, poly.Position[2].y });


            // 足元が上端付近に達しているか
            const bool atTop = feetPosition.y >= top - Const::PLAYER_SKIN;
            if (atTop)
            {
                // 通常時は、この側面を壁として扱わん
                if (!keepTopContact)
                {
                    continue;
                }

                // climbingで上まで言ったら
                const bool aboveTop = feetPosition.y > top + Const::PLAYER_SKIN;
                if (aboveTop)
                {
                    continue;
                }
            }

            // 体の高さと重ならない側面は使わない。
            if (headY <= bottom) continue;

            Vec3 outward = Vec3(normal.x, 0.0f, normal.z).Normalized();
            const float signedDistance = Vec3::Dot(feetPosition - DxConv::ToVec3(poly.Position[0]), outward);
            // モデルの面の表裏によらず、壁からプレイヤーへ向く法線にそろえる。
            if (signedDistance < 0.0f) outward = -outward;

            const float wallDirection = Vec3::Dot(direction, outward);
            const float distance = std::abs(signedDistance);
            // 角で複数の面に触れた場合も、入力が向かっている面を優先する。
            if (!foundWall || wallDirection < bestDirection - 0.0001f ||
                (std::abs(wallDirection - bestDirection) <= 0.0001f &&
                    distance < bestDistance))
            {
				wallNormal = outward;           // 壁の法線は、プレイヤーの進行方向に向かうようにそろえる。
				bestDirection = wallDirection;  // 壁の法線と進行方向の内積が最も小さい面を優先する。
                bestDistance = distance;        // 壁までの距離が最も近い面を優先する。
                if (wallTopY) *wallTopY = top;
				foundWall = true;               //  壁の法線が見つかったことを記録する。
            }
        }

        // 判定結果を解放してから、見つかったかどうかを返す。
        MV1CollResultPolyDimTerminate(result);
    }
    return foundWall;
}

Vec3 Stage::SlideAlongPillars(const Vec3& oldPosition, const Vec3& desiredPosition, bool& touched) const
{
   
    Vec3 move{ desiredPosition.x - oldPosition.x, 0.0f, desiredPosition.z - oldPosition.z };
    Vec3 candidate = desiredPosition;
    touched = false;
    // 隣接する柱の側面も考慮して、数回判定し直す。移動速度の増加や進行方向の自動補正は行わない。
    for (int attempt = 0; attempt < 4; ++attempt)
    {
        //移動してもPlayerの当たり判定を置いてる
        const Vec3 start = candidate + Vec3::Up() * Const::PLAYER_RADIUS;
        const Vec3 end = candidate + Vec3::Up() * (Const::PLAYER_HEIGHT - Const::PLAYER_RADIUS);

        Vec3 wallNormal{};
        float mostInward = -0.0001f;

        for (int handle : pillarCollisionHandles_)
        {
            if (handle < 0) continue;

            const auto result = MV1CollCheck_Capsule(handle, -1,
                DxConv::ToVECTOR(start), DxConv::ToVECTOR(end), Const::PLAYER_RADIUS);

            for (int i = 0; i < result.HitNum; ++i)
            {
                const auto& poly = result.Dim[i];
                Vec3 normal = DxConv::ToVec3(poly.Normal).Normalized();

                if (std::abs(normal.y) > 0.01f) continue; // 柱の上下面と斜面は対象外にする。

                float top = (std::max)(poly.Position[0].y,
                    (std::max)(poly.Position[1].y, poly.Position[2].y));

                if (candidate.y >= top - Const::PLAYER_SKIN) continue; // 柱の上にいる場合は側面との衝突を処理しない。

                normal.y = 0.0f;
                normal = normal.Normalized();
				//壁に向かう移動の際に、最も壁に向かう法線を採用する。
                float inward = Vec3::Dot(move, normal);

                if (inward < mostInward)
                { mostInward = inward; wallNormal = normal; }
            }
            MV1CollResultPolyDimTerminate(result);
        }
        if (wallNormal.LengthSq() == 0.0f) return candidate; // 壁から離れる移動も含む。
        touched = true;

        // 壁へ入り込む成分だけを取り除き、横方向に弾き飛ばさない。
        move -= wallNormal * Vec3::Dot(move, wallNormal);
        candidate = { oldPosition.x + move.x, desiredPosition.y, oldPosition.z + move.z };
    }
    // 最後の再判定でも移動できなければ、このフレームの移動前の水平位置を維持する。
    return { oldPosition.x, desiredPosition.y, oldPosition.z };
}


void Stage::Reset()
{
    // 初期位置・回転・拡大率はJSONファイルから読み込む
    MObjs().LoadActorTransform(*this);
    // モデルのロード
    for (auto& model : stagePillars_)
    {
        model.Load();
    }
    for (auto& model : stageModels_)
    {
        model.Load();
    }
    stageModel_.Load();



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
        model.WithParentTransform(GetPosition(), GetRotation()).Draw();
    }
    for (const auto& model : stageModels_)
    {
        model.WithParentTransform(GetPosition(), GetRotation()).Draw();
    }

    stageModel_.WithParentTransform(GetPosition(), GetRotation()).Draw();
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
