// ==============================
// Gameplay/Collision/Collision.h
// ==============================
#pragma once
#include "Vector3.h"

struct ModelObject;
#ifdef _DEBUG
#include <functional>
class Player;
class Boss;
#endif
class ModelCollision;

namespace Collision									// あたり判定用名前空間
{
	//Axisの方向に沿った楕円体で、モデルのワールド空間の境界に合わせてフィットさせる。
    struct Ellipsoid
    {
        Vec3 center{};
        Vec3 radii{};
    };

	//近似値だけで、各軸の半径の合計を使用して判定する。
    bool IsHitEllipsoidEllipsoid(const Ellipsoid& a, const Ellipsoid& b);

	//モデルのコンタクトを更新し、接触している間は両方のモデルをハイライト表示する。
    bool UpdateModelContact(ModelObject& a, ModelObject& b);

	struct Capsule
	{
		Vec3 start{};
		Vec3 end{};
		float radius{};
	};

	struct Sphere							// 球のあたり判定情報
	{
		Vec3 center{};						// 中心座標
		float radius{};					    // 半径
        Vec3 rotate{};                      //角度
	};

	bool IsHitCapsuleSphere(const Capsule& capsule, const Sphere& sphere);

	bool IsHitSphereSphere(							// 球同士のあたり判定
		const Vec3& centerA, float radiusA,
		const Vec3& centerB, float radiusB);

	bool IsHitSphereSphere(							// 球同士のあたり判定
		const Sphere& sphereA, const Sphere& sphereB);
#ifdef _DEBUG
    struct DebugCanWatchState
    {
        bool touchingA{ false };
        bool touchingB{ false };
    };
    using DebugSphereProvider = std::function<Sphere()>;

    // 開始時に対象を登録する。判定・bool更新・描画は共通ループから実行する。
    void RegisterDebugCanWatch(const void* owner, DebugSphereProvider sphereA, DebugSphereProvider sphereB);

    // PlayerとBossを使う場合の登録。AがPlayer、BがBoss。
    void RegisterDebugCanWatch(const void* owner, const Player& player, const Boss& boss);
    void UnregisterDebugCanWatch(const void* owner);
    DebugCanWatchState GetDebugCanWatchState(const void* owner);

    // 共通ループから、シーン更新後に判定とboolを更新する。
    void UpdateDebug();
    // 共通ループから、シーン描画後に更新済みの球と接触状態を表示する。
    void DrawDebug();
    void ClearDebugCanWatch();
    // 球を1個だけ描く共通処理。登録した対象の描画でも同じ処理を使う。
    void DrawDebugSphere(const Sphere& sphere, unsigned int color);
#endif
	// 透明化したモデルを元に戻す。シーン更新の最初に呼ぶ。
    void ResetTransparentModels();
	// 透明化するモデルを登録する。シーン更新の最後に呼ぶ。
    void SetModelsBetweenTransparent(
        const Vec3& start,
        const Vec3& end,
        const ModelCollision& collision,
		const float alpha = 0.7f
    );
}