// ================================
// Gameplay/Collision/Collision.cpp
// ================================

#include "Collision.h"
#include "ModelCollision.h"
#include "DrawableObject.h"

#include "DxConv.h"
#include "DxPlus/Debug.h"
#include "DxLib.h"

#include "Player.h"
#include "Boss.h"
#include "Consts.h"

#include <cmath>
#include <vector>
#include <algorithm>

// =====================================
// ”¼“§–¾ƒ‚ƒfƒ‹ŠÇ—
// =====================================
namespace
{
    struct TransparentModelData
    {
        ModelObject* model{};
        float originalAlpha{ 1.0f };
    };

    std::vector<TransparentModelData> transparentModels;
}

// =====================================
// Collision
// =====================================
namespace Collision
{
    bool IsHitSphereSphere(
        const Vec3& centerA,
        float radiusA,
        const Vec3& centerB,
        float radiusB)
    {
        // 2‚Â‚Ì‹…‚Ì’†S‚Ì·‚ğ‹‚ß‚é
        Vec3 diff = centerB - centerA;

        // 2‚Â‚Ì‹…‚Ì”¼Œa‚ğ‘«‚·
        float radiusSum = radiusA + radiusB;

        // ’†SŠÔ‹——£‚ª”¼Œa‚Ì‡ŒvˆÈ‰º‚È‚ç“–‚½‚Á‚Ä‚¢‚é
        return diff.LengthSq() <= radiusSum * radiusSum;
    }

    bool IsHitSphereSphere(const Sphere& sphereA, const Sphere& sphereB)
    {
        // Sphere‚Ì’†SÀ•W‚Æ”¼Œa‚ğg‚Á‚Ä”»’è‚·‚é
        return IsHitSphereSphere(
            sphereA.center, sphereA.radius,
            sphereB.center, sphereB.radius);
    }

    bool IsHitCapsuleSphere(const Capsule& capsule, const Sphere& sphere)
    {
        // ‹…‚Ì’†S‚ğƒJƒvƒZƒ‹‚Ìü•ª‚É“Š‰e‚·‚é
        const Vec3 segment = capsule.end - capsule.start;
        const float segmentLengthSq = segment.LengthSq();

        float t = 0.0f;

        if (segmentLengthSq > 0.0f)
        {
            t = Vec3::Dot(
                sphere.center - capsule.start,
                segment) / segmentLengthSq;

            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;
        }

        // ü•ªã‚ÌÅ‹ß“_
        const Vec3 closestPoint = capsule.start + segment * t;

        const float radiusSum = capsule.radius + sphere.radius;

        return (sphere.center - closestPoint).LengthSq()
            <= radiusSum * radiusSum;
    }

    bool IsHitEllipsoidEllipsoid(
        const Ellipsoid& a,
        const Ellipsoid& b)
    {
        // ”¼Œa‚ª0ˆÈ‰º‚¾‚Á‚½‚ç”»’è‚µ‚È‚¢
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

// =====================================
// Debug—p‚Ì“à•”ˆ—
// =====================================
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

        // xAyAz‚Ì’†‚Åˆê”Ô‘å‚«‚¢”¼Œa‚ğg‚¤
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

    Vec3 GetDebugSpherePoint(
        const Collision::Sphere& sphere,
        float latitude,
        float longitude)
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

// =====================================
// Debug—p‚ÌŒöŠJŠÖ”
// =====================================

void Collision::RegisterDebugCanWatch(
    const void* owner,
    DebugSphereProvider sphereA,
    DebugSphereProvider sphereB)
{
    // “¯‚¶owner‚Å“o˜^‚µ’¼‚µ‚½‚Æ‚«‚Í‘O‚Ì“o˜^‚ğÁ‚·
    UnregisterDebugCanWatch(owner);

    if (owner == nullptr) return;
    if (!sphereA) return;
    if (!sphereB) return;

    DebugCanWatchEntry entry;

    entry.owner = owner;
    entry.sphereA = sphereA;
    entry.sphereB = sphereB;

    DebugCanWatchEntries().push_back(entry);
}

void Collision::RegisterDebugCanWatch(
    const void* owner,
    const Player& player,
    const Boss& boss)
{
    UnregisterDebugCanWatch(owner);

    if (owner == nullptr) return;

    // Player‚ÆBoss‚ğ“o˜^‚·‚é
    DebugCanWatchEntry entry;

    entry.owner = owner;
    entry.player = &player;
    entry.boss = &boss;

    entry.nameA = L"Player";
    entry.nameB = L"Boss";

    DebugCanWatchEntries().push_back(entry);
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

Collision::DebugCanWatchState Collision::GetDebugCanWatchState(
    const void* owner)
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

        // ”¼Œa‚ª—LŒø‚È‹…‚¾‚¯”»’è‚·‚é
        bool touching = false;

        if (a.radius > 0.0f && b.radius > 0.0f)
        {
            touching = IsHitSphereSphere(a, b);
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

        if (!entry.hasUpdated) continue;

        unsigned int color = DxLib::GetColor(80, 220, 255);

        if (entry.state.touchingA)
        {
            color = DxLib::GetColor(255, 220, 60);
        }

        DrawDebugSphere(entry.drawSphere, color);

        const wchar_t* textA = L"false";
        const wchar_t* textB = L"false";

        if (entry.state.touchingA) textA = L"true";
        if (entry.state.touchingB) textB = L"true";

        DxPlus::Debug::SetFormatString(
            L"ÚG %s: %s / %s: %s",
            entry.nameA, textA,
            entry.nameB, textB);
    }
}

void Collision::DrawDebugSphere(
    const Sphere& sphere,
    unsigned int color)
{
    if (sphere.radius <= 0.0f) return;

    constexpr int meridians = 6;
    constexpr int arcSegments = 16;
    constexpr int equatorSegments = 32;
    constexpr float pi = 3.14159265358979323846f;

    // 6–{‚Ìcü‚ğ•`‚­
    for (int line = 0; line < meridians; ++line)
    {
        const float longitude = 2.0f * pi * line / meridians;

        Vec3 previous = GetDebugSpherePoint(
            sphere, 0.0f, longitude);

        for (int segment = 1; segment <= arcSegments; ++segment)
        {
            const Vec3 next = GetDebugSpherePoint(
                sphere,
                pi * segment / arcSegments,
                longitude);

            DxLib::DrawLine3D(
                DxConv::ToVECTOR(previous),
                DxConv::ToVECTOR(next),
                color);

            previous = next;
        }
    }

    // Ô“¹•”•ª‚ğ•`‚­
    Vec3 previous = GetDebugSpherePoint(
        sphere, pi * 0.5f, 0.0f);

    for (int segment = 1; segment <= equatorSegments; ++segment)
    {
        const Vec3 next = GetDebugSpherePoint(
            sphere,
            pi * 0.5f,
            2.0f * pi * segment / equatorSegments);

        DxLib::DrawLine3D(
            DxConv::ToVECTOR(previous),
            DxConv::ToVECTOR(next),
            color);

        previous = next;
    }
}

#endif // _DEBUG

// =====================================
// ƒ‚ƒfƒ‹”¼“§–¾‰»
// =====================================

void Collision::ResetTransparentModels()
{
    for (TransparentModelData& data : transparentModels)
    {
        if (data.model != nullptr)
        {
            data.model->SetAlpha(data.originalAlpha);
        }
    }
    transparentModels.clear();
}   

void Collision::SetModelsBetweenTransparent(
    const Vec3& start,
    const Vec3& end,
    const ModelCollision& collision,
    const float alpha
)
{
    // ¡‰ñRay‚É“–‚½‚Á‚½ƒ‚ƒfƒ‹‚ğæ“¾
    auto hitModels = collision.GetModelsHitByRay(start, end);

    // V‚µ‚­‘ÎÛ‚É‚È‚Á‚½ƒ‚ƒfƒ‹‚ğ”¼“§–¾‚É‚·‚é
    for (ModelObject* model : hitModels)
    {
        if (model == nullptr) continue;

        const auto it = std::find_if(
            transparentModels.begin(),
            transparentModels.end(),
            [model](const TransparentModelData& data)
            {
                return data.model == model;
            });

        if (it != transparentModels.end()) continue;

        transparentModels.push_back({
            model,
            model->GetAlpha()
            });

        model->SetAlpha(alpha);
    }
}