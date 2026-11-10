#pragma once

#include "ActorBase.h"
#include "AnimationHandle.h"
#include "CameraShakeType.h"
#include "CharSettings.h"
#include "Consts.h"
#include "ModelAnimation.h"
#include "ModelCollision.h"
#include "ObjectTransform.h"
#include "PlayerAnimType.h"
#include "PlayerInput.h"
#include "PlayerStateMachine.h"
#include "Vector3.h"
#include "DrawableObject.h"

#include <optional>


class Player : public ActorBase
{
public:
    // ========================================================
    // Types
    // ========================================================

    enum class GroundType
    {
        None,
        DownFloor,
        UpFloor
    };


    // ========================================================
    // Constants
    // ========================================================

    static constexpr int MaxHealth =
        Const::PLAYER_MAX_HEALTH;

    static constexpr float DamageRecoveryTime =
        Const::PLAYER_DAMAGE_RECOVERY_SECONDS;

    // 被弾後の移動速度低下設定
    static constexpr float DamageSlowRate = 0.5f;
    static constexpr float DamageSlowTime = 1.0f;

    static constexpr float CoyoteTime = 0.1f;

    static constexpr float AvoidInvTime =
        Const::PLAYER_AVOID_INVINCIBILITY_SECONDS;

    static constexpr float UltimateGaugeMax =
        Const::PLAYER_ULTIMATE_GAUGE_MAX;


    // ========================================================
    // Lifecycle
    // ========================================================

    Player();

    void Init() override;
    void Reset() override;
    void Update(float deltaTime) override;
    void Draw() const override;

    // シーンが登録したモデルとの当たり判定込みの更新
    void Update(
        float deltaTime,
        ModelCollision& collision,
        const Vec3& movementCenter = Vec3{}
    );


    // ========================================================
    // Health
    // ========================================================

    int GetHealth() const
    {
        return health_;
    }

    void SetHealth(int health)
    {
        health_ = health;
    }

    bool IsDead() const
    {
        return health_ <= 0;
    }

    bool IsDamaging() const
    {
        return GetCurrentState() == PlayerStateType::Damaging;
    }

    bool TakeDamage(int damage = 1);
    void AddDamage();
    void ApplyAvoid();

    float GetAvoidInvTime() const
    {
        return avoidInvTime_;
    }

    void DrawStatus() const;


    // ========================================================
    // State
    // ========================================================

    PlayerStateType GetCurrentState() const
    {
        return stateMachine_.GetCurrentType();
    }

    PlayerStateMachine& GetStateMachine()
    {
        return stateMachine_;
    }

    const PlayerStateMachine& GetStateMachine() const
    {
        return stateMachine_;
    }


    // ========================================================
    // Collision
    // ========================================================

    Collision::Sphere GetHitSphere() const
    {
        return {
            GetWorldPosition() +
                Vec3::Up() * (Const::PLAYER_HEIGHT * 0.5f),
            Const::PLAYER_RADIUS
        };
    }

    bool IsTouchingWall() const
    {
        return touchingWall_;
    }

    const Vec3& GetWallNormal() const
    {
        return wallNormal_;
    }

    float GetWallTopY() const
    {
        return wallTopY_;
    }

    bool IsNearWallTop() const
    {
        const float distance =
            wallTopY_ - GetPosition().y;

        return touchingWall_ &&
            distance >= -Const::PLAYER_SKIN &&
            distance <= Const::PLAYER_CLIMB_TOP_HOLD_DISTANCE;
    }


    // ========================================================
    // Movement
    // ========================================================

    void SetMoveDirection(const Vec3& moveDir);
    void StopMove();

    void SetMoveRate(float rate)
    {
        moveRate_ = rate;
    }

    void SetVerticalSpeed(float speed)
    {
        velocity_.y = speed;
    }

    Vec3& GetVelocity()
    {
        return velocity_;
    }

    Vec3 GetWorldMoveDirection(const Vec3& moveDir) const;

    void SetDefMove(bool defMove)
    {
        isdefMove_ = defMove;
    }

    void SetControlLocked(bool locked)
    {
        controlLocked_ = locked;
    }

#ifdef _DEBUG
    void SetDebugDashEnabled(bool enabled)
    {
        debugDashEnabled_ = enabled;
    }
#endif


    // ========================================================
    // Rotation
    // ========================================================

    void Turn(
        float direction,
        float speed,
        float deltaTime
    );

    void TurnTowards(
        const Vec3& moveDir,
        float deltaTime
    );


    // ========================================================
    // Ground and floor
    // ========================================================

    bool IsGrounded() const
    {
        return isGrounded_;
    }

    void SetGround(bool grounded)
    {
        isGrounded_ = grounded;

        // 外部から空中状態へ変更された場合、
        // 現在の床だけを解除する。
        if (!grounded)
        {
            currentFloor_ = GroundType::None;
        }
    }

    // 現在位置。
    // None      : 空中
    // DownFloor : 1階
    // UpFloor   : 2階
    GroundType GetCurrentFloor() const
    {
        return currentFloor_;
    }

    // 最後に接地していた階。
    // 空中になっても保持する。
    GroundType GetLastGroundFloor() const
    {
        return lastGroundFloor_;
    }

    // ジャンプを開始した階。
    // 着地するまで保持する。
    GroundType GetJumpStartFloor() const
    {
        return jumpStartFloor_;
    }

    bool IsAirborne() const
    {
        return currentFloor_ == GroundType::None;
    }

    bool IsFirstFloor() const
    {
        return currentFloor_ == GroundType::DownFloor;
    }

    bool IsSecondFloor() const
    {
        return currentFloor_ == GroundType::UpFloor;
    }

    bool WasLastOnFirstFloor() const
    {
        return lastGroundFloor_ == GroundType::DownFloor;
    }

    bool WasLastOnSecondFloor() const
    {
        return lastGroundFloor_ == GroundType::UpFloor;
    }


    // ========================================================
    // Gravity and jump
    // ========================================================

    bool IsGravityEnabled() const
    {
        return gravityEnabled_;
    }

    void SetGravityEnabled(bool enabled);

    bool CanGroundJump() const
    {
        return gravityEnabled_ &&
            (isGrounded_ || coyoteTimeRemaining_ > 0.0f);
    }

    bool CanDoubleJump() const
    {
        return gravityEnabled_ &&
            !isGrounded_ &&
            !doubleJumpUsed_;
    }

    bool TryJump();
    bool JumpAction();

    void StartDoubleJump();

    float GetDoubleJumpSpeed() const
    {
        return doubleJumpSpeed_;
    }

    void SetDoubleJumpSpeed(float value);
    void ResetDoubleJumpSpeed();

    void StartWallJump();

    float GetWallJumpMoveSpeed() const
    {
        return wallJumpMoveSpeed_;
    }

    float GetWallJumpSpeed() const
    {
        return wallJumpSpeed_;
    }

    void SetWallJumpMoveSpeed(float value);
    void SetWallJumpSpeed(float value);
    void ResetWallJumpSpeeds();


    // ========================================================
    // Climbing
    // ========================================================

    float GetClimbingSpeed() const
    {
        return climbingSpeed_;
    }

    float GetWallSlideSpeed() const
    {
        return wallSlideSpeed_;
    }

    void SetClimbingSpeed(float value);
    void SetWallSlideSpeed(float value);
    void ResetClimbingSpeeds();

    bool IsPillarElevatorActive() const
    {
        return elevatorPillar_ != nullptr;
    }

    void StartPillarElevator(
        const ModelObject* pillar,
        const Vec3& center,
        float topY
    );

    bool UpdatePillarElevator(float deltaTime);

    void EndPillarElevator();
    void StartPillarExitJump();
    void ForceFallFromPillar();

    Vec3 GetPillarElevatorCenter() const
    {
        return elevatorCenter_;
    }


    // ========================================================
    // Knockback
    // ========================================================

    void ApplyKnockback(
        const Vec3& direction,
        float power,
        float duration
    );


    // ========================================================
    // Position and transform
    // ========================================================

    void SetPosition(const Vec3& position)
    {
        ActorBase::SetPosition(position);
    }

    void SetStartPosition(const Vec3& position);
    void ResetPosition();

    // 記録済みの安全な1階の位置へ戻す。
    void ReturnToLastSafePosition(
        const ModelCollision& collision
    );

    ObjectTransform GetTransform() const
    {
        return {
            model_.GetScale(),
            GetPosition(),
            GetRotation()
        };
    }

    void SetTransform(const ObjectTransform& transform)
    {
        SetPosition(transform.position);
        SetPitch(transform.rotation.x);
        SetYaw(transform.rotation.y);
        model_.SetScale(transform.scale);
    }


    // ========================================================
    // Model
    // ========================================================

    ModelObject& GetModelObject()
    {
        return model_;
    }

    const ModelObject& GetModelObject() const
    {
        return model_;
    }

    void SetModelKey(const std::wstring& key);


    // ========================================================
    // Input
    // ========================================================

    const PlayerInput& GetInput() const
    {
        return input_;
    }


    // ========================================================
    // Attack
    // ========================================================

    // 攻撃球の出現位置。
    Vec3 GetAttackBallSpawnPosition() const;
    void PlayAttackEffect() const;


    // ========================================================
    // Ultimate
    // ========================================================

    float GetUltimateGauge() const
    {
        return ultimateGauge_;
    }

    void SetUltimateGauge(float value);
    void AddUltimateGauge(float amount);

    bool CanUseUltimate() const
    {
#ifdef _DEBUG
        if (input_.debugUltimatePressed)
        {
            return true;
        }
#endif

        return ultimateGauge_ >= UltimateGaugeMax;
    }

    bool TryUseUltimateGauge();

    void RequestUltimateShot();

    std::optional<Vec3> TakeUltimateShotOrigin();


    // ========================================================
    // Character settings
    // ========================================================

    CharData& GetCharData()
    {
        return charData;
    }

    const CharData& GetCharData() const
    {
        return charData;
    }

    void SetCharData(const CharData& data)
    {
        charData = data;
    }


    // ========================================================
    // Animation
    // ========================================================

    void SetAnimation(
        PlayerAnimType::Type type,
        bool loop = true,
        bool playAfterBlend = true,
        float blendSeconds = 0.15f
    );

    void PlayAnimationFromModel(
        const wchar_t* animationModelKey,
        bool loop = true
    );

    AnimationHandle GetAnimationHandle(
        PlayerAnimType::Type type
    ) const;

    ModelAnimation& GetAnimation()
    {
        return animation_;
    }

    void UpdateAnimation(float deltaTime);


    // ========================================================
    // Camera shake
    // ========================================================

    void CameraShakeRequest(
        CameraShakeType type,
        float duration = 0.25f,
        float strength = 20.0f,
        float frequency = 20.0f
    );

    CameraShakeType GetCameraShakeRequest() const;

    float GetCameraShakeDuration() const
    {
        return cameraShakeDuration_;
    }

    float GetCameraShakeStrength() const
    {
        return cameraShakeStrength_;
    }

    float GetCameraShakeFrequency() const
    {
        return cameraShakeFrequency_;
    }

    void ResetCameraShakeRequest();


private:
    // ========================================================
    // Internal update
    // ========================================================

    void UpdateMovement(float deltaTime);

    void UpdateFallRecovery(
        const ModelCollision& collision,
        const ModelCollision::ModelOb& body,
        bool landedOnFirstFloor
    );

    void UpdateHorizontalVelocity(float deltaTime);
    void UpdateStageMoveVelocity(float moveSpeed, float deltaTime);

    void Move(float deltaTime);

    void RestorePosition(const Vec3& position);

    void fallAndResetPosition(
        float deltaTime,
        Vec3& playerPosition
    );

#ifdef _DEBUG
    void PlayDebugKedamaEffect() const;
#endif


    // ========================================================
    // Health
    // ========================================================

    int health_{ MaxHealth };
    int addDamage_{};

    float avoidInvTime_{ 0.0f };
    float damageSlowTime_{ 0.0f };


    // ========================================================
    // Model and state
    // ========================================================

    ModelObject model_;

    PlayerInput input_;
    PlayerStateMachine stateMachine_;


    // ========================================================
    // Character settings
    // ========================================================

    CharData charData;


    // ========================================================
    // Position and movement
    // ========================================================

    Vec3 startPosition_{};
    Vec3 lastGroundedPosition_{};
    Vec3 lastFirstFloorPosition_{};

    Vec3 movementCenter_{};
    Vec3 moveInput_{};
    Vec3 velocity_{ 0.0f, 0.0f, 0.0f };

    float moveRate_{ 1.0f };

    bool isdefMove_{ false };
    bool controlLocked_{ false };


    // ========================================================
    // Ground and floor
    // ========================================================

    bool isGrounded_{ false };
    bool gravityEnabled_{ true };

    // 現在接地している床。
    // 空中の場合はNone。
    GroundType currentFloor_{ GroundType::None };

    // 最後に接地していた床。
    // 空中になっても保持する。
    GroundType lastGroundFloor_{ GroundType::None };

    // ジャンプを開始した床。
    // 着地するまで保持する。
    GroundType jumpStartFloor_{ GroundType::None };

    float coyoteTimeRemaining_{ 0.0f };


    // ========================================================
    // Jump
    // ========================================================

    // 着地するまでtrueを保持する。
    bool doubleJumpUsed_{ false };

    float doubleJumpSpeed_{
        Const::PLAYER_DOUBLE_JUMP_SPEED
    };

    float wallJumpMoveSpeed_{
        Const::PLAYER_WALL_JUMP_MOVE_SPEED
    };

    float wallJumpSpeed_{
        Const::PLAYER_WALL_JUMP_SPEED
    };


    // ========================================================
    // Collision
    // ========================================================

    bool touchingModel_{ false };
    bool wasTouchingModel_{ false };

    bool touchingWall_{ false };

    Vec3 wallNormal_{};

    float wallTopY_{ 0.0f };


    // ========================================================
    // Climbing
    // ========================================================

    float climbingSpeed_{
        Const::PLAYER_ELEVATOR_SPEED
    };

    float wallSlideSpeed_{
        Const::PLAYER_WALL_SLIDE_SPEED
    };

    const ModelObject* elevatorPillar_{ nullptr };

    Vec3 elevatorCenter_{};

    float elevatorTopY_{};

    bool elevatorInputConsumed_{ false };


    // ========================================================
    // Knockback
    // ========================================================

    Vec3 knockbackVelocity_{};

    float knockbackTime_{ 0.0f };


    // ========================================================
    // Ultimate
    // ========================================================

    float ultimateGauge_{
        Const::PLAYER_ULTIMATE_GAUGE_INITIAL
    };

    std::optional<Vec3> ultimateShotOrigin_;


    // ========================================================
    // Animation
    // ========================================================

    ModelAnimation animation_;

    PlayerAnimType::Type currentAnimType_{
        PlayerAnimType::Idle
    };

    bool currentAnimLoop_{ true };

    AnimationHandle idleAnimHandle_;
    AnimationHandle walkAnimHandle_;
    AnimationHandle jumpAnimHandle_;
    AnimationHandle doubleJumpAnimHandle_;
    AnimationHandle atkAnimHandle_;
    AnimationHandle ultAnimHandle_;


    // ========================================================
    // Camera shake
    // ========================================================

    CameraShakeType cameraShakeRequest_{
        CameraShakeType::None
    };

    float cameraShakeDuration_{ 0.25f };
    float cameraShakeStrength_{ 20.0f };
    float cameraShakeFrequency_{ 20.0f };


#ifdef _DEBUG
    // ========================================================
    // Debug
    // ========================================================

    bool debugDashEnabled_{ false };
#endif
};