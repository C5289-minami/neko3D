#include "Player.h"
#include "ResourceManager.h"
#include "ResourceKeys.h"
#include "Consts.h"
#include "Raycast.h"
#include "DxLib.h"
#include "Stage.h"

#include <cmath>
#include "Sound3D.h"
#include <algorithm>
#include <vector>
#include <iostream>
#include "ModelObjectManager.h"


Player::Player()
	: stateMachine_(*this)
{
}


void Player::Init()
{
  model_.modelKey = ResourceKeys::Model_Paladin;
	const int modelHandle = RM().GetModel(model_.modelKey);
	if (modelHandle < 0) return;

	model_.scale = { 100.0f, 100.0f, 100.0f };

	// 編集対象に設定
    MObjs().Add(model_, *this);
	MObjs().AddActor("Player", *this);
 Reset();
}


void Player::Reset()
{
    health_ = MaxHealth;
    wasTouchingPillar_ = false;
    model_.collisionHighlighted = false;
    ultimateGauge_ = 0.0f;
  SetPosition({ 0.0f, 26.0f, -1066.0f });
	SetPitch(0.0f);
	SetYaw(0.0f);
	MObjs().LoadActorTransform(*this);
	model_.scale = { 100.0f, 100.0f, 100.0f };


	moveInput_ = {};
	velocity_ = { 0.0f, 0.0f, 0.0f };

	// 空中から開始し、ステージへのレイキャストで接地させる。
	isGrounded_ = false;
	animation_.Reset();
	SetAnimation(PlayerAnimType::Idle, true);

    stateMachine_.Initialize();
    // 更新を止めても描画できるように初期ポーズを用意する
    animation_.Play3D(RM().GetModel(model_.modelKey), currentAnimIndex_, currentAnimLoop_, Const::ANIM_FPS);
    animation_.Update(0.0f);

	model_.Load(); // 無理やり上書きしてます
}


void Player::Update(float deltaTime)
{
	input_.Update();

	stateMachine_.Tick(deltaTime);

	// 中心がどっちにあるか調べる。高さはここでは考えない
	PlayerDiagonalMove(deltaTime, position_, movementCenter_);
    // 横移動は PlayerDiagonalMove、上下移動は velocity_.y にまとめる。

	if (gravityEnabled_)
	{
		velocity_.y -= Const::GRAVITY * deltaTime;
	}
	position_.y += velocity_.y * deltaTime;
	isGrounded_ = false;

	const int modelHandle = RM().GetModel(model_.modelKey);
	animation_.Play3D(modelHandle, currentAnimIndex_, currentAnimLoop_, Const::ANIM_FPS);
	animation_.Update(deltaTime);
}

void Player::Update(float deltaTime, const Stage& stage, const Vec3& movementCenter)
{
	movementCenter_ = movementCenter;
	const float previousY = GetPosition().y;

	Update(deltaTime);

	// 落下中のみ床面に合わせ、このフレームの落下距離もレイの長さに含める。
	if (velocity_.y <= 0.0f)
	{
		Physics::RayHit hit{};
		constexpr float groundSnapDistance = 30.0f;
		const float fallDistance = previousY - position_.y;
		const float rayDistance =
			fallDistance + groundSnapDistance + Const::PLAYER_SKIN;

		std::vector<int> handles;
		stage.GetCollisionModelHandles(handles);

		if (Physics::RaycastDown(
			handles,
			Vec3(
				GetPosition().x,
				previousY + Const::PLAYER_SKIN,
				GetPosition().z
			),
			rayDistance,
			hit))
		{
			if (position_.y - hit.point.y <= groundSnapDistance)
			{
				position_.y = hit.point.y;
				velocity_.y = 0.0f;
				isGrounded_ = true;
			}
		}
	}

    // 足元から高さ・半径を使って、プレイヤーのカプセルを作る。
    const Vec3 capsuleStart = GetPosition() + Vec3::Up() * Const::PLAYER_RADIUS;
    const Vec3 capsuleEnd = GetPosition() + Vec3::Up() * (Const::PLAYER_HEIGHT - Const::PLAYER_RADIUS);
    const bool touchingPillar = stage.IsHitPillar(capsuleStart, capsuleEnd, Const::PLAYER_RADIUS);
    if (touchingPillar && !wasTouchingPillar_)
    {
        std::cout << "Hit: Player touched a pillar." << std::endl;
        OutputDebugStringA("Hit: Player touched a pillar.\n");
    }
    wasTouchingPillar_ = touchingPillar;

	Sound3D::SetListener(
		GetPosition(),
		Vec3(
			std::sin(GetYaw()),
			0.0f,
			std::cos(GetYaw())
		)
	);
}

void Player::Draw() const
{
   const ModelObject worldModel = model_.WithParentTransform(GetPosition(), GetRotation());
	worldModel.ApplyCollisionColor();
	animation_.Draw3D(worldModel.position, worldModel.scale, worldModel.rotation);
}


bool Player::TakeDamage()
{
    if (IsDead() || IsDamaging()) return false;

    --health_;
    stateMachine_.ChangeState(PlayerStateType::Damaging);
    return true;
}

void Player::DrawStatus() const
{
    const wchar_t* state = IsDamaging() ? L"Damaging" : IsDead() ? L"Die" : L"";
    DxLib::DrawFormatString(24, 150, DxLib::GetColor(255, 255, 255),
        L"HP: %d / %d  %s", health_, MaxHealth, state);
}

void Player::SetAnimation(PlayerAnimType::Type type, bool loop)
{
	currentAnimIndex_ = static_cast<int>(type);
	currentAnimLoop_ = loop;
}


void Player::SetUltimateGauge(float value)//Ultのマックスゲージを超えないようにする
{
    if (!std::isfinite(value)) return;
    ultimateGauge_ = std::clamp(value, 0.0f, UltimateGaugeMax);
}

void Player::AddUltimateGauge(float amount)//Ultゲージを増やす
{
    if (!std::isfinite(amount) || amount <= 0.0f) return;
    SetUltimateGauge(ultimateGauge_ + amount);
}

bool Player::TryUseUltimateGauge()
{
    if (!CanUseUltimate()) return false;
    ultimateGauge_ = 0.0f;
    return true;
}

void Player::SetMoveDirection(const Vec3& moveDir)
{
	// 押している方向を覚えておく。実際に動かすのはUpdate
	moveInput_ = { moveDir.x, 0.0f, moveDir.z };
}


void Player::StopMove()
{
	moveInput_ = {};
	velocity_.x = 0.0f;
	velocity_.z = 0.0f;
}


void Player::Turn(float direction, float speed, float deltaTime)
{
	SetYaw(
		GetYaw() +
		direction * speed * deltaTime
	);
}


// 入力方向から角度を求め、
// プレイヤーの向きを滑らかに変更する
void Player::TurnTowards(
	const Vec3& moveDir,
	float deltaTime)
{
	if (moveDir.x == 0.0f &&
		moveDir.z == 0.0f)
	{
		return;
	}

	if (deltaTime <= 0.0f)
	{
		return;
	}

	const float pi = DX_PI_F;

	const float targetAngle =
		std::atan2(moveDir.x, moveDir.z);

	float diff =
		targetAngle - GetYaw();

	// 近いほうに回る
	if (diff > pi)
	{
		diff -= pi * 2.0f;
	}

	if (diff < -pi)
	{
		diff += pi * 2.0f;
	}

	float speed =
		Const::PLAYER_TURN_RATE_90;

	const float angle =
		std::abs(diff);

	// 90度を超えたら、
	// 角度が大きいほど回転速度を上げる
	if (angle > pi * 0.5f)
	{
		const float t =
			(angle - pi * 0.5f) /
			(pi * 0.5f);

		speed +=
			(Const::PLAYER_TURN_RATE_180 - speed)
			* t;
	}

	float rate =
		speed * deltaTime;

	if (rate > 1.0f)
	{
		rate = 1.0f;
	}

	float yaw =
		GetYaw() + diff * rate;

	if (yaw > pi)
	{
		yaw -= pi * 2.0f;
	}

	if (yaw < -pi)
	{
		yaw += pi * 2.0f;
	}

	SetYaw(yaw);
}

void Player::PlayerDiagonalMove(float deltaTime, Vec3& playerPos, const Vec3& movementCenter)
{
	if (deltaTime <= 0.0f || moveInput_.LengthSq() == 0.0f)
	{
		return;
	}

	//センターを中心にして、プレイヤーの位置から前方向を求める
	Vec3 forward = movementCenter - playerPos;
	forward.y = 0.0f;
	const float radius = forward.Length();
	if (radius < 0.001f)
	{
		forward = Vec3(std::sin(GetYaw()), 0.0f, std::cos(GetYaw()));
	}
	else
	{
		forward /= radius;
	}

	const Vec3 input = moveInput_.Normalized();
	const Vec3 right{ forward.z, 0.0f, -forward.x };
    
	const float moveSpeed =
		GetCurrentState() == PlayerStateType::LungingJump?
		lungingMoveSpeed_: Const::PLAYER_MOVE_SPEED;
	if (GetCurrentState() == PlayerStateType::LungingJump)
	{
		input_.isMoving = false;
	}

    const float distance = moveSpeed * deltaTime;
	const float nextRadius = (std::max)(1.0f, radius - input.z * distance);
	const float angle = input.x * distance / (std::max)(1.0f, radius);
	const Vec3 outward = -forward * std::cos(angle) + right * std::sin(angle);

	playerPos.x = movementCenter.x + outward.x * nextRadius;
	playerPos.z = movementCenter.z + outward.z * nextRadius;

	TurnTowards(forward * input.z + right * input.x, deltaTime);
}

void Player::SetLungingMoveSpeed(float value)
{
    if (std::isfinite(value)) lungingMoveSpeed_ = (std::max)(0.0f, value);
}

void Player::SetLungingJumpSpeed(float value)
{
    if (std::isfinite(value)) lungingJumpSpeed_ = (std::max)(0.0f, value);
}

void Player::ResetLungingSpeeds()
{
    lungingMoveSpeed_ = Const::PLAYER_LUNGING_MOVE_SPEED;
    lungingJumpSpeed_ = Const::PLAYER_LUNGING_JUMP_SPEED;
}

void Player::StartLungingJump(const Vec3& moveDir)
{
    if (!gravityEnabled_ || !isGrounded_ || moveDir.LengthSq() == 0.0f)
    {
        return;
    }

    StopMove();
    // 横移動の入力を保存し、ジャンプでは上向きの速度だけ設定する。
    SetMoveDirection(moveDir);
    velocity_.y = lungingJumpSpeed_;
    isGrounded_ = false;
}

void Player::JumpAction()
{
   if (!gravityEnabled_ || !isGrounded_)
	{
		return;
	}

   velocity_.y =
		Const::PLAYER_JUMP_SPEED;

	isGrounded_ = false;
}


void Player::SetGravityEnabled(bool enabled)
{
	gravityEnabled_ = enabled;
	if (!gravityEnabled_)
	{
		velocity_.y = 0.0f;
	}
}
