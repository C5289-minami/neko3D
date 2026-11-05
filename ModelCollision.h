#pragma once
#include "Vector3.h"
#include "Raycast.h"
#include "Collision.h"
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
        Floor,  // 足場。上面は着地、側面は滑り移動用の判定。
        Stand,  // 乗れる台。横からぶつかると止まり、上に乗ると足元がそろう。
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
    void AddFallRecoveryFloor(const ModelObject& model);
    // 手動で置く接触判定。床・壁には含めない。
    void SetContactCapsule(const Collision::Capsule& capsule) { contactCapsule_ = capsule; }
    const Collision::Capsule& GetContactCapsule() const { return contactCapsule_; }
    bool IsTouchingCapsule(const Collision::Sphere& sphere) const;
    void Clear();

    // 判定前に位置・回転・拡大率を反映する。描画する必要はない。
    // Player::Update は、この更新もまとめて呼ぶ。
    bool Refresh();

    //必要な判定を呼ぶ。座標の変更や登る状態への遷移は呼び出し側が行う。
    bool IsTouchingWalls(const Vec3& feetPosition, const ModelOb& modelOb) const;

    // 壁にぶつかる分だけ止まり、斜めに進むと壁沿いにすべる。柱は通り抜ける。
    Vec3 SlideAlongWalls(const Vec3& oldPosition, const Vec3& desiredPosition,
        const ModelOb& modelOb, bool& touched) const;

    // pillarCenterを指定すると、接触判定で選ばれた柱の中心も返す。
    bool FindWall(const Vec3& feetPosition, const ModelOb& modelOb, Vec3& wallNormal,
        const Vec3& moveDirection = {}, float* wallTopY = nullptr,
        float detectionMargin = 0.0f, float topHoldDistance = 0.0f,
        Vec3* pillarCenter = nullptr, const ModelObject** pillarModel = nullptr) const;
    bool GetPillarBounds(const ModelObject* model, Vec3& center, float& topY) const;

    // 種類を問わず全モデルを調べる、汎用のレイ判定。
    bool Raycast(const Vec3& start, const Vec3& end, Physics::RayHit& hit) const;
    std::vector<ModelObject*> GetModelsHitByRay(
        const Vec3& start,
        const Vec3& end
    ) const;
    // 着地用。床だけを調べる。柱は接触判定のみ。
    bool RaycastDown(const Vec3& origin, float maxDistance, Physics::RayHit& hit,
        bool* hitRecoveryFloor = nullptr) const;

    bool HasFallRecoveryFloor() const { return fallRecoveryFloor_ != nullptr; }
    bool HasFallRecoverySupport(const Vec3& position, const ModelOb& body, float walkableNormalY) const;
    bool CanLandOnFallRecoveryFloor(const Vec3& position, float skin, float snapDistance, float walkableNormalY) const;

    // 柱の上面だけへの着地。足元の調整量だけ判定面を下げる。
    bool RaycastPillarTop(const Vec3& origin, float maxDistance, float topOffset,
        Physics::RayHit& hit) const;
    // 頭上用。床・壁を調べる。柱は接触判定のみ。
    bool RaycastUp(const Vec3& origin, float maxDistance, Physics::RayHit& hit) const;
    // Refresh 後の、拡大縮小・回転を含めたモデルの幅・高さ・奥行き。
    Vec3 GetSize(const ModelObject& model) const;


private:
    struct Entry;
    const Entry* GetFallRecoveryFloor() const;
    bool UpdateBounds(Entry& entry);
    bool FindStandSide(const Vec3& feetPosition, const ModelOb& modelOb,
    const Vec3& startPosition, const Vec3& direction, Vec3& wallNormal) const;
    void SweepStands(const Vec3& startPosition, const Vec3& direction, const ModelOb& modelOb,
    float& travel, Vec3& wallNormal) const;

    bool RaycastModels(const Vec3& start, const Vec3& end, Physics::RayHit& hit,
        bool forLanding, bool ignorePillars = false, bool* hitRecoveryFloor = nullptr) const;
    bool RaycastWall(const Vec3& start, const Vec3& end, Physics::RayHit& hit) const;

    struct Entry
    {
        const ModelObject* model{};
        Surface surface{ Surface::Floor };
        int sourceHandle{ -1 };
        int handle{ -1 }; // 同じモデルを複数配置しても混ざらない、判定専用の複製。
        Vec3 center{};
        Vec3 radii{};
        float topY{};
        bool boundsReady{ false };
    };
    std::vector<Entry> models_;
    const ModelObject* fallRecoveryFloor_{ nullptr };
    Collision::Capsule contactCapsule_{};
};
