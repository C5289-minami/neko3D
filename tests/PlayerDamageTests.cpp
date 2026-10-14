#include "Player.h"
#include "Collision.h"

#include <iostream>
#include <stdexcept>

bool g_raise_imgui_viewports = false;

namespace
{
    void Check(bool condition, const char* message)
    {
        if (!condition) throw std::runtime_error(message);
    }
}

int main()
{
    try
    {
        Player player;
        player.Reset();
        player.SetGravityEnabled(false);
        Check(player.GetHealth() == Player::MaxHealth, "Wrong initial HP.");

        // モデルの色変更と同じ楕円体判定で、接触と非接触を確認する。
        const Collision::Ellipsoid body{ {}, { 10.0f, 20.0f, 10.0f } };
        const Collision::Ellipsoid farAway{ { 100.0f, 0.0f, 0.0f }, body.radii };
        Check(!Collision::IsHitEllipsoidEllipsoid(body, farAway), "Separated bodies touched.");
        Check(Collision::IsHitEllipsoidEllipsoid(body, body), "Overlapping bodies did not touch.");

        player.SetMoveDirection({ 1.0f, 0.0f, 0.0f });
        const Vec3 hitPosition = player.GetPosition();
        Check(player.TakeDamage(), "Contact damage was rejected.");
        Check(player.GetHealth() == Player::MaxHealth - 1, "Contact did not subtract exactly one HP.");
        Check(player.IsDamaging(), "Contact did not enter Damaging.");

        // 接触が続いていても、被弾中の毎フレームの呼び出しではHPを減らさない。
        Check(!player.TakeDamage(), "Damaging accepted another hit.");
        player.Update(Player::DamageRecoveryTime * 0.5f);
        Check(player.IsDamaging(), "Damaging ended too early.");
        Check(!player.TakeDamage(), "Damaging accepted contact before recovery.");
        Check(player.GetHealth() == Player::MaxHealth - 1, "HP changed during recovery.");
        Check(player.GetPosition().x == hitPosition.x && player.GetPosition().z == hitPosition.z,
            "Horizontal movement continued while damaging.");
        player.Update(Player::DamageRecoveryTime * 0.5f);
        Check(player.GetCurrentState() == PlayerStateType::Idle, "Player did not recover to Idle.");
        player.Update(Player::DamageRecoveryTime);
        Check(player.GetHealth() == Player::MaxHealth - 1, "HP changed without contact damage.");

        // ボスの攻撃状態に関係なく、復帰後の接触では再び1だけ減る。
        Check(player.TakeDamage(), "Contact after recovery was rejected.");
        Check(player.GetHealth() == Player::MaxHealth - 2, "Repeated contact did not subtract one HP.");

        Player fallingPlayer;
        fallingPlayer.Reset();
        fallingPlayer.SetPosition({ 0.0f, 1000.0f, 0.0f });
        Check(fallingPlayer.TakeDamage(), "Airborne player could not take damage.");
        fallingPlayer.Update(Player::DamageRecoveryTime);
        Check(fallingPlayer.GetCurrentState() == PlayerStateType::NeutralJump,
            "Airborne recovery did not return to a landing-aware state.");
        Check(fallingPlayer.GetPosition().y < 1000.0f, "Damage stopped gravity.");

        Player dyingPlayer;
        dyingPlayer.Reset();
        dyingPlayer.SetGravityEnabled(false);
        for (int expected = Player::MaxHealth - 1; expected >= 0; --expected)
        {
            Check(dyingPlayer.TakeDamage(), "A valid hit was rejected.");
            Check(dyingPlayer.GetHealth() == expected, "Wrong HP after hit.");
            dyingPlayer.Update(Player::DamageRecoveryTime);
        }
        Check(dyingPlayer.GetCurrentState() == PlayerStateType::Die, "Zero HP did not enter Die.");
        Check(!dyingPlayer.TakeDamage() && dyingPlayer.GetHealth() == 0, "HP dropped below zero.");
        dyingPlayer.SetMoveDirection({ 1.0f, 0.0f, 0.0f });
        const Vec3 deathPosition = dyingPlayer.GetPosition();
        dyingPlayer.Update(1.0f);
        Check(dyingPlayer.GetCurrentState() == PlayerStateType::Die, "Dead player left Die.");
        Check(dyingPlayer.GetPosition().x == deathPosition.x && dyingPlayer.GetPosition().z == deathPosition.z,
            "Dead player could move.");
        dyingPlayer.Reset();
        Check(dyingPlayer.GetHealth() == Player::MaxHealth &&
            dyingPlayer.GetCurrentState() == PlayerStateType::Idle, "Reset did not revive player.");
        Check(dyingPlayer.TakeDamage(), "Reset left player invulnerable.");

        std::cout << "PASS: contact damage, recovery immunity, repeated contact, gravity, death, reset.\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
