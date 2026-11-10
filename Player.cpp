#include "Player.h"
#include "ResourceManager.h"
#include "ResourceKeys.h"
#include "Consts.h"
#include "Raycast.h"
#include "DxLib.h"
// #include "Stage.h"
#include "SoundManager.h"
#include "ModelObjectManager.h"

#include <cmath>
#include "Sound3D.h"
#include "EffectManager.h"
#include <algorithm>
#include <iostream>
#include <utility>

#include <tchar.h>
#include <cstdio>
#include <Windows.h>

namespace
{
	constexpr float GroundSnapDistance = 30.0f;
	constexpr float WalkableNormalY = 0.7f;
}


Player::Player()
	: ActorBase("Player"), stateMachine_(*this)
{
	model_.SetOwner(*this);
	model_.SetModelKey(ResourceKeys::Model_Player.mike);

	model_.SetObjectId("player_model");
	model_.SetObjectName("Player");
}

Vec3 Player::GetAttackBallSpawnPosition() const
//攻撃処理とTestSceneの攻撃位置を合わせるための関数
{
	const Vec3 forward{ std::sin(GetYaw()), 0.0f, std::cos(GetYaw()) };
	return GetPosition()
		+ Vec3::Up() * (Const::PLAYER_HEIGHT * Const::PLAYER_ATTACK_BALL_HEIGHT_MULTIPLIER)
		+ forward * (Const::PLAYER_RADIUS + Const::PLAYER_ATTACK_BALL_FORWARD_OFFSET);
}

void Player::PlayAttackEffect() const
{
	const int effectHandle = RM().GetEffect(ResourceKeys::Effect_PlayerAttack);
	if (effectHandle < 0) return;

	const Vec3 spawnPosition = GetAttackBallSpawnPosition();
	EffectManager::PlayEffect(effectHandle, spawnPosition, GetYaw() + DX_PI_F);
}

void Player::SetAnimation(
	PlayerAnimType::Type type,
	bool loop,
	bool playAfterBlend,
	float blendSeconds
)
{
	//if (currentAnimType_ == type && currentAnimLoop_ == loop)
	//	return;

	currentAnimType_ = type;
	currentAnimLoop_ = loop;

	animation_.Play3D(
		RM().GetModel(model_.GetModelKey()),
		GetAnimationHandle(currentAnimType_),
		currentAnimLoop_,
		Const::ANIM_FPS,
		blendSeconds,
		playAfterBlend
	);
}

void Player::PlayAnimationFromModel(const wchar_t* animationModelKey, bool loop)
{
	const int animationModelHandle = RM().GetModel(animationModelKey);
	if (animationModelHandle < 0) return;

	animation_.Play3D(
		RM().GetModel(model_.GetModelKey()),
		AnimationHandle::FromIndex(animationModelHandle, 0),
		loop,
		Const::ANIM_FPS
	);
}

void Player::UpdateAnimation(float deltaTime)
{
	animation_.Update(deltaTime);
}
AnimationHandle Player::GetAnimationHandle(PlayerAnimType::Type type) const
{
	switch (type)
	{
	case PlayerAnimType::Idle:
		return idleAnimHandle_;
	case PlayerAnimType::Walk:
		return walkAnimHandle_;
	case PlayerAnimType::Jump:
		return jumpAnimHandle_;
	case PlayerAnimType::Double_Jump:
		return doubleJumpAnimHandle_;
	case PlayerAnimType::Attack:
		return atkAnimHandle_;
	case PlayerAnimType::Ult_Attack:
		return ultAnimHandle_;
	default:
		return idleAnimHandle_;
	}
}

void Player::Init()
{
	const int modelHandle = RM().GetModel(model_.GetModelKey());
	if (modelHandle < 0) return;

	model_.SetModelType(ModelType::Bone);

	// AnimationHandleの初期化
	const int idleModelHandle = RM().GetModel(ResourceKeys::Model_PlayerAnim_Idle);
	idleAnimHandle_ = AnimationHandle::FromIndex(idleModelHandle, 0);

	const int walkAnimHandle = RM().GetModel(ResourceKeys::Model_PlayerAnim_Walk);
	walkAnimHandle_ = AnimationHandle::FromIndex(walkAnimHandle, 0);
	const int jumpAnimHandle = RM().GetModel(ResourceKeys::Model_PlayerAnim_Jump);
	jumpAnimHandle_ = AnimationHandle::FromIndex(jumpAnimHandle, 0);
	const int doubleJumpAnimHandle = RM().GetModel(ResourceKeys::Model_PlayerAnim_Dodge);
	doubleJumpAnimHandle_ = AnimationHandle::FromIndex(doubleJumpAnimHandle, 0);

	const int atkAnimHandle = RM().GetModel(ResourceKeys::Model_PlayerAnim_Attack);
	atkAnimHandle_ = AnimationHandle::FromIndex(atkAnimHandle, 0);
	const int ultAnimHandle = RM().GetModel(ResourceKeys::Model_PlayerAnim_Ult);
	ultAnimHandle_ = AnimationHandle::FromIndex(ultAnimHandle, 0);

	Reset();
}


void Player::Reset()
{

	health_ = MaxHealth;
	addDamage_ = 0;
	controlLocked_ = false;
	avoidInvTime_ = 0.0f;
	// wasTouchingPillar_ = false;
	wasTouchingModel_ = false;

	touchingWall_ = false;
	wallNormal_ = {};
	wallTopY_ = 0.0f;
	EndPillarElevator();
	elevatorInputConsumed_ = false;
#ifdef _DEBUG
	debugDashEnabled_ = false;
#endif

	model_.collisionHighlighted = false;
	ultimateGauge_ = Const::PLAYER_ULTIMATE_GAUGE_INITIAL;
	ultimateShotOrigin_.reset();
	// 初期位置・回転・拡大率は既定値を設定した後にJSONから読み込む
	MObjs().LoadActorTransform(*this);
	model_.Load();
	startPosition_ = GetPosition();
	lastGroundedPosition_ = startPosition_;
	lastFirstFloorPosition_ = startPosition_;

	moveInput_ = {};
	velocity_ = { 0.0f, 0.0f, 0.0f };

	// ゲームをやり直したときも、ダブルジャンプを使える状態に戻す。
	doubleJumpUsed_ = false;
	coyoteTimeRemaining_ = 0.0f;
	lastGroundFloor_ = GroundType::None;
	jumpStartFloor_ = GroundType::None;

	currentFloor_ = GroundType::None;
	lastGroundFloor_ = GroundType::None;
	jumpStartFloor_ = GroundType::None;

	isGrounded_ = false;

	stateMachine_.Initialize();

	// 初期アニメーションの再生
	animation_.Reset();
	animation_.Play3D(
		RM().GetModel(model_.GetModelKey()),
		GetAnimationHandle(PlayerAnimType::Idle),
		true,
		Const::ANIM_FPS
	);
	animation_.Update(0.0f);
}

void Player::SetStartPosition(const Vec3& position)
{
	SetPosition(position);
	startPosition_ = position;
	lastGroundedPosition_ = position;
	lastFirstFloorPosition_ = position;
}

void Player::ResetPosition()
{
	RestorePosition(startPosition_);
}

void Player::RestorePosition(const Vec3& position)
{
	lastGroundFloor_ = GroundType::None;
	jumpStartFloor_ = GroundType::None;
	if (!IsDamaging())
		stateMachine_.ChangeState(PlayerStateType::Idle);
	EndPillarElevator();
	StopMove();
	position_ = position;
	velocity_ = {};
	knockbackVelocity_ = {};
	knockbackTime_ = 0.0f;
	currentFloor_ = GroundType::None;
	lastGroundFloor_ = GroundType::None;
	jumpStartFloor_ = GroundType::None;
	isGrounded_ = false;
	touchingWall_ = false;
	touchingModel_ = false;
	wasTouchingModel_ = false;
	wallNormal_ = {};
	wallTopY_ = 0.0f;
	doubleJumpUsed_ = false;
	coyoteTimeRemaining_ = 0.0f;
	elevatorInputConsumed_ = input_.jumpPressing;
}

#ifdef _DEBUG
void Player::PlayDebugKedamaEffect() const
{
	const int effectHandle = RM().GetEffect(ResourceKeys::Effect_Kedama);
	if (effectHandle < 0) return;

	const Vec3 spawnPosition =
		GetWorldPosition() + Vec3::Up() * (Const::PLAYER_HEIGHT * 0.5f);
	EffectManager::PlayEffect(effectHandle, spawnPosition, GetWorldRotation().y);
}
#endif

void Player::Update(float deltaTime)
{
	input_.Update();
	if (controlLocked_)
	{
		input_ = {};
		moveInput_ = {};
		velocity_ = {};
		knockbackVelocity_ = {};
		knockbackTime_ = 0.0f;
		return;
	}
#ifdef _DEBUG
	if (input_.shiftRKeyPressed)
	{
		PlayDebugKedamaEffect();
	}
	if (input_.RkeyPressed)
	{
		ResetPosition();
		return;
	}
#endif
	UpdateMovement(deltaTime);
}

void Player::SetModelKey(const std::wstring& key)
{
	if (key.empty() || key == model_.GetModelKey()) return;

	const int modelHandle = RM().GetModel(key);
	if (modelHandle < 0) return;

	model_.SetModelKey(key);
	model_.SetModelType(ModelType::Bone);

	animation_.Reset();

	currentAnimType_ = PlayerAnimType::Idle;
	currentAnimLoop_ = true;

	animation_.Play3D(
		modelHandle,
		GetAnimationHandle(currentAnimType_),
		currentAnimLoop_,
		Const::ANIM_FPS,
		0.0f
	);
}

void Player::UpdateMovement(float deltaTime)
{
	const float elapsed = (std::max)(0.0f, deltaTime);
	avoidInvTime_ = (std::max)(0.0f, avoidInvTime_ - elapsed);

	// 前のフレームで床に着いたら、2回目のジャンプを再び使えるようにする。
	// 空中で被弾してステートが変わっても、このフラグはリセットしない。
	if (isGrounded_)
	{
		doubleJumpUsed_ = false;
		coyoteTimeRemaining_ = gravityEnabled_ ? CoyoteTime : 0.0f;
		//コヨーテタイムの残り時間をリセットする。重力が無効な場合は0にする。
	}
	else
	{
		coyoteTimeRemaining_ = (std::max)(0.0f,//コヨーテタイムの残り時間を減らす。0未満にならないようにする。
			coyoteTimeRemaining_ - elapsed);
	}

	// 歩いて足場を離れた場合も、通常ジャンプ中と同じ空中入力を受け付ける。
	// Jumpへの遷移だけでは上向きの速度を加えない。
	auto state = GetCurrentState();

	if (gravityEnabled_ && !isGrounded_ &&
		(state == PlayerStateType::Idle || state == PlayerStateType::Walk ||
			state == PlayerStateType::Grooming))
	{
		stateMachine_.ChangeState(PlayerStateType::Jump);
	}



#ifndef NDEBUG
	if (!charData.name.empty())
	{
		DxPlus::Debug::SetFormatString(
			L"CharacterSetting: %ls", DxPlus::Text::Utf8ToUtf16(charData.name).c_str());
	}
#endif

	// 接触中の柱に沿って上下移動する Elevator は、ジャンプで離れるまで続く。
	const bool elevatorFrame = GetCurrentState() == PlayerStateType::Climbing;
	stateMachine_.Tick(deltaTime);

	// 到着してジャンプへ切り替わるフレームも、上下移動を二重に加えない。
	if (!elevatorFrame)
	{
		UpdateHorizontalVelocity(deltaTime);

		if (knockbackTime_ > 0.0f)
		{
			knockbackTime_ = (std::max)(0.0f, knockbackTime_ - elapsed);

			velocity_ = knockbackVelocity_;
		}
		else
		{
			knockbackVelocity_ = {};
		}

		if (gravityEnabled_ && GetCurrentState() != PlayerStateType::Climbing)
		{
			velocity_.y -= Const::GRAVITY * deltaTime;
		}
		Move(deltaTime);
		isGrounded_ = false;
	}
	fallAndResetPosition(deltaTime, position_);
}

// void Player::Update(float deltaTime, const Stage& stage, const Vec3& movementCenter)
void Player::Update(float deltaTime, ModelCollision& collision, const Vec3& movementCenter)
{
	// 体の形はプレイヤーが渡す。判定対象の形・配置は ModelCollision が管理する。
	const ModelCollision::ModelOb modelOb{ Const::PLAYER_RADIUS, Const::PLAYER_HEIGHT, Const::PLAYER_SKIN };
	collision.Refresh();
	movementCenter_ = movementCenter;
	const Vec3 oldPosition = GetPosition();
	const float previousY = oldPosition.y;

	input_.Update();
	if (controlLocked_)
	{
		input_ = {};
		moveInput_ = {};
	}
#ifdef _DEBUG
	if (input_.shiftRKeyPressed)
	{
		PlayDebugKedamaEffect();
	}
	if (input_.RkeyPressed)
	{
		ResetPosition();
		// 落ちていた場所から当たり判定を続けないように、ここで終わる。
		return;
	}
#endif
	// 今フレームの入力に合う側面を選び、柱のどの方向からでも登れるようにする。
	// touchingWall_ = stage.FindPillarWall(GetPosition(), wallNormal_, GetWorldMoveDirection(input_.moveDir),
		// &wallTopY_, GetCurrentState() == PlayerStateType::Climbing);
	Vec3 pillarCenter{};

	const ModelObject* pillarModel = nullptr;

	if (!input_.jumpPressing) elevatorInputConsumed_ = false;
	const bool canStartElevator = input_.jumpPressing && !elevatorInputConsumed_ &&
		!IsPillarElevatorActive() && gravityEnabled_ && !IsDead() && !IsDamaging();

	// 柱の情報は毎回受け取り、上昇を始めるかどうかは後のifで決める。
	touchingWall_ = collision.FindWall(//<-カプセルの処理
		GetPosition(), modelOb, wallNormal_,
		GetWorldMoveDirection(input_.moveDir),
		&wallTopY_,
		Const::PLAYER_CLIMB_DETECTION_MARGIN,
		0.0f,
		&pillarCenter,
		&pillarModel
	);
	if (IsPillarElevatorActive())
	{
		float topY{};
		if (collision.GetPillarBounds(elevatorPillar_, elevatorCenter_, topY))
			elevatorTopY_ = topY - Const::PLAYER_ELEVATOR_TOP_OFFSET;
		else if (GetPosition().y > elevatorTopY_)
		{
			EndPillarElevator();
			input_.isMoving = false;
		}
	}
	if (canStartElevator && touchingWall_)
	{
		StartPillarElevator(pillarModel, pillarCenter, wallTopY_);
	}
	if (IsPillarElevatorActive()) input_.jumpPressed = false;
	UpdateMovement(deltaTime);

	// のぼり中は、壁の上端に吸着するように位置を補正する。
	if (!IsPillarElevatorActive())
	{
		position_ = collision.SlideAlongWalls(oldPosition, position_, modelOb, touchingModel_);
	}

	const float dy = position_.y - previousY;

	if (dy > 0.0f)
	{
		// 移動前の頭から移動後の頭まで調べ、天井のすり抜けを防ぐ。
		Physics::RayHit hit{};
		const float rayDistance = dy + Const::PLAYER_SKIN * 2.0f;
		if (collision.RaycastUp(
			Vec3(
				GetPosition().x,
				previousY + Const::PLAYER_HEIGHT - Const::PLAYER_SKIN,
				GetPosition().z
			),
			rayDistance,
			hit, false))
		{
			position_.y = hit.point.y - Const::PLAYER_HEIGHT - Const::PLAYER_SKIN;
			velocity_.y = 0.0f;
			isGrounded_ = false;
			EndPillarElevator();
		}
	}
	// ステージの内周・外周は、足元の点ではなく体の球で移動経路を調べる。
	const Vec3 bodyOffset = Vec3::Up() * (modelOb.height * 0.5f);
	const Collision::Sphere oldBody{ oldPosition + bodyOffset, modelOb.radius };
	position_ = collision.SlideAlongFallRecoveryFloor(oldBody, position_ - oldPosition, velocity_) - bodyOffset;

	bool landedOnFirstFloor = false;
	// 落下中のみ床面に合わせ、このフレームの落下距離もレイの長さに含める。
	// エレベーター中と、到着時の小ジャンプ中は床へ吸着させない。
	if (dy <= 0.0f && velocity_.y <= 0.0f && GetCurrentState() != PlayerStateType::Climbing)
	{
		Physics::RayHit hit{};
		const float rayStartY = (std::max)(previousY, position_.y);
		const float fallDistance = rayStartY - position_.y;
		const float rayDistance =
			fallDistance + GroundSnapDistance + Const::PLAYER_SKIN;

		const Vec3 origin{ position_.x, rayStartY + Const::PLAYER_SKIN, position_.z };

		bool foundGround = collision.RaycastDown(origin, rayDistance, hit, nullptr, false);

		Physics::RayHit pillarHit{};
		// 小ジャンプの頂点から急に吸い戻されないよう、柱は足元付近だけで着地する。
		const float pillarRayDistance = fallDistance + Const::PLAYER_SKIN * 2.0f;

		if (collision.RaycastPillarTop(origin, pillarRayDistance, Const::PLAYER_ELEVATOR_TOP_OFFSET, pillarHit) &&
			(!foundGround || pillarHit.distance < hit.distance))
		{
			hit = pillarHit;
			foundGround = true;
		}

		Physics::RayHit stageHit{};
		const Collision::Sphere body{ position_ + bodyOffset, modelOb.radius };
		if (collision.SweepFallRecoveryFloor(body, -Vec3::Up() * GroundSnapDistance, stageHit) &&
			(stageHit.normal.y >= WalkableNormalY || stageHit.distance <= modelOb.skin))
		{
			const float floorY = position_.y - stageHit.distance;
			if (!foundGround || floorY >= hit.point.y)
			{
				stageHit.point.y = floorY;
				hit = stageHit;
				foundGround = true;
				landedOnFirstFloor = true;
			}
		}

		if (foundGround)
		{
			if (position_.y - hit.point.y <= GroundSnapDistance)
			{
				// 坂の中へ落ち込まないよう、足元を床の高さへ戻す。
				position_.y = hit.point.y;

				if (hit.normal.y < WalkableNormalY)
				{
					// 急な坂では踏ん張れないので、坂の下へ流される。
					isGrounded_ = false;

					// ステージの坂では、移動時に重力を斜面に沿わせている。
					if (gravityEnabled_ && !landedOnFirstFloor)
					{
						const Vec3 slideDirection = Vec3{ hit.normal.x, 0.0f, hit.normal.z }.Normalized();
						const float slideAmount = Const::GRAVITY * deltaTime;
						const Vec3 slidePosition = position_ + slideDirection * slideAmount;

						// 坂の途中に壁があれば、そこを突き抜けずに止まる。
						bool touched = false;

						const Vec3 slid = collision.SlideAlongWalls(position_, slidePosition, modelOb, touched);
						const Collision::Sphere slideBody{ position_ + bodyOffset, modelOb.radius };
						position_ = collision.SlideAlongFallRecoveryFloor(slideBody, slid - position_, velocity_) - bodyOffset;
						touchingModel_ |= touched;
					}
				}
				else
				{
					// 立てる床なら、落下を止めて足をつける。
					velocity_.y = 0.0f;
					isGrounded_ = true;
				}
			}
		}
	}

	UpdateFallRecovery(collision, modelOb, landedOnFirstFloor);

	// 足元を基準にしたカプセルで、補正後の接触も確認する。
	// const Vec3 capsuleStart = GetPosition() + Vec3::Up() * Const::PLAYER_RADIUS;
	// const Vec3 capsuleEnd = GetPosition() + Vec3::Up() * (Const::PLAYER_HEIGHT - Const::PLAYER_RADIUS);
	// touchingPillar |= stage.IsHitPillar(capsuleStart, capsuleEnd, Const::PLAYER_RADIUS);

	touchingModel_ |= collision.IsTouchingWalls(GetPosition(), modelOb);

	// if (touchingPillar && !wasTouchingPillar_)
	if (touchingModel_ && !wasTouchingModel_)
	{
		// std::cout << "Hit: Player touched a pillar." << std::endl;
		std::cout << "Hit: Player touched a collision model." << std::endl;
	}
	// wasTouchingPillar_ = touchingPillar;
	wasTouchingModel_ = touchingModel_;

	Sound3D::SetListener(
		GetPosition(),
		Vec3(
			std::sin(GetYaw()),
			0.0f,
			std::cos(GetYaw())
		)
	);
}

void Player::UpdateFallRecovery(
	const ModelCollision& collision,
	const ModelCollision::ModelOb& body,
	bool landedOnFirstFloor)
{
	// 現在の床は毎フレーム判定し直す。
	// 空中ならNoneになる。
	currentFloor_ = GroundType::None;

	if (!gravityEnabled_ ||
		!collision.HasFallRecoveryFloor() ||
		IsDead())
	{
		return;
	}

	if (isGrounded_)
	{
		currentFloor_ = landedOnFirstFloor
			? GroundType::DownFloor
			: GroundType::UpFloor;

		// 最後に接地していた床を更新する。
		lastGroundFloor_ = currentFloor_;

		// 着地したので、ジャンプ開始床を解除する。
		jumpStartFloor_ = GroundType::None;

		const bool outsideBossBody =
			!collision.IsTouchingCapsule(GetHitSphere());

		const bool isFirstFloor =
			currentFloor_ == GroundType::DownFloor;

		const bool safeGround =
			outsideBossBody &&
			(!isFirstFloor ||
				collision.HasFallRecoverySupport(
					position_,
					body,
					WalkableNormalY
				));

		if (safeGround)
		{
			lastGroundedPosition_ = position_;

			if (isFirstFloor)
			{
				lastFirstFloorPosition_ = position_;
			}

			return;
		}
	}

	// 空中ではcurrentFloor_だけがNoneになる。
	// lastGroundFloor_とjumpStartFloor_は保持する。

	const float fallDistance =
		lastFirstFloorPosition_.y - position_.y;

	if (velocity_.y > 0.0f ||
		fallDistance < Const::PLAYER_FALL_RECOVERY_DISTANCE)
	{
		return;
	}

	ReturnToLastSafePosition(collision);
}
void Player::ReturnToLastSafePosition(const ModelCollision& collision)
{
	if (IsDead() || !collision.HasFallRecoveryFloor()) return;

	Vec3 recoveryPosition = lastGroundedPosition_;
	recoveryPosition.y = lastFirstFloorPosition_.y;

	// 同じ X・Z に1階の床がなければ、最後に立てた1階の位置へ戻す。
	if (!collision.CanLandOnFallRecoveryFloor(recoveryPosition, Const::PLAYER_SKIN, GroundSnapDistance, WalkableNormalY))
		recoveryPosition = lastFirstFloorPosition_;

	recoveryPosition.y += Const::PLAYER_FALL_RECOVERY_HEIGHT_OFFSET;
	SoundManager::PlaySE(RM().GetSound(ResourceKeys::Sound_Fall));
	RestorePosition(recoveryPosition);
}

void Player::Draw() const
{
	model_.Draw();
	stateMachine_.Draw();
}


bool Player::TakeDamage(int damage)
{
	if (damage <= 0 || IsDead() || IsDamaging() || avoidInvTime_ > 0.0f) return false;

	addDamage_ = damage;
	stateMachine_.ChangeState(PlayerStateType::Damaging);
	return true;
}

void Player::AddDamage()
{
	health_ = (std::max)(0, health_ - addDamage_);
	addDamage_ = 0;
}

void Player::ApplyAvoid()
{
	addDamage_ = 0;
	avoidInvTime_ = AvoidInvTime;
}

void Player::DrawStatus() const
{
	const wchar_t* state = IsDamaging() ? L"Damaging" : IsDead() ? L"Die" : L"";
	DxLib::DrawFormatString(24, 150, DxLib::GetColor(255, 255, 255),
		L"HP: %d / %d  %s", health_, MaxHealth, state);
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
	ultimateGauge_ = Const::PLAYER_ULTIMATE_GAUGE_AFTER_USE;
	return true;
}

void Player::RequestUltimateShot()
{
	ultimateShotOrigin_ = GetWorldPosition() + Vec3::Up() * (Const::PLAYER_HEIGHT * 0.5f);
}

std::optional<Vec3> Player::TakeUltimateShotOrigin()
{
	return std::exchange(ultimateShotOrigin_, std::nullopt);
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


Vec3 Player::GetWorldMoveDirection(const Vec3& moveDir) const//Plyaerがどこを向いているかのワールド座標系での移動方向を取得する
{
	Vec3 forward = movementCenter_ - GetPosition();
	forward.y = 0.0f;

	if (forward.LengthSq() < Const::EPS)
		forward = Vec3(std::sin(GetYaw()), 0.0f, std::cos(GetYaw()));
	else
		forward = forward.Normalized();
	const Vec3 right{ forward.z, 0.0f, -forward.x };

	return (forward * moveDir.z + right * moveDir.x).Normalized();
	//プレイヤーの移動方向をワールド座標系で取得するための関数です。
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

void Player::UpdateHorizontalVelocity(float deltaTime)
{
	// 壁キック中は、開始時に設定した横速度をそのまま使う。
	const auto state = GetCurrentState();
	if (state == PlayerStateType::WallJump) return;
	const bool isDoubleJump = state == PlayerStateType::DoubleJump;

	// 通常時は入力を離すと横速度を止める。ダブルジャンプ中は、
	// 入力を離したら直前の横速度を保ち、入力があれば方向を変える。
	if (!isDoubleJump)
	{
		velocity_.x = 0.0f;//入力を離したら横速度を止める
		velocity_.z = 0.0f;
	}
	if (deltaTime <= 0.0f) return;

	if (isDoubleJump && moveInput_.LengthSq() == 0.0f) return;

	// 移動速度
	float moveSpeed = Const::PLAYER_MOVE_SPEED * moveRate_;

    if (isGrounded_ && state == PlayerStateType::Walk)
    {
        moveSpeed *= charData.moveSpeed / CharData::DefaultMoveSpeed;
    }

#ifdef _DEBUG
    if (debugDashEnabled_)
    {
        moveSpeed = Const::PLAYER_DEBUG_DASH_SPEED;
    }
#endif

    if (isDoubleJump)
    {
        moveSpeed = Const::PLAYER_DOUBLE_JUMP_MOVE_SPEED;
    }

	if (isdefMove_)
	{
		// 移動と同じ、カメラ基準の入力方向を使う。
		float cameraRotY = DxLib::GetCameraAngleHRotate();
		//カメラが向いている方を「前」にする。
		Vec3 cameraForward{ std::sin(cameraRotY), 0.0f, std::cos(cameraRotY) };
		//前の向きを右に90度回して、カメラから見た「右」を作る。
		//だから、カメラの向きが変わると、前後左右の移動方向も変わる。
		Vec3 cameraRight{ cameraForward.z, 0.0f, -cameraForward.x };
		Vec3 moveInput = input_.moveDir;
		//キーに合わせて、前後左右カメラが動く向きが決まる
		Vec3 moveDirection = cameraRight * moveInput.x + cameraForward * moveInput.z;//
		Vec3 horizontalVelocity = moveDirection.Normalized() * moveSpeed;//入力
		velocity_.x = horizontalVelocity.x;
		velocity_.z = horizontalVelocity.z;
	}
	else
	{
		UpdateStageMoveVelocity(moveSpeed, deltaTime);
	}

}

void Player::Move(float deltaTime)
{
	if (deltaTime <= 0.0f) return;
	position_ += velocity_ * deltaTime;
	// 上下の速度は向きに含めず、横に進む方向へ回転する。
	TurnTowards(Vec3(velocity_.x, 0.0f, velocity_.z), deltaTime);
}

void Player::fallAndResetPosition(float deltaTime, Vec3& PlayerPos)
{
	position_ = PlayerPos;

	Vec3 oldPos = position_;

	Vec3 pos = model_.GetPosition();
	if (position_.y <= pos.y - 100.0f)
	{
		position_ = oldPos;
	}

}

void Player::UpdateStageMoveVelocity(float moveSpeed, float deltaTime)
{
	if (deltaTime <= 0.0f || moveInput_.LengthSq() == 0.0f) return;
	const float distance = moveSpeed * deltaTime;

	// センターを中心にして、プレイヤーの位置から前方向を求める。
	Vec3 forward = movementCenter_ - position_;
	forward.y = 0.0f;
	const float radius = forward.Length();
	if (radius < Const::EPS)
	{
		forward = Vec3(std::sin(GetYaw()), 0.0f, std::cos(GetYaw()));
	}
	else
	{
		forward /= radius;
	}

	const Vec3 input = moveInput_.Normalized();
	const Vec3 right{ forward.z, 0.0f, -forward.x };

	const float nextRadius = (std::max)(1.0f, radius - input.z * distance);
	const float angle = input.x * distance / (std::max)(1.0f, radius);
	const Vec3 outward = -forward * std::cos(angle) + right * std::sin(angle);

	// 円周移動した後のX・Z座標を求める。
	const float nextX = movementCenter_.x + outward.x * nextRadius;
	const float nextZ = movementCenter_.z + outward.z * nextRadius;

	// 次の座標までの距離を、このフレームの時間で割って横速度にする。
	// 座標への反映はMoveで行う。ジャンプ・重力のY速度はここでは触らない。
	velocity_.x = (nextX - position_.x) / deltaTime;
	velocity_.z = (nextZ - position_.z) / deltaTime;
}

void Player::SetDoubleJumpSpeed(float value)
{
	if (std::isfinite(value)) doubleJumpSpeed_ = (std::max)(0.0f, value);
}

void Player::ResetDoubleJumpSpeed()
{
	doubleJumpSpeed_ = Const::PLAYER_DOUBLE_JUMP_SPEED;
}

void Player::StartDoubleJump()
{
	if (!CanDoubleJump())
	{
		return;
	}

	// 通常ジャンプ開始時の床が記録されていない場合のみ補完する。
	if (jumpStartFloor_ == GroundType::None)
	{
		jumpStartFloor_ = lastGroundFloor_;
	}

	doubleJumpUsed_ = true;
	coyoteTimeRemaining_ = 0.0f;

	const float moveSpeed =
		Const::PLAYER_MOVE_SPEED * 3.0f;

	velocity_.x = std::sin(GetYaw()) * moveSpeed;
	velocity_.z = std::cos(GetYaw()) * moveSpeed;
	velocity_.y = doubleJumpSpeed_;

	isGrounded_ = false;
	currentFloor_ = GroundType::None;
}


void Player::SetClimbingSpeed(float value)
{
	if (std::isfinite(value)) climbingSpeed_ = (std::max)(0.0f, value);
}

void Player::SetWallSlideSpeed(float value)
{
	if (std::isfinite(value)) wallSlideSpeed_ = (std::max)(0.0f, value);
}

void Player::ResetClimbingSpeeds()
{
	// climbingSpeed_ = Const::PLAYER_CLIMBING_SPEED;
	climbingSpeed_ = Const::PLAYER_ELEVATOR_SPEED;
	wallSlideSpeed_ = Const::PLAYER_WALL_SLIDE_SPEED;
}

void Player::ApplyKnockback(const Vec3& direction, float power, float duration)
{
	Vec3 outward = direction;
	//outward.y = 0.0f;

	if (outward.LengthSq() <= 0.000001f) return;
	jumpStartFloor_ = GroundType::None;

	outward = outward.Normalized();

	knockbackVelocity_ = outward * power;
	knockbackTime_ = duration;

	velocity_.y = 0.0f;
	isGrounded_ = false;
	EndPillarElevator();

	// 被ダメージ中は、回避の受付と保留中のダメージ処理を続ける。
	if (!IsDamaging())
		stateMachine_.ChangeState(PlayerStateType::Jump);

	SoundManager::PlaySE(RM().GetSound(ResourceKeys::Sound_Fall));
}

void Player::SetWallJumpMoveSpeed(float value)
{
	if (std::isfinite(value)) wallJumpMoveSpeed_ = (std::max)(0.0f, value);
}

void Player::SetWallJumpSpeed(float value)
{
	if (std::isfinite(value)) wallJumpSpeed_ = (std::max)(0.0f, value);
}

void Player::ResetWallJumpSpeeds()
{
	wallJumpMoveSpeed_ = Const::PLAYER_WALL_JUMP_MOVE_SPEED;
	wallJumpSpeed_ = Const::PLAYER_WALL_JUMP_SPEED;
}

void Player::StartWallJump()
{
	const Vec3 direction = Vec3(wallNormal_.x, 0.0f, wallNormal_.z).Normalized();
	if (!gravityEnabled_ || !touchingWall_ || direction.LengthSq() == 0.0f) return;

	StopMove();
	// WallJump専用の速度とジャンプ力で、接触中の壁から外向きに跳ぶ。
	velocity_.x = direction.x * wallJumpMoveSpeed_;
	velocity_.z = direction.z * wallJumpMoveSpeed_;
	velocity_.y = wallJumpSpeed_;
	isGrounded_ = false;
	coyoteTimeRemaining_ = 0.0f;
}

bool Player::TryJump()
{
	if (CanGroundJump())
	{

		stateMachine_.ChangeState(PlayerStateType::Jump);
		JumpAction();
		stateMachine_.NotifyEvent(PlayerActionEvent::Jumped);
		return true;
	}

	if (CanDoubleJump())
	{
		stateMachine_.ChangeState(PlayerStateType::DoubleJump);
		return true;
	}

	return false;
}

bool Player::JumpAction()
{
	if (!CanGroundJump())
	{
		return false;
	}

	jumpStartFloor_ =
		currentFloor_ != GroundType::None
		? currentFloor_
		: lastGroundFloor_;

	velocity_.y = Const::PLAYER_JUMP_SPEED;
	isGrounded_ = false;
	currentFloor_ = GroundType::None;
	coyoteTimeRemaining_ = 0.0f;

	return true;
}


void Player::SetGravityEnabled(bool enabled)
{
	gravityEnabled_ = enabled;
	coyoteTimeRemaining_ = 0.0f;
	if (!gravityEnabled_)
	{
		velocity_.y = 0.0f;
		EndPillarElevator();
	}
}

void Player::StartPillarElevator(const ModelObject* pillar, const Vec3& center, float topY)
{
	static const float PILLAR_RADIUS = 55.0f;

	float recordFirstPosY = topY - Const::PLAYER_ELEVATOR_TOP_OFFSET;
	if (!pillar || !gravityEnabled_ || IsDamaging() ||
		IsPillarElevatorActive()) return;

	// 上端の近くではジャンプを優先する。Spaceを離すまで吸い戻さない。
	float topJumpMargin = 20.0f;
	if (position_.y >= recordFirstPosY - topJumpMargin)
	{
		elevatorInputConsumed_ = true;
		return;
	}

	elevatorPillar_ = pillar;
	elevatorCenter_ = center;
	elevatorTopY_ = recordFirstPosY;
	elevatorInputConsumed_ = true;
	coyoteTimeRemaining_ = 0.0f;

	Vec3 dir = position_ - center;
	dir.y = 0.0f;

	if (dir.LengthSq() <= Const::EPS) return;

	dir = dir.Normalized();

	position_.x = center.x + dir.x * PILLAR_RADIUS;
	position_.y = position_.y + 80.0f; // 少し上に上げてから吸着する
	position_.z = center.z + dir.z * PILLAR_RADIUS;

	// 柱の中心を向く
	SetYaw(std::atan2(
		center.x - position_.x,
		center.z - position_.z
	));

	isGrounded_ = false;
	stateMachine_.ChangeState(PlayerStateType::Climbing);
}

bool Player::UpdatePillarElevator(float deltaTime)
{
	StopMove();
	velocity_.y = 0.0f;
	isGrounded_ = false;
	// 位置を変えないようにする
	//position_.x = elevatorCenter_.x;
	//position_.z = elevatorCenter_.z;

	// Animが来てないので仮で上に向ける
	SetRotation(Vec3(-DX_PI_F * 0.5f, GetYaw(), 0.0f));


	if (deltaTime <= 0.0f) return false;
	const float climbSpeed = climbingSpeed_ * charData.climbSpeedMultiplier;
	position_.y = (std::min)(elevatorTopY_, position_.y + climbSpeed * deltaTime);
	return position_.y >= elevatorTopY_;
}

void Player::EndPillarElevator()
{
	elevatorPillar_ = nullptr;

	pitch_ = 0.0f;
	SetRotation(Vec3(0.0f, GetYaw(), 0.0f));
}

void Player::StartPillarExitJump()
{
	jumpStartFloor_ = GroundType::None;
	velocity_.y = Const::PLAYER_ELEVATOR_EXIT_JUMP_SPEED;
	isGrounded_ = false;
	elevatorInputConsumed_ = true;
	coyoteTimeRemaining_ = 0.0f;
}

void Player::ForceFallFromPillar()
{
	if (!IsPillarElevatorActive()) return;
	jumpStartFloor_ = GroundType::None;

	EndPillarElevator();
	StopMove();

	velocity_.y = 0.0f;
	isGrounded_ = false;
	coyoteTimeRemaining_ = 0.0f;

	stateMachine_.ChangeState(PlayerStateType::Jump);
}

void Player::CameraShakeRequest(
	CameraShakeType type,
	float duration,
	float strength,
	float frequency)
{
	cameraShakeRequest_ = type;
	cameraShakeDuration_ = duration;
	cameraShakeStrength_ = strength;
	cameraShakeFrequency_ = frequency;
}

CameraShakeType Player::GetCameraShakeRequest() const
{
	return cameraShakeRequest_;
}

void Player::ResetCameraShakeRequest()
{
	cameraShakeRequest_ = CameraShakeType::None;
}
