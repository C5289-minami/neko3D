// =============================
// Gameplay/Actors/Enemy.cpp
// =============================
#include "Enemy.h"
#include "DxLib.h"
#include "ResourceManager.h"
#include "ResourceKeys.h"
#include "DxConv.h"

float Enemy::GetHpRate() const
{
    if (maxHp <= 0) return 0.0f;
    float rate = static_cast<float>(hp) / static_cast<float>(maxHp);
    return std::clamp(rate, 0.0f, 1.0f);
}

void Enemy::Init()
{
    int baseModelHandle = RM().GetModel(ResourceKeys::Model_Slime);
    if (baseModelHandle < 0) return;

    modelHandle = DxLib::MV1DuplicateModel(baseModelHandle);
    if (modelHandle < 0) return;

    DxLib::MV1SetScale(modelHandle, DxConv::ToVECTOR(scale));
}

void Enemy::Reset(const Vec3& startPosition, float startYaw)
{
    position = startPosition;
    yaw = startYaw;

    if (modelHandle < 0) return;

    animTime = 0.0f;
    animTotalTime = 0.0f;
    currentAnim = -1;

    hp = maxHp;
    isAlive = true;

    knockbackVelocity = {};
}

void Enemy::Update(float deltaTime, const Vec3& playerPos)
{
    if (!isAlive) return;

    if (modelHandle < 0) return;

    bool isMove = false;

    if (knockbackVelocity.LengthSq() > 0.00001f)
    {
        position += knockbackVelocity * deltaTime;

        float speed = knockbackVelocity.Length();
        float nextSpeed = speed - knockbackFriction * deltaTime;

        if (nextSpeed <= knockbackStopSpeed)
        {
            knockbackVelocity = {};
        }
        else
        {
            knockbackVelocity *= (nextSpeed / speed);
        }

        isMove = true;
    }
    else
    {
        Vec3 toPlayer = playerPos - position;
        toPlayer.y = 0.0f;
        float distanceSq = toPlayer.LengthSq();

        if (distanceSq > stopDistance * stopDistance)
        {
            Vec3 moveDir = toPlayer.Normalized();
            position += moveDir * moveSpeed * deltaTime;

            yaw = std::atan2(moveDir.x, moveDir.z);

            isMove = true;
        }
    }

    if (isMove)
    {
        StartAnimation(ANIM_MOVE);
    }
    else
    {
        StartAnimation(ANIM_IDLE);
    }

    if (animAttachIndex < 0) return;
    if (animTotalTime <= 0.0f) return;

    animTime += animSpeed * deltaTime;
    if (animTime >= animTotalTime)
    {
        animTime = 0.0f;
    }
    DxLib::MV1SetAttachAnimTime(modelHandle, animAttachIndex, animTime);
}

void Enemy::Draw() const
{
    if (!isAlive) return;

    if (modelHandle < 0) return;

    DxLib::MV1SetPosition(modelHandle, DxConv::ToVECTOR(position));
    DxLib::MV1SetRotationXYZ(modelHandle, DxConv::ToVECTOR({ 0.0f, yaw, 0.0f }));
    DxLib::MV1SetScale(modelHandle, DxConv::ToVECTOR(scale));

    DxLib::MV1DrawModel(modelHandle);
}

void Enemy::DrawDebug() const
{
    if (!isAlive) return;

    const int division = 24;
    const unsigned int color = DxLib::GetColor(255, 255, 0);

    const float bottomY = position.y;
    const float topY = position.y + height;

    for (int i = 0; i < division; ++i)
    {
        float angle0 = DxPlus::PI * 2.0f * i / division;
        float angle1 = DxPlus::PI * 2.0f * (i + 1) / division;

        Vec3 bottom0{
            position.x + std::sin(angle0) * radius, 
            bottomY, 
            position.z + std::cos(angle0) * radius
        };

        Vec3 bottom1{
            position.x + std::sin(angle1) * radius,
            bottomY,
            position.z + std::cos(angle1) * radius
        };

        Vec3 top0{
            bottom0.x,
            topY,
            bottom0.z
        };

        Vec3 top1{
            bottom1.x,
            topY,
            bottom1.z
        };

        // ‰º‚Ì‰~
        DxLib::DrawLine3D(
            DxConv::ToVECTOR(bottom0),
            DxConv::ToVECTOR(bottom1),
            color
        );

        // ã‚Ì‰~
        DxLib::DrawLine3D(
            DxConv::ToVECTOR(top0),
            DxConv::ToVECTOR(top1),
            color
        );
        // cü
        if (i % 6 == 0)
        {
            DxLib::DrawLine3D(
                DxConv::ToVECTOR(bottom0),
                DxConv::ToVECTOR(top0),
                color
            );
        }
    }

    DxLib::DrawSphere3D(
        DxConv::ToVECTOR(GetHitCenter()),
        GetRadius(),
        16,
        DxLib::GetColor(255, 0, 0),
        DxLib::GetColor(255, 0, 0),
        false
    );
}

void Enemy::Release()
{
    if (modelHandle >= 0)
    {
        DxLib::MV1DeleteModel(modelHandle);
        modelHandle = -1;
    }
}

void Enemy::TakeDamage(int damage)
{
    if (!isAlive) return;

    hp -= damage;
    if (hp <= 0)
    {
        hp = 0;
        Kill();
    }
}

void Enemy::Kill()
{
    isAlive = false;
}

void Enemy::ApplyKnockback(const Vec3& direction, float speed)
{
    if (!isAlive) return;
    Vec3 knockbackDir = direction;
    knockbackDir.y = 0.0f;

    if (knockbackDir.LengthSq() >= 0.00001f)
    {
        knockbackDir = knockbackDir.Normalized();
    }
    else
    {
        knockbackDir = { 0.0f, 0.0f, 1.0f };
    }

    knockbackVelocity = knockbackDir * speed;
}

void Enemy::StartAnimation(int nextAnim)
{
    if (modelHandle < 0) return;
    if (currentAnim == nextAnim) return;

    if (animAttachIndex >= 0)
    {
        DxLib::MV1DetachAnim(modelHandle, animAttachIndex);
        animAttachIndex = -1;
    }

    currentAnim = nextAnim;
    animTime = 0.0f;

    animAttachIndex = DxLib::MV1AttachAnim(modelHandle, currentAnim);
    if (animAttachIndex < 0) return;

    animTotalTime = DxLib::MV1GetAttachAnimTotalTime(modelHandle, animAttachIndex);
    DxLib::MV1SetAttachAnimTime(modelHandle, animAttachIndex, animTime);
}

