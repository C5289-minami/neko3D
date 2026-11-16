#pragma once

#include "GameContext.h"
#include "DxConv.h"

#include <algorithm>

#ifndef NDEBUG
#include "imgui.h"
#endif

namespace PresentationCollision
{
    struct DisplaySettings
    {
        bool enabled{ false };
        bool attack{ true };
        bool hurt{ true };
        bool body{ true };
        bool contactMarkers{ true };
    };

    inline DisplaySettings& Settings()
    {
        static DisplaySettings settings;
        return settings;
    }

    struct ContactStatus
    {
        bool body{};
        bool tail{};
        bool tailActive{};
        bool playerAttack{};
    };

    inline ContactStatus& Status()
    {
        static ContactStatus status;
        return status;
    }

    // Nearest point on the analytical capsule surface, not a mesh contact point.
    inline Vec3 ApproximateContact(const Collision::Capsule& capsule,
        const Collision::Sphere& sphere)
    {
        const Vec3 axis = capsule.end - capsule.start;
        const float lengthSquared = axis.LengthSq();
        const float amount = lengthSquared > Const::EPS
            ? std::clamp(Vec3::Dot(sphere.center - capsule.start, axis) / lengthSquared, 0.0f, 1.0f)
            : 0.0f;
        const Vec3 nearestAxis = capsule.start + axis * amount;
        Vec3 direction = sphere.center - nearestAxis;
        direction = direction.LengthSq() > Const::EPS ? direction.Normalized() : Vec3{ 1, 0, 0 };
        return nearestAxis + direction * capsule.radius;
    }

    inline void DrawImGui()
    {
#ifndef NDEBUG
        auto& settings = Settings();
        ImGui::Separator();
        if (!ImGui::CollapsingHeader(u8"発表用の当たり判定", ImGuiTreeNodeFlags_DefaultOpen)) return;
        ImGui::Checkbox(u8"当たり判定を表示 (B)", &settings.enabled);
        ImGui::BeginDisabled(!settings.enabled);
        ImGui::Checkbox(u8"攻撃判定（猫・ボス）", &settings.attack);
        ImGui::Checkbox(u8"被弾判定（猫・ダウン中のボス）", &settings.hurt);
        ImGui::Checkbox(u8"ボス本体の接触カプセル", &settings.body);
        ImGui::Checkbox(u8"近似の接触位置を表示", &settings.contactMarkers);
        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.3f, 1.0f), u8"赤: 猫の攻撃球");
        ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.4f, 1.0f), u8"緑: 猫の被弾球");
        ImGui::TextColored(ImVec4(0.3f, 0.85f, 1.0f, 1.0f), u8"青: 本体・尻尾のカプセル / 黄色: 接触中");
        ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), u8"オレンジ: ダウン中のボスの被弾球");
        ImGui::TextWrapped(u8"ボスの手は左右で青/赤。振り下ろしは青:追従、黄:固定、オレンジ:命中判定中。");
        ImGui::TextWrapped(u8"白い点は球とカプセルから求めた近似位置です。メッシュ上の厳密な接触点ではありません。");
        if (settings.enabled)
        {
            const auto& status = Status();
            ImGui::Text(u8"猫とボス本体: %s", status.body ? u8"接触" : u8"非接触");
            ImGui::Text(u8"猫と尻尾: %s", !status.tailActive ? u8"判定なし" : status.tail ? u8"接触" : u8"非接触");
            ImGui::Text(u8"猫の攻撃球: %s", status.playerAttack ? u8"有効" : u8"なし");
        }
        ImGui::EndDisabled();
#endif
    }

    inline void Draw(GameContext& context)
    {
#ifdef _DEBUG
        auto& status = Status();
        status = {};
        const auto* player = context.GetPlayer();
        if (!player) return;
        auto& boss = context.GetBoss();
        const auto& stage = context.GetStage();
        const auto playerBody = player->GetHitSphere();
        const auto& body = stage.GetCollision().GetContactCapsule();
        const auto tail = boss.IsTailWhipHitActive()
            ? boss.GetTailWhipHitCapsule(stage.GetWorldPosition(), stage.GetArenaRadius(),
                stage.GetArenaSurfaceY() + 30.0f) : Collision::Capsule{};
        status.body = body.radius > 0.0f && Collision::IsHitCapsuleSphere(body, playerBody);
        status.tailActive = tail.radius > 0.0f;
        status.tail = status.tailActive && Collision::IsHitCapsuleSphere(tail, playerBody);
        status.playerAttack = player->GetStateMachine().IsAttackBallActive();

        const auto& settings = Settings();
        if (!settings.enabled) return;
        const unsigned int cyan = DxLib::GetColor(80, 220, 255);
        const unsigned int touching = DxLib::GetColor(255, 220, 60);
        const unsigned int white = DxLib::GetColor(255, 255, 255);
        DxLib::SetUseVertexShader(-1);
        DxLib::SetUsePixelShader(-1);
        DxLib::SetUseZBuffer3D(FALSE);
        DxLib::SetWriteZBuffer3D(FALSE);
        if (settings.attack)
        {
            if (status.playerAttack)
                Collision::DrawDebugSphere(player->GetStateMachine().GetAttackBallSphere(), DxLib::GetColor(255, 100, 80));
            boss.DrawClapAttackDebug();
            boss.DrawSlamAttackDebug();
            // The existing skill helpers restore the depth state themselves.
            DxLib::SetUseZBuffer3D(FALSE);
            DxLib::SetWriteZBuffer3D(FALSE);
            if (status.tailActive) Collision::DrawDebugCapsule(tail, status.tail ? touching : cyan);
        }
        if (settings.hurt)
        {
            Collision::DrawDebugSphere(playerBody, DxLib::GetColor(80, 255, 100));
            if (boss.GetCurrentState() == BossStateType::Down)
                Collision::DrawDebugSphere(stage.GetBossDownHurtSphere(), DxLib::GetColor(255, 160, 60));
        }
        if (settings.body && body.radius > 0.0f)
            Collision::DrawDebugCapsule(body, status.body ? touching : cyan);
        if (settings.contactMarkers)
        {
            const auto drawContact = [&](const Collision::Capsule& capsule)
            {
                const Vec3 position = ApproximateContact(capsule, playerBody);
                Collision::DrawDebugSphere({ position, 8.0f }, white);
                DxLib::DrawLine3D(DxConv::ToVECTOR(position), DxConv::ToVECTOR(playerBody.center), white);
            };
            if (status.body) drawContact(body);
            if (status.tail) drawContact(tail);
        }
        DxLib::SetUseZBuffer3D(TRUE);
        DxLib::SetWriteZBuffer3D(TRUE);
#else
        (void)context;
#endif
    }
}
