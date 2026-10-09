// =============================
// Gameplay/Actors/Stage.cpp
// =============================
#include "Stage.h"
#include "ResourceManager.h"
#include "ResourceKeys.h"
#include "DxConv.h"
#include "Raycast.h"
#include "Consts.h"

namespace
{
    // 透過表示するステージモデルの大きさと、プレイヤー頭上の余白。
    float AlphaStageScale = 0.15f;
    float AlphaStageHeadClearance = 40.0f;

    float AlphaStageFadeSpeed = 2.4f;
}

void Stage::Init()
{
    // 通常のステージモデルをリソース管理側から取得する。
    modelHandle = RM().GetModel(ResourceKeys::Model_Stage);
    if (modelHandle < 0) return;

    // 地面との当たり判定に使うコリジョン情報を生成する。
    MV1SetupCollInfo(modelHandle, -1, 8, 8, 8);
}

void Stage::Reset()
{
    // 透過モデルの配置情報は毎回リセット
    alphaModelReady_ = false;

    // リセット直後は透過効果が残らないように不透明度を初期値へ
    alphaModel_.SetAlpha(1.0f);
}

void Stage::Draw() const
{
    if (modelHandle < 0) return;

    // 通常のステージは、プレイヤーとの距離に関係なく描画する。
    MV1DrawModel(modelHandle);
}

void Stage::ResetAlphaModel(const Vec3& playerPosition)
{
  
    alphaModelReady_ = false;

    // ResourceKeys
    alphaModel_.model.modelKey = ResourceKeys::Model_StageAlpha;
    alphaModel_.model.scale = { AlphaStageScale, AlphaStageScale, AlphaStageScale };
    alphaModel_.model.rotation = {};
    alphaModel_.model.center = {};
    alphaModel_.SetAlpha(1.0f);

    const int handle = RM().GetModel(ResourceKeys::Model_StageAlpha);
    if (handle < 0) return;

    MV1SetPosition(handle, VGet(0.0f, 0.0f, 0.0f));
    MV1SetRotationXYZ(handle, VGet(0.0f, 0.0f, 0.0f));
    MV1SetScale(handle, DxConv::ToVECTOR(alphaModel_.model.scale));// スケールを適用してから範囲を取得するため、先にスケールをセットする。
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
    alphaModel_.model.position = {
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