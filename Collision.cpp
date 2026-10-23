// ================================
// Gameplay/Collision/Collision.cpp
// ================================
#include "Collision.h"
#include "DxConv.h"
#include "DxPlus/Debug.h"
#include "Player.h"
#include "Boss.h"
#include <cmath>

#include <vector>

namespace Collision
{
	bool IsHitSphereSphere(const Vec3& centerA, float radiusA,
		const Vec3& centerB, float radiusB)
	{
		// 2つの球の中心の差を求める
		Vec3 diff = centerB - centerA;

		// 2つの球の半径を足す
		float radiusSum = radiusA + radiusB;

		// 中心間距離が半径の合計以下なら当たっている
		return diff.LengthSq() <= radiusSum * radiusSum;
	}

	bool IsHitSphereSphere(const Sphere& sphereA, const Sphere& sphereB)
	{
		// Sphereの中心座標と半径を使って判定する
		return IsHitSphereSphere(sphereA.center, sphereA.radius,
			sphereB.center, sphereB.radius);
	}
}
namespace Collision
{
    bool IsHitEllipsoidEllipsoid(const Ellipsoid& a, const Ellipsoid& b)
    {

		// 2つの楕円体の半径が0以下だったら当たり判定を行わない
		if (a.radii.x <= 0.0f || a.radii.y <= 0.0f || a.radii.z <= 0.0f ||
            b.radii.x <= 0.0f || b.radii.y <= 0.0f || b.radii.z <= 0.0f)
        {
            return false;
        }

        const Vec3 difference = b.center - a.center;
        const Vec3 radiiSum = a.radii + b.radii;
        const Vec3 normalized{
            difference.x / radiiSum.x,
            difference.y / radiiSum.y,
            difference.z / radiiSum.z
        };
        return normalized.LengthSq() <= 1.0f;
    }



}

#ifdef _DEBUG
namespace
{
    struct DebugCanWatchEntry
    {
        const void* owner{};
        Collision::DebugSphereProvider sphereA;
        Collision::DebugSphereProvider sphereB;
        const Player* player{ nullptr };
        const Boss* boss{ nullptr };
        Collision::DebugCanWatchState state{};
        Collision::Sphere drawSphere{};
        bool hasUpdated{ false };
        const wchar_t* nameA{ L"A" };
        const wchar_t* nameB{ L"B" };
    };

    std::vector<DebugCanWatchEntry>& DebugCanWatchEntries()
    {
        static std::vector<DebugCanWatchEntry> contacts;
        return contacts;
    }

    DebugCanWatchEntry* FindDebugCanWatch(const void* owner)
    {
        std::vector<DebugCanWatchEntry>& contacts = DebugCanWatchEntries();

        // 先頭から順番に、同じownerの登録を探す。
        for (std::size_t i = 0; i < contacts.size(); i++)
        {
            if (contacts[i].owner == owner)
            {
                return &contacts[i];
            }
        }

        return nullptr;
    }

    Collision::Sphere GetPlayerDebugSphere(const Player& player)
    {
        Collision::Sphere sphere;
        sphere.center = player.GetPosition();
        sphere.center.y += Const::PLAYER_HEIGHT * 0.5f;
        sphere.radius = Const::PLAYER_RADIUS;
        return sphere;
    }

    Collision::Sphere GetBossDebugSphere(const Boss& boss)
    {
        Collision::Ellipsoid body = boss.GetModelObject().GetHitEllipsoid();

        Collision::Sphere sphere;
        sphere.center = body.center;
        sphere.radius = body.radii.x;

        // x、y、zの中で一番大きい半径を使う。
        if (body.radii.y > sphere.radius)
        {
            sphere.radius = body.radii.y;
        }
        if (body.radii.z > sphere.radius)
        {
            sphere.radius = body.radii.z;
        }

        return sphere;
    }

    Vec3 GetDebugSpherePoint(const Collision::Sphere& sphere, float latitude, float longitude)
    {
        Vec3 point;
        point.x = std::sin(latitude) * std::cos(longitude);
        point.y = std::cos(latitude);
        point.z = std::sin(latitude) * std::sin(longitude);
        point = point * sphere.radius;
        point = point + sphere.center;
        return point;
    }
}

void Collision::RegisterDebugCanWatch(const void* owner, DebugSphereProvider sphereA, DebugSphereProvider sphereB)
{
    // 同じownerで登録し直したときは、前の登録を消す。
    UnregisterDebugCanWatch(owner);

    if (owner == nullptr)
    {
        return;
    }
    if (!sphereA)
    {
        return;
    }
    if (!sphereB)
    {
        return;
    }

    DebugCanWatchEntry entry;
    entry.owner = owner;
    entry.sphereA = sphereA;
    entry.sphereB = sphereB;

    std::vector<DebugCanWatchEntry>& contacts = DebugCanWatchEntries();
    contacts.push_back(entry);
}

void Collision::RegisterDebugCanWatch(const void* owner, const Player& player, const Boss& boss)
{
    UnregisterDebugCanWatch(owner);

    if (owner == nullptr)
    {
        return;
    }

    // キャラクターを覚えておき、UpdateDebugで現在の球を取得する。
    DebugCanWatchEntry entry;
    entry.owner = owner;
    entry.player = &player;
    entry.boss = &boss;
    entry.nameA = L"Player";
    entry.nameB = L"Boss";

    std::vector<DebugCanWatchEntry>& contacts = DebugCanWatchEntries();
    contacts.push_back(entry);
}

void Collision::UnregisterDebugCanWatch(const void* owner)
{
    std::vector<DebugCanWatchEntry>& contacts = DebugCanWatchEntries();

    for (std::size_t i = 0; i < contacts.size(); i++)
    {
        if (contacts[i].owner == owner)
        {
            contacts.erase(contacts.begin() + i);
            return;
        }
    }
}

Collision::DebugCanWatchState Collision::GetDebugCanWatchState(const void* owner)
{
    DebugCanWatchEntry* entry = FindDebugCanWatch(owner);
    if (entry != nullptr)
    {
        return entry->state;
    }

    DebugCanWatchState emptyState;
    return emptyState;
}

void Collision::ClearDebugCanWatch()
{
    DebugCanWatchEntries().clear();
}

void Collision::UpdateDebug()
{
    std::vector<DebugCanWatchEntry>& contacts = DebugCanWatchEntries();

    for (std::size_t i = 0; i < contacts.size(); i++)
    {
        DebugCanWatchEntry& entry = contacts[i];
        Sphere a;
        Sphere b;

        if (entry.player != nullptr)
        {
            a = GetPlayerDebugSphere(*entry.player);
        }
        else
        {
            a = entry.sphereA();
        }

        if (entry.boss != nullptr)
        {
            b = GetBossDebugSphere(*entry.boss);
        }
        else
        {
            b = entry.sphereB();
        }

        // 半径が有効な球だけ、当たり判定を行う。
        bool touching = false;
        if (a.radius > 0.0f)
        {
            if (b.radius > 0.0f)
            {
                touching = IsHitSphereSphere(a, b);
            }
        }

        entry.state.touchingA = touching;
        entry.state.touchingB = touching;
        entry.drawSphere = a;
        entry.hasUpdated = true;
    }
}

void Collision::DrawDebug()
{
    std::vector<DebugCanWatchEntry>& contacts = DebugCanWatchEntries();

    for (std::size_t i = 0; i < contacts.size(); i++)
    {
        const DebugCanWatchEntry& entry = contacts[i];
        if (!entry.hasUpdated)
        {
            continue;
        }

        unsigned int color = DxLib::GetColor(80, 220, 255);
        if (entry.state.touchingA)
        {
            color = DxLib::GetColor(255, 220, 60);
        }
        DrawDebugSphere(entry.drawSphere, color);

        const wchar_t* textA = L"false";
        const wchar_t* textB = L"false";
        if (entry.state.touchingA)
        {
            textA = L"true";
        }
        if (entry.state.touchingB)
        {
            textB = L"true";
        }

        DxPlus::Debug::SetFormatString(L"接触 %s: %s / %s: %s",
            entry.nameA, textA, entry.nameB, textB);
    }
}

void Collision::DrawDebugSphere(const Sphere& sphere, unsigned int color)
{
    if (sphere.radius <= 0.0f) return;
    constexpr int meridians = 6;
    constexpr int arcSegments = 16;
    constexpr int equatorSegments = 32;
    constexpr float pi = 3.14159265358979323846f;
    // 塗りつぶしや細かい横線を描かず、6本の縦線で形を確認する。
    for (int line = 0; line < meridians; ++line)
    {
        const float longitude = 2.0f * pi * line / meridians;
        Vec3 previous = GetDebugSpherePoint(sphere, 0.0f, longitude);
        for (int segment = 1; segment <= arcSegments; ++segment)
        {
            const Vec3 next = GetDebugSpherePoint(sphere, pi * segment / arcSegments, longitude);
            DxLib::DrawLine3D(DxConv::ToVECTOR(previous), DxConv::ToVECTOR(next), color);
            previous = next;
        }
    }
    Vec3 previous = GetDebugSpherePoint(sphere, pi * 0.5f, 0.0f);
    for (int segment = 1; segment <= equatorSegments; ++segment)
    {
        const Vec3 next = GetDebugSpherePoint(sphere, pi * 0.5f, 2.0f * pi * segment / equatorSegments);
        DxLib::DrawLine3D(DxConv::ToVECTOR(previous), DxConv::ToVECTOR(next), color);
        previous = next;
    }
}
#endif
