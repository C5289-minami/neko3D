#include "Player.h"
#include "ResourceManager.h"
#include "ResourceKeys.h"
#include "Consts.h"
#include "Raycast.h"
#include "DxLib.h"
// #include "Stage.h"
#include "ModelObjectManager.h"

#include <cmath>
#include "Sound3D.h"
#include <algorithm>
#include <iostream>


Player::Player()
	: ActorBase("Player"), stateMachine_(*this)
{
	model_.SetOwner(*this);
	model_.SetModelKey(ResourceKeys::Model_Player);

	model_.SetObjectId("player_model");
	model_.SetObjectName("Player");
}


void Player::Init()
{
	const int modelHandle = RM().GetModel(model_.GetModelKey());
	if (modelHandle < 0) return;

	model_.SetModelType(ModelType::Bone);

	Reset();
}


void Player::Reset()
{

	health_ = MaxHealth;
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
	ultimateGauge_ = 0.0f;
	// 初期位置・回転・拡大率は既定値を設定した後にJSONから読み込む
	MObjs().LoadActorTransform(*this);
	model_.Load();

	moveInput_ = {};
	velocity_ = { 0.0f, 0.0f, 0.0f };

    // ゲームをやり直したときも、ダブルジャンプを使える状態に戻す。
    doubleJumpUsed_ = false;

	// 空中から開始し、ステージへのレイキャストで接地させる。
	isGrounded_ = false;
	animation_.Reset();
	SetAnimation(PlayerAnimType::Idle, true);

	stateMachine_.Initialize();
	// 更新を止めても描画できるように初期ポーズを用意する
	animation_.Play3D(RM().GetModel(model_.GetModelKey()), currentAnimIndex_, currentAnimLoop_, Const::ANIM_FPS);
	animation_.Update(0.0f);
}

void Player::Update(float deltaTime)
{
	input_.Update();
	UpdateMovement(deltaTime);
}

void Player::SetModelKey(const std::wstring& key)
{
	// keyがないか、既に設定済みのモデルキーと同じなら何もしない
	if (key.empty() || key == model_.GetModelKey()) return;

	const int modelHandle = RM().GetModel(key);
	if (modelHandle < 0) return;

	model_.SetModelKey(key);
	model_.SetModelType(ModelType::Bone);

	animation_.Reset();
	currentAnimIndex_ = static_cast<int>(PlayerAnimType::Idle);
	currentAnimLoop_ = true;

	animation_.Play3D(
		modelHandle,
		currentAnimIndex_,
		currentAnimLoop_,
		Const::ANIM_FPS);

	animation_.Update(0.0f);
}
void Player::UpdateMovement(float deltaTime)
{
    // 前のフレームで床に着いたら、2回目のジャンプを再び使えるようにする。
    // 空中で被弾してステートが変わっても、このフラグはリセットしない。
    if (isGrounded_)
    {
        doubleJumpUsed_ = false;
    }

    // 段階実装：接触のTrue/Falseだけを残し、壁登りへの自動遷移は停止する。
//     // 移動入力を離している間は、壁登り用の接触状態を解除する。
//     if (!input_.isMoving)
//     {
//         touchingWall_ = false;
//     }

// 	bool NearTheWall = false;
// 	if (input_.isMoving)
// 	{
// 		Vec3 moveDirection = GetWorldMoveDirection(input_.moveDir);
// 		float wallDirection = Vec3::Dot(moveDirection, wallNormal_);//壁の法線と移動方向の内積を計算して、壁に対してどの方向に向かっているかを判定する

// 		if (wallDirection < -0.25f)
// 		{
// 			NearTheWall = true;
// 		}
// 	}
// 	//か
// 	if (touchingWall_ && gravityEnabled_ && !IsDead() && !IsDamaging() &&
// 		GetCurrentState() != PlayerStateType::WallJump &&
// 		NearTheWall)
// 	{
// 		stateMachine_.ChangeState(PlayerStateType::Climbing);
// 	}

	DxPlus::Debug::SetFormatString(L"CharacterSetting: %s", charData.name.c_str());

	// 接触中の柱に沿って上下移動する Elevator は、ジャンプで離れるまで続く。
	const bool elevatorFrame = GetCurrentState() == PlayerStateType::Climbing;
	stateMachine_.Tick(deltaTime);

	// 到着してジャンプへ切り替わるフレームも、上下移動を二重に加えない。
	if (!elevatorFrame)
	{
		if (isdefMove_)
		{
			// カメラの向きに合わせて移動方向を決める。
			const float cameraRotY = DxLib::GetCameraAngleHRotate();
			const Vec3 cameraForward{ std::sin(cameraRotY), 0.0f, std::cos(cameraRotY) };
			const Vec3 cameraRight{ cameraForward.z, 0.0f, -cameraForward.x };


            // 旧仕様の入力固定と横方向への突進は使わない。
//             bool isLunging = GetCurrentState() == PlayerStateType::LungingJump;
//             Vec3 moveInput = input_.moveDir;//とびかかり中：moveInput_＝発動した瞬間に保存した入力
// 											//通常移動中：input_.moveDir＝今押している入力だよん
//
//             float moveSpeed = Const::PLAYER_MOVE_SPEED;
//             if (isLunging)
//             {
//                 moveInput = moveInput_;
//                 moveSpeed = lungingMoveSpeed_;
//             }

            // 地上・空中とも、現在の入力で通常速度の移動だけを行う。
            Vec3 moveInput = input_.moveDir;
            //if (moveInput.x != 0.0f && moveInput.z != 0.0f)
            //{
            //    moveInput = {};
            //}
            float moveSpeed = Const::PLAYER_MOVE_SPEED;

            Vec3 moveDirection = cameraRight * moveInput.x + cameraForward * moveInput.z;
			moveDirection.y = 0.0f;

			if (moveDirection.LengthSq() > 0.0f)
			{
				moveDirection = moveDirection.Normalized();
				float moveSpeed = Const::PLAYER_MOVE_SPEED;
#ifdef _DEBUG
				if (debugDashEnabled_) moveSpeed *= 5.0f;
#endif
				position_.x += moveDirection.x * moveSpeed * deltaTime;
				position_.z += moveDirection.z * moveSpeed * deltaTime;
				TurnTowards(moveDirection, deltaTime);
			}
		}
		else
		{
			// 中心がどっちにあるか調べる。高さはここでは考えない
			MoveAroundStage(deltaTime, position_, movementCenter_);
		}
		// 横移動は MoveAroundStage、上下移動は velocity_.y にまとめる。

		if (gravityEnabled_ && GetCurrentState() != PlayerStateType::Climbing)
		{
			velocity_.y -= Const::GRAVITY * deltaTime;
		}
		position_.y += velocity_.y * deltaTime;
		isGrounded_ = false;
	}

	const int modelHandle = RM().GetModel(model_.GetModelKey());
	animation_.Play3D(modelHandle, currentAnimIndex_, currentAnimLoop_, Const::ANIM_FPS);
	animation_.Update(deltaTime);
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
		else
			EndPillarElevator();
	}
	if (canStartElevator && touchingWall_)
	{
		StartPillarElevator(pillarModel, pillarCenter, wallTopY_);
	}
	if (IsPillarElevatorActive()) input_.jumpPressed = false;
	UpdateMovement(deltaTime);
    // 旧仕様：中心へ寄せるだけの処理。
// 	const bool snapToPillar = touchingWall_ && input_.jumpPressing && !IsDead() && !IsDamaging();
// 	// 柱に触れているときのSpaceは、通常ジャンプより中心への吸着を優先する。
// 	if (snapToPillar) input_.jumpPressed = false;
//
// 	UpdateMovement(deltaTime);
// 	if (snapToPillar)
// 	{
// 		// 高さは動かさず、横方向だけ柱の中心へ寄せる。
// 		position_.x = pillarCenter.x;
// 		position_.z = pillarCenter.z;
// 		StopMove();
// 	}

	// // ステージの柱に沿ってスライドさせる
	// position_ = stage.SlideAlongPillars(oldPosition, position_, touchingPillar);
	// 登録されたモデルの壁に沿ってスライドさせる
	const Vec3 desiredPosition = position_;
	position_ = collision.SlideAlongWalls(oldPosition, position_, modelOb, touchingModel_);
	if (IsPillarElevatorActive() && (position_ - desiredPosition).LengthSq() > Const::PLAYER_SKIN * Const::PLAYER_SKIN)
		EndPillarElevator();

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
			hit))
		{
			position_.y = hit.point.y - Const::PLAYER_HEIGHT - Const::PLAYER_SKIN;
			velocity_.y = 0.0f;
			isGrounded_ = false;
			EndPillarElevator();
		}
	}


	// 落下中のみ床面に合わせ、このフレームの落下距離もレイの長さに含める。
	// エレベーター中と、到着時の小ジャンプ中は床へ吸着させない。
	// if (dy<= 0.0f)
	if (dy <= 0.0f && velocity_.y <= 0.0f && GetCurrentState() != PlayerStateType::Climbing)
	{
		Physics::RayHit hit{};
		constexpr float groundSnapDistance = 30.0f;
		const float fallDistance = previousY - position_.y;
		const float rayDistance =
			fallDistance + groundSnapDistance + Const::PLAYER_SKIN;

		// std::vector<int> handles;
		// stage.GetCollisionModelHandles(handles);

		// if (Physics::RaycastDown(
			// handles,
		const Vec3 origin{ GetPosition().x, previousY + Const::PLAYER_SKIN, GetPosition().z };
		bool foundGround = collision.RaycastDown(origin, rayDistance, hit);
		Physics::RayHit pillarHit{};
		// 小ジャンプの頂点から急に吸い戻されないよう、柱は足元付近だけで着地する。
		const float pillarRayDistance = fallDistance + Const::PLAYER_SKIN * 2.0f;
		if (collision.RaycastPillarTop(origin, pillarRayDistance, Const::PLAYER_ELEVATOR_TOP_OFFSET, pillarHit) &&
			(!foundGround || pillarHit.distance < hit.distance))
		{
			hit = pillarHit;
			foundGround = true;
		}
		if (foundGround)
		{
            // 旧仕様：壁の上端で接触を保持している間の床補正。
            // const bool holdingClimbTop = GetCurrentState() == PlayerStateType::Climbing && IsNearWallTop();
            // if (position_.y - hit.point.y <= groundSnapDistance &&
            //     (!holdingClimbTop || hit.point.y >= position_.y))
			if (position_.y - hit.point.y <= groundSnapDistance)
			{
				position_.y = hit.point.y;
				velocity_.y = 0.0f;
				isGrounded_ = true;
			}
		}
	}

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

void Player::Draw() const
{
	const Matrix4 worldMatrix = model_.GetWorldMatrix();

	model_.ApplyCollisionColor();

	animation_.Draw3D(
		worldMatrix.GetTranslation(),
		worldMatrix.GetScale(),
		worldMatrix.GetRotationXYZ()
	);
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
    // 前後と左右の同時入力は、ジャンプ中も両方とも無効にする。
    //if (moveInput_.x != 0.0f && moveInput_.z != 0.0f) moveInput_ = {};
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

	if (forward.LengthSq() < 0.000001f)
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

void Player::MoveAroundStage(float deltaTime, Vec3& playerPos, const Vec3& movementCenter)
{
	if (deltaTime <= 0.0f) return;

	if (GetCurrentState() == PlayerStateType::WallJump)
	{
		// 壁を蹴った方向を保ち、入力や足場の中心に左右されず壁から離れる。
		playerPos.x += velocity_.x * deltaTime;
		playerPos.z += velocity_.z * deltaTime;
		TurnTowards(Vec3(velocity_.x, 0.0f, velocity_.z), deltaTime);
		return;
	}
	if (moveInput_.LengthSq() == 0.0f)
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

    // 旧仕様の突進速度は使わず、2回目のジャンプも通常移動速度にする。
// 	const float moveSpeed =
// 		GetCurrentState() == PlayerStateType::LungingJump ?
// 		lungingMoveSpeed_ : Const::PLAYER_MOVE_SPEED;
// 	if (GetCurrentState() == PlayerStateType::LungingJump)
// 	{
// 		input_.isMoving = false;
// 	}
	float normalMoveSpeed = Const::PLAYER_MOVE_SPEED;
#ifdef _DEBUG
	if (debugDashEnabled_) normalMoveSpeed *= 5.0f;
#endif
	const float moveSpeed = normalMoveSpeed;

	const float distance = moveSpeed * deltaTime;



	const float nextRadius = (std::max)(1.0f, radius - input.z * distance);
	const float angle = input.x * distance / (std::max)(1.0f, radius);
	const Vec3 outward = -forward * std::cos(angle) + right * std::sin(angle);

	playerPos.x = movementCenter.x + outward.x * nextRadius;
	playerPos.z = movementCenter.z + outward.z * nextRadius;

	TurnTowards(forward * input.z + right * input.x, deltaTime);
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
    if (!CanDoubleJump()) return;
    doubleJumpUsed_ = true;
    StopMove();
    SetMoveDirection(input_.moveDir);
    velocity_.y = doubleJumpSpeed_;
    isGrounded_ = false;
}

// 旧仕様：空中で入力方向へ突進する処理。
// void Player::SetLungingMoveSpeed(float value)
// {
// 	if (std::isfinite(value)) lungingMoveSpeed_ = (std::max)(0.0f, value);
// }
//
// void Player::SetLungingJumpSpeed(float value)
// {
// 	if (std::isfinite(value)) lungingJumpSpeed_ = (std::max)(0.0f, value);
// }
//
// void Player::ResetLungingSpeeds()
// {
// 	lungingMoveSpeed_ = Const::PLAYER_LUNGING_MOVE_SPEED;
// 	lungingJumpSpeed_ = Const::PLAYER_LUNGING_JUMP_SPEED;
// }
//
// void Player::StartLungingJump(const Vec3& moveDir)
// {
//     // 地面にいるときや、すでに2回目を使った後は発動させない。
//     if (!CanAirLunge())
//     {
//         return;
//     }
//
//     // 発動したことを覚える。着地するまで、もう一度は使えない。
//     doubleJumpUsed_ = true;
//     StopMove();
//
//     // 2回目のSpaceを押した瞬間の方向を保存する。
//     Vec3 moveInput = moveDir;
//     // 方向キーを押していなければ、前へとびかかる。
//     // 円周移動ではボス方向、通常移動ではカメラが向いている方向になる。
//     if (moveInput.LengthSq() == 0.0f)
//     {
//         moveInput = Vec3(0.0f, 0.0f, 1.0f);
//     }
// 	// 左右と前後が同時に入力された場合、左右を優先して斜め移動を防ぐ
// 	if (moveInput.x != 0.0f)
// 		moveInput.z = 0.0f;
//
//     SetMoveDirection(moveInput);
//     // 落下中でも上向きの速度に入れ直すので、空中でもう一度跳べる。
//     velocity_.y = lungingJumpSpeed_;
//     isGrounded_ = false;
// }

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
		EndPillarElevator();
	}
}

void Player::StartPillarElevator(const ModelObject* pillar, const Vec3& center, float topY)
{
    const float arrivalY = topY - Const::PLAYER_ELEVATOR_TOP_OFFSET;
    if (!pillar || !gravityEnabled_|| IsDamaging() ||
        IsPillarElevatorActive() || position_.y >= arrivalY - Const::PLAYER_SKIN) return;

	elevatorPillar_ = pillar;
    elevatorCenter_ = center;
    elevatorTopY_ = arrivalY;
    elevatorInputConsumed_ = true;

    position_.x = center.x;
    position_.z = center.z;

	isGrounded_ = false;
    stateMachine_.ChangeState(PlayerStateType::Climbing);
}

bool Player::UpdatePillarElevator(float deltaTime)
{
    StopMove();
    velocity_.y = 0.0f;
    isGrounded_ = false;
    position_.x = elevatorCenter_.x;
    position_.z = elevatorCenter_.z;
    if (deltaTime <= 0.0f) return false;
    position_.y = (std::min)(elevatorTopY_, position_.y + climbingSpeed_ * deltaTime);
    return position_.y >= elevatorTopY_;
}

void Player::EndPillarElevator()
{
    elevatorPillar_ = nullptr;
}

void Player::StartPillarExitJump()
{
    velocity_.y = Const::PLAYER_ELEVATOR_EXIT_JUMP_SPEED;
    isGrounded_ = false;
    elevatorInputConsumed_ = true;
}
