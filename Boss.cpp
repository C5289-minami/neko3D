#include "Boss.h"
#include "Consts.h"
#include "ResourceKeys.h"
#include "ResourceManager.h"

#include <cmath>
#include "DxPlus/Debug.h"
#include "ModelObjectManager.h"

// skill
#include "BossClapSkill.h"
#include "BossTailWhipSkill.h"
#include "BossSlamSkill.h"


Boss::Boss()
	: ActorBase("Boss"), stateMachine_(*this), ai_(*this)
{
	model_.SetOwner(*this);
	model_.SetModelKey(ResourceKeys::Model_Boss);

	model_.SetObjectId("boss_model");
	model_.SetObjectName("Boss");
}
Boss::~Boss() = default;

void Boss::Init()
{
	activeSkill_ = nullptr;
	stateMachine_.Initialize();

	model_.SetModelType(ModelType::Bone);

	clapSkill_ = std::make_unique<BossClapSkill>(*this);
	slamSkill_ = std::make_unique<BossSlamSkill>(*this);
	tailWhipSkill_ = std::make_unique<BossTailWhipSkill>(*this);

	// AnimationHandleの初期化
	const int idleModelHandle = RM().GetModel(ResourceKeys::Model_BossAnim_Idle);
	idleAnimHandle_ = AnimationHandle::FromIndex(idleModelHandle, 0);
	const int clapModelHandle = RM().GetModel(ResourceKeys::Model_BossAnim_Nekodamashi);
	ClapAnimHandle_ = AnimationHandle::FromIndex(clapModelHandle, 0);

	const int sippoModelHandle = RM().GetModel(ResourceKeys::Model_BossAnim_Sippo);
	tailWhipAnimHandle_ = AnimationHandle::FromIndex(sippoModelHandle, 0);
	const int AtkHitModelHandle = RM().GetModel(ResourceKeys::Model_BossAnim_Attck_Hit);
	atkHitAnimHnadle_ = AnimationHandle::FromIndex(AtkHitModelHandle, 0);
	const int downModelHandle = RM().GetModel(ResourceKeys::Model_BossAnim_Down);
	flachAnimHandle_ = AnimationHandle::FromIndex(downModelHandle, 0);
	const int dieModelHandle = RM().GetModel(ResourceKeys::Model_BossAnim_Die);
	dieAnimHandle_ = AnimationHandle::FromIndex(dieModelHandle, 0);

	Reset();
}

void Boss::Reset()
{
	isActive_ = true;
	model_.SetAlpha(1.0f);

	downGauge_ = maxDownGauge_;
	damageTakenThisFrame_ = false;

	health_ = maxHealth_;
	phase_ = BossPhase::Phase1;
	// JSON設定から位置・回転・スケールを読み込む。
	MObjs().LoadActorTransform(*this);
	model_.Load();

	model_.collisionHighlighted = false;
	velocity_ = {};
	activeSkill_ = nullptr;
	SetAnimation(BossAnimType::Idle, true);

	stateMachine_.Initialize();

#ifndef NDEBUG
	activeAI = false;
#endif // !NDEBUG

	ai_.Reset();
	if (clapSkill_ != nullptr) clapSkill_->Reset();
	if (slamSkill_ != nullptr) slamSkill_->Reset();

	// アニメーションハンドルを初期化する。
	const int idleModelHandle = RM().GetModel(ResourceKeys::Model_BossAnim_Idle);
	idleAnimHandle_ = AnimationHandle::FromIndex(idleModelHandle, 0);

	const int clapModelHandle = RM().GetModel(ResourceKeys::Model_BossAnim_Nekodamashi);
	ClapAnimHandle_ = AnimationHandle::FromIndex(clapModelHandle, 0);

	const int sippoModelHandle = RM().GetModel(ResourceKeys::Model_BossAnim_Sippo);
	tailWhipAnimHandle_ = AnimationHandle::FromIndex(sippoModelHandle, 0);

	const int AtkHitModelHandle = RM().GetModel(ResourceKeys::Model_BossAnim_Attck_Hit);
	atkHitAnimHnadle_ = AnimationHandle::FromIndex(AtkHitModelHandle, 0);

	const int downModelHandle = RM().GetModel(ResourceKeys::Model_BossAnim_Down);
	flachAnimHandle_ = AnimationHandle::FromIndex(downModelHandle, 0);

	const int dieModelHandle = RM().GetModel(ResourceKeys::Model_BossAnim_Die);
	dieAnimHandle_ = AnimationHandle::FromIndex(dieModelHandle, 0);

	// 死亡モデルは本体と同じテクスチャを使うため、Bossモデルのテクスチャを共有する。
	//const int baseModelHandle = RM().GetModel(ResourceKeys::Model_Boss);
	//const int baseTextureCount = DxLib::MV1GetTextureNum(baseModelHandle);
	//const int dieTextureCount = DxLib::MV1GetTextureNum(dieModelHandle);
	//if (baseTextureCount > 0)
	//{
	//	const int sharedTextureHandle = DxLib::MV1GetTextureGraphHandle(baseModelHandle, 0);
	//	if (sharedTextureHandle != -1)
	//	{
	//		for (int textureIndex = 0; textureIndex < dieTextureCount; ++textureIndex)
	//			DxLib::MV1SetTextureGraphHandle(dieModelHandle, textureIndex, sharedTextureHandle, FALSE);
	//	}
	//}

	// 初期アニメーションを再生する。
	animation_.Play3D(
		RM().GetModel(model_.GetModelKey()),
		GetAnimationHandle(BossAnimType::Idle),
		true,
		Const::ANIM_FPS
	);
	animation_.Update(0.0f);

}

void Boss::TakeDamage(int damage)
{
	damageTakenThisFrame_ = true;
	// ダメージを受けたらHPを減らす。
	// ダメージを受けると移動を止め、ゲージを減らす。
	// ダメージ処理後にフェーズ移行を確認する。


	health_ -= damage;
	velocity_ = {};




	// HPが最大値の半分以下になるとフェーズ2へ移行する。
	if (phase_ == BossPhase::Phase1 && health_ <= maxHealth_ / 2)
		phase_ = BossPhase::Phase2;

	if (health_ <= 0)
	{
		health_ = 0;
		stateMachine_.ChangeState(BossStateType::Die);
	}

}

void Boss::ApplyDownGaugeDamage(float damage)
{
	if (health_ <= 0 || !std::isfinite(damage) || damage <= 0.0f) return;
	SetDownGauge(downGauge_ - damage);
	if (downGauge_ <= 0.0f && GetCurrentState() != BossStateType::Flinch)
		stateMachine_.ChangeState(BossStateType::Flinch);
}

void Boss::UpdateDown(float deltaTime)
{
	float AccumulationCounter = 1.0f * deltaTime; // 累積カウンターを更新する。
	if (health_ <= 0 && downGauge_ <= 0)
		return;


	downGauge_ -= deltaTime * 10.0f; // ダウンゲージの減少速度を調整する。

	if (AccumulationCounter >= 5.0f)
	{
		downGauge_ -= 5.0f;			// 5秒ごとにさらに減少させる。
		AccumulationCounter = 0.0f; // カウンターをリセットする。
	}
	if (damageTakenThisFrame_)
	{
		downGauge_ -= 20.0f;
		damageTakenThisFrame_ = false;
	} // ダメージを受けた瞬間にさらに減少させる。

	if (downGauge_ <= 0.0f)
	{
		downGauge_ = 0.0f;
		stateMachine_.ChangeState(BossStateType::Flinch);
	}

}

void Boss::Update(float deltaTime)
{
	// アニメーション更新はスキルのフェーズ処理側で行う。
	// 再生区間や攻撃中の動作をスキル側で管理する。

	// AIの更新

#ifndef NDEBUG
	if (activeAI)
#endif
	{
		ai_.Update(deltaTime);
	}

	stateMachine_.Tick(deltaTime);


	position_ += velocity_ * deltaTime;

	if (damageTakenThisFrame_) UpdateDown(deltaTime);

	if (health_ > 0 && downGauge_ <= 0.0f &&
		stateMachine_.GetCurrentType() != BossStateType::Flinch)
	{
		stateMachine_.ChangeState(BossStateType::Flinch);
	}

}


void Boss::TurnToTarget(const Vec3& targetPosition, float deltaTime)
{
	ai_.TurnToTarget(targetPosition, deltaTime);
}


void Boss::Draw() const
{
	model_.ApplyCollisionColor();
	model_.Draw();
}

void Boss::CameraShakeRequest(
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

CameraShakeType Boss::GetCameraShakeRequest() const
{
	return cameraShakeRequest_;
}

void Boss::ResetCameraShakeRequest()
{
	cameraShakeRequest_ = CameraShakeType::None;
}
#ifdef _DEBUG

void Boss::DrawClapAttackDebug() const
{
	if (clapSkill_ != nullptr)
		clapSkill_->DrawDebug();
}

void Boss::DrawSlamAttackDebug() const
{
	if (GetCurrentState() == BossStateType::Attack && activeSkill_ == slamSkill_.get() && slamSkill_ != nullptr)
		slamSkill_->DrawDebug();
}
#endif

void Boss::UpdateAnimation(float deltaTime)
{
	animation_.Update(deltaTime);
}

void Boss::StartRotation()
{

	isRotationEnabled_ = true;
}

void Boss::StopRotation()
{
	isRotationEnabled_ = false;
}

AnimationHandle Boss::GetAnimationHandle(BossAnimType::Type type) const
{

	switch (type)
	{
	case BossAnimType::Idle:  return idleAnimHandle_;
	case BossAnimType::TailWhip: return tailWhipAnimHandle_;
	case BossAnimType::Clap: return ClapAnimHandle_;
	case BossAnimType::Slam: return atkHitAnimHnadle_;
	case BossAnimType::Flach: return flachAnimHandle_;
	case BossAnimType::Die: return dieAnimHandle_;
	default:
		DxPlus::Utils::FatalError(L"アニメーションハンドルを取得できませんでした。GetAnimationHandleを確認してください。");
		return idleAnimHandle_;
	}
}

bool Boss::UseSkill(BossAttackType type)
{
	const BossStateType currentState = stateMachine_.GetCurrentType();
	if (currentState == BossStateType::Attack ||
		currentState == BossStateType::Flinch ||
		currentState == BossStateType::Die)
		return false;

	switch (type)
	{
	case BossAttackType::Clap:
		clapSkill_->Reset();
		activeSkill_ = clapSkill_.get();
		break;

	case BossAttackType::Slam:
		slamSkill_->Reset();
		activeSkill_ = slamSkill_.get();
		break;

	case BossAttackType::TailWhip:
		tailWhipSkill_->Reset();
		activeSkill_ = tailWhipSkill_.get();
		break;

	default:
		return false;
	}

	stateMachine_.ChangeState(BossStateType::Attack);
	return stateMachine_.GetCurrentType() == BossStateType::Attack;
}

bool Boss::IsTailWhipActive() const
{
	return GetCurrentState() == BossStateType::Attack &&
		tailWhipSkill_ != nullptr &&
		activeSkill_ == tailWhipSkill_.get();
}


bool Boss::IsTailWhipHitActive() const
{
	return tailWhipSkill_ != nullptr && tailWhipSkill_->IsHitActive();
}

Collision::Capsule Boss::GetTailWhipHitCapsule(const Vec3& stageCenter, float stageRadius, float attackHeight) const
{
	if (tailWhipSkill_ != nullptr)
	{
		return tailWhipSkill_->GetHitCapsule(stageCenter, stageRadius, attackHeight);
	}

	return Collision::Capsule{};
}
void Boss::SetMoveDirection(const Vec3& moveDirection, float speed)
{
	Vec3 direction{ moveDirection.x, 0.0f, moveDirection.z };
	if (direction.LengthSq() <= 0.0f || speed <= 0.0f)
	{
		StopMove();
		return;
	}

	direction = direction.Normalized();
	velocity_ = direction * speed;
	yaw_ = std::atan2(direction.x, direction.z);
	stateMachine_.ChangeState(BossStateType::Walk);
}

void Boss::StopMove()
{
	velocity_ = {};
	if (stateMachine_.GetCurrentType() == BossStateType::Walk)
	{
		stateMachine_.ChangeState(BossStateType::Idle);
	}
}

const int Boss::GetModelHandle() const { return RM().GetModel(model_.GetModelKey()); }

void Boss::SetAnimation(
	BossAnimType::Type type,
	bool loop,
	bool playAfterBlend,
	float blendSeconds)
{
	if (currentAnimType_ == type && currentAnimLoop_ == loop)
		return;

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

float Boss::GetAnimTotalTime(BossAnimType::Type type) const
{
	float totalTime{};
	float totalFlame{};
	int index = static_cast<int>(type);
	int modelHandle = RM().GetModel(model_.GetModelKey());

	totalFlame = DxLib::MV1GetAnimTotalTime(modelHandle, index);
	totalTime = totalFlame / Const::ANIM_FPS; // フレーム数を秒数に変換する。
	return totalTime;
}


bool Boss::IsHitByAttackSphere(const Collision::Sphere& sphere, const Collision::Capsule& tailWhipCapsule) const
{
	if (health_ <= 0 || GetCurrentState() != BossStateType::Attack || sphere.radius <= 0.0f)
		return false;

	if (clapSkill_ != nullptr && activeSkill_ == clapSkill_.get())
		return clapSkill_->IsHitByAttackSphere(sphere);

	if (IsSlamHitActive())
	{
		// Slam攻撃が有効な場合、Slamの当たり判定カプセルとプレイヤーの攻撃球が重なるか調べる。
		for (const auto& capsule : slamSkill_->GetHitCapsules())
		{
			if (capsule.radius > 0.0f && Collision::IsHitCapsuleSphere(capsule, sphere))
				return true;
		}
	}

	if (IsTailWhipActive() && IsTailWhipHitActive())// 尻尾攻撃が有効な場合、尻尾の当たり判定カプセルとプレイヤーの攻撃球が重なるか調べる。
	{
		return tailWhipCapsule.radius > 0.0f &&
			Collision::IsHitCapsuleSphere(tailWhipCapsule, sphere);
	}

	return false;
}

bool Boss::UpdateHandAttack(float deltaTime, const Vec3& playerPosition, const Vec3& stagePosition)
{
	return clapSkill_ != nullptr && clapSkill_->UpdateHandAttack(playerPosition, stagePosition);
}

void Boss::UpdateClapTargetPosition(const Vec3& playerPosition)
{
	if (clapSkill_ != nullptr)
		clapSkill_->UpdateTargetPosition(playerPosition);
}
bool Boss::UpdateSlamAttack(const Vec3& playerPosition, const Vec3& stageCenter)
{
	bool hit = false;
	if (IsSlamSkillActive())
		hit = slamSkill_->UpdateAttack(playerPosition, stageCenter);
	return hit;
}

bool Boss::IsSlamHitActive() const
{
	bool hitActive = false;
	if (IsSlamSkillActive())
		hitActive = slamSkill_->IsHitActive();
	return hitActive;
}

bool Boss::IsSlamSkillActive() const
{
	return GetCurrentState() == BossStateType::Attack && slamSkill_ != nullptr && activeSkill_ == slamSkill_.get();
}
