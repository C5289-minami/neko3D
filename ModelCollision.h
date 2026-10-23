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
    enum class Surface//表面
    {
        Ground,        // 着地だけに使う床・足場。
        Wall,          // 着地と、側面に沿う滑りに使う壁。
        ClimbableWall  // 壁の判定に加えて、登る面も探せる柱など。
    };

    // 動く側の体の大きさ。プレイヤー以外にも同じ判定を使える。
    // 旧名: Body
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

    // 1. モデルごとに、床・壁・登れる壁のどれとして使うか登録する。
    void AddModel(const ModelObject& model, Surface surface = Surface::Wall);
    void Clear();

    // 2. 判定前に位置・回転・拡大率を反映する。描画する必要はない。
    // Player::Update は、この更新もまとめて呼ぶ。
    bool Refresh();

    // 3. 必要な判定を呼ぶ。座標の変更や登る状態への遷移は呼び出し側が行う。
    bool IsTouchingWalls(const Vec3& feetPosition, const ModelOb& modelOb) const;
    Vec3 SlideAlongWalls(const Vec3& oldPosition, const Vec3& desiredPosition,
        const ModelOb& modelOb, bool& touched) const;
    bool FindWall(const Vec3& feetPosition, const ModelOb& modelOb, Vec3& wallNormal,
        const Vec3& moveDirection = {}, float* wallTopY = nullptr,
        float detectionMargin = 0.0f, float topHoldDistance = 0.0f) const;

    bool Raycast(const Vec3& start, const Vec3& end, Physics::RayHit& hit) const;
    bool RaycastDown(const Vec3& origin, float maxDistance, Physics::RayHit& hit) const;
    // Refresh 後の、拡大縮小・回転を含めたモデルの幅・高さ・奥行き。
    Vec3 GetSize(const ModelObject& model) const;

private:
    struct Entry
    {
        const ModelObject* model{};
        Surface surface{ Surface::Wall };
        int sourceHandle{ -1 };
        int handle{ -1 }; // 同じモデルを複数配置しても混ざらない、判定専用の複製。
    };
    std::vector<Entry> models_;
};
