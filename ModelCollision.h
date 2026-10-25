#pragma once
#include "Vector3.h"
#include "Raycast.h"
#include <vector>

struct ModelObject;

// シーンに置いたモデルを登録して使う、共通の当たり判定。
// 登録した ModelObject は、Clear() するまで生存させておく。
class ModelCollision
{
public:
    // 登録したモデルに使う判定を選ぶ。
    enum class Surface
    {
        Floor,  // 床。着地用の判定。
        Wall,   // 壁。側面で移動を止める・滑らせる。登れない。
        Pillar  
    };

    // 動く側の体の大きさ。プレイヤー以外にも同じ判定を使える。
    struct ModelOb
    {
        float radius{};
        float height{};
        float skin{ 1.0f };
        bool IsValid() const { return radius > 0.0f && height >= radius * 2.0f && skin >= 0.0f; }
    };

    ModelCollision() = default;
    ~ModelCollision();
    ModelCollision(const ModelCollision&) = delete;
    ModelCollision& operator=(const ModelCollision&) = delete;

    // 判定の種類を1つ指定して登録する。同じモデル・同じ種類の重複登録はしない。
    //  AddModel(floor, Surface::Floor);
    //  AddModel(wall, Surface::Wall);
    //  AddModel(pillar, Surface::Pillar);
    void AddModel(const ModelObject& model, Surface surface);
    void Clear();

    // 判定前に位置・回転・拡大率を反映する。描画する必要はない。
    // Player::Update は、この更新もまとめて呼ぶ。
    bool Refresh();

    //必要な判定を呼ぶ。座標の変更や登る状態への遷移は呼び出し側が行う。
    bool IsTouchingWalls(const Vec3& feetPosition, const ModelOb& modelOb) const;
    Vec3 SlideAlongWalls(const Vec3& oldPosition, const Vec3& desiredPosition,
        const ModelOb& modelOb, bool& touched) const;
    // pillarCenterを指定すると、接触判定で選ばれた柱の中心も返す。
    bool FindWall(const Vec3& feetPosition, const ModelOb& modelOb, Vec3& wallNormal,
        const Vec3& moveDirection = {}, float* wallTopY = nullptr,
        float detectionMargin = 0.0f, float topHoldDistance = 0.0f,
        Vec3* pillarCenter = nullptr) const;

    // 種類を問わず全モデルを調べる、汎用のレイ判定。
    bool Raycast(const Vec3& start, const Vec3& end, Physics::RayHit& hit) const;
    // 着地用。床だけを調べる。柱は接触判定のみ。
    bool RaycastDown(const Vec3& origin, float maxDistance, Physics::RayHit& hit) const;
    // 頭上用。床・壁を調べる。柱は接触判定のみ。
    bool RaycastUp(const Vec3& origin, float maxDistance, Physics::RayHit& hit) const;
    // Refresh 後の、拡大縮小・回転を含めたモデルの幅・高さ・奥行き。
    Vec3 GetSize(const ModelObject& model) const;

private:
    bool RaycastModels(const Vec3& start, const Vec3& end, Physics::RayHit& hit,
        bool forLanding, bool ignorePillars = false) const;

    struct Entry
    {
        const ModelObject* model{};
        Surface surface{ Surface::Floor };
        int sourceHandle{ -1 };
        int handle{ -1 }; // 同じモデルを複数配置しても混ざらない、判定専用の複製。
    };
    std::vector<Entry> models_;
};
