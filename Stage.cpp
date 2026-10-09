// =============================
// Gameplay/Actors/Stage.cpp
// =============================
#include "Stage.h"
#include "ResourceManager.h"
#include "ResourceKeys.h"
#include "DxConv.h"
#include "Raycast.h"
#include "Consts.h"

#include <algorithm>
#include <cmath>

namespace
{
    // 透過表示するステージモデルの大きさと、プレイヤー頭上の余白。
    float AlphaStageScale = 0.15f;
    float AlphaStageHeadClearance = 40.0f;

    // プレイヤーが近いときは薄く、離れるほど不透明にする。
    float AlphaStageMinOpacity = 0.4f;
	float AlphaStageFadeStart = Const::PLAYER_RADIUS * 4.0f;//半径の4倍の距離　Playerの半径の4倍
    float AlphaStageFadeEnd = Const::PLAYER_RADIUS;         
    float AlphaStageFadeSpeed = 2.4f;

    // 値が[min, max]の範囲内なら0、範囲外なら最も近い端までの距離を返す。
    float DistanceToInterval(float value, float minimum, float maximum)
    {
        return std::max({ minimum - value, value - maximum, 0.0f });
    }
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

    // 以降の距離判定で使う、配置後のモデル範囲をワールド座標で保持する。
    alphaBoundsMin_ = minimum + alphaModel_.model.position;
    alphaBoundsMax_ = maximum + alphaModel_.model.position;
    alphaModelReady_ = true;
}

void Stage::UpdateAlphaModel(float deltaTime, const Vec3& playerPosition)
{
    if (!alphaModelReady_) return;

    // 透過モデルは見た目のためだけに使い、当たり判定には通常モデルを使う。
    // 各軸でモデルの範囲外に出た距離を求める。範囲内ならその軸の距離は0。
    const float dx = DistanceToInterval(playerPosition.x, alphaBoundsMin_.x, alphaBoundsMax_.x);
    const float dz = DistanceToInterval(playerPosition.z, alphaBoundsMin_.z, alphaBoundsMax_.z);
    const float horizontalDistance = std::sqrt(dx * dx + dz * dz);

    // プレイヤーの足元ではなく、頭の高さまでを使って上下方向の距離を判定する。
    const float verticalDistance = DistanceToInterval(
        playerPosition.y + Const::PLAYER_HEIGHT, alphaBoundsMin_.y, alphaBoundsMax_.y);

    // 水平方向と垂直方向の距離から、透明度計算に使う距離を決める。
    const float distance = std::max(horizontalDistance, verticalDistance);

    // フェード範囲内の距離を0～1に変換する。
    // 近いほどtは0に近く、遠いほど1に近くなる。
    float t = std::clamp((distance - AlphaStageFadeEnd) /
        (AlphaStageFadeStart - AlphaStageFadeEnd), 0.0f, 1.0f);

    // Smoothstepで変化をなめらかにし、透明度が急に切り替わるのを防ぐ。
    t = t * t * (3.0f - 2.0f * t);

    // 近距離では最低不透明度、フェード範囲の外では完全不透明にする。
    const float targetAlpha = AlphaStageMinOpacity + (1.0f - AlphaStageMinOpacity) * t;
    alphaModel_.FadeTo(targetAlpha, AlphaStageFadeSpeed, deltaTime);
}

void Stage::DrawAlphaModel() const
{
    if (alphaModelReady_) alphaModel_.Draw();
}