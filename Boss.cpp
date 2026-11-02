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
	: ActorBase("Boss"), stateMachine_(*this)
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

	Reset();
}

void Boss::Reset()
{

	counter_ = 0.0f;
	health_ = maxHealth_;
	phase_ = BossPhase::Phase1;
	distance_ = 200.0f;
	handAttackActive_ = false;
	// JSON設定から位置・回転・スケールを読み込む。
	MObjs().LoadActorTransform(*this);
	model_.Load();

	model_.collisionHighlighted = false;
	velocity_ = {};
	activeSkill_ = nullptr;
	SetAnimation(BossAnimType::Idle, true);
	animation_.Reset();
	stateMachine_.Initialize();

	attackHands_.clear();
	attackHands_.resize(2);

	// アニメーションハンドルを初期化する。
	const int idleModelHandle = RM().GetModel(ResourceKeys::Model_BossAnim_Idle);
	idleAnimHandle_ = AnimationHandle::FromIndex(idleModelHandle, 0);

	const int sippoModelHandle = RM().GetModel(ResourceKeys::Model_BossAnim_Sippo);
	tailWhipAnimHandle_ = AnimationHandle::FromIndex(sippoModelHandle, 0);
	const int AtkHitModelHandle = RM().GetModel(ResourceKeys::Model_BossAnim_Attck_Hit);
	atkHitAnimHnadle_ = AnimationHandle::FromIndex(AtkHitModelHandle, 0);
	const int damageModelHandle = RM().GetModel(ResourceKeys::Model_BossAnim_Damage);
	damageAnimHandle_ = AnimationHandle::FromIndex(damageModelHandle, 0);
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

void Boss::TakeDamage(int damaege)
{
	// ダメージを受けたらHPを減らす。
	// その場で停止させ、ひるみ状態へ移行する。
	// ダメージ処理後にフェーズ移行を確認する。

	health_ -= damaege;
	velocity_ = {};

	// ダメージを受けたらひるみ状態へ移行する。
	stateMachine_.ChangeState(BossStateType::flinch);

	// HPが最大値の半分以下になるとフェーズ2へ移行する。
	if (phase_ == BossPhase::Phase1 && health_ <= maxHealth_ / 2)
		phase_ = BossPhase::Phase2;

	if (health_ <= 0)
	{
		health_ = 0;
		stateMachine_.ChangeState(BossStateType::Die);
	}

}

void Boss::Update(float deltaTime)
{
	// アニメーション更新はスキルのフェーズ処理側で行う。
	// 再生区間や攻撃中の動作をスキル側で管理する。
	stateMachine_.Tick(deltaTime);

	position_ += velocity_ * deltaTime;

}

void Boss::Draw() const
{
	model_.ApplyCollisionColor();
	model_.Draw();
}

#ifdef _DEBUG
void Boss::DrawHandAttackDebug() const
{
	if (!handAttackActive_) return;

	DxLib::SetUseZBuffer3D(FALSE);
	DxLib::SetWriteZBuffer3D(FALSE);

	for (std::size_t i = 0; i < attackHands_.size(); ++i)
	{
		const unsigned int color = i == 0
			? DxLib::GetColor(80, 220, 255)
			: DxLib::GetColor(255, 120, 80);
		Collision::DrawDebugSphere(attackHands_[i].hitSphere, color);
	}

	DxLib::SetUseZBuffer3D(TRUE);
	DxLib::SetWriteZBuffer3D(TRUE);
}
#endif
void Boss::UpdateAnimation(float deltaTime)
{
	animation_.Update(deltaTime);
}

AnimationHandle Boss::GetAnimationHandle(BossAnimType::Type type) const
{

	switch (type)
	{
	case BossAnimType::Idle:  return idleAnimHandle_;
	case BossAnimType::TailWhip: return tailWhipAnimHandle_;
	case BossAnimType::Clap: return atkHitAnimHnadle_;
	case BossAnimType::Slam: return atkHitAnimHnadle_;
	case BossAnimType::Damage: return damageAnimHandle_;
	case BossAnimType::Die: return dieAnimHandle_;
	default:
		DxPlus::Utils::FatalError(L"アニメーションハンドルを取得できませんでした。GetAnimationHandleを確認してください。");
		return {};
	}
}

bool Boss::UseSkill(BossAttackType type)
{
	if (stateMachine_.GetCurrentType() == BossStateType::Attack)
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

void Boss::StartHandAttack()
{
	counter_ = 0.0f;
	distance_ = 200.0f;
	handAttackActive_ = true;
	for (AttackData& hand : attackHands_)
	{
		hand.isHit = true;
	}
}

bool Boss::IsHitByAttackSphere(const Collision::Sphere& sphere) const
{
	// 手攻撃が有効なとき、左右の手の当たり判定を確認する。
	if (!handAttackActive_) return false;

	for (const AttackData& hand : attackHands_)
	{
		if (Collision::IsHitSphereSphere(hand.hitSphere, sphere))
			return true;
	}
	return false;
}

bool Boss::UpdateHandAttack(
	float deltaTime, const Vec3& playerPosition, const Vec3& stagePosition)
{
	if (!handAttackActive_) return false;
	if (distance_ <= 0.0f)
	{
		handAttackActive_ = false;
		return false;
	}

	attackHands_[0].hitSphere.radius = 50.0f;
	attackHands_[1].hitSphere.radius = 50.0f;

	// プレイヤーの位置より30上を中心として扱う。
	Vec3 center = playerPosition + Vec3::Up() * 30.0f;
	if (counter_ < 3.0f)
	{
		oldPos = center;
	}

	counter_ += deltaTime;
	if (counter_ >= 3.0f)
	{
		distance_ -= 600.0f * deltaTime;

		if (distance_ < 0.0f)
		{
			distance_ = 0.0f;
		}
	}

	// ステージ中心からプレイヤー位置へ向かう水平ベクトルを求める。
	Vec3 outward = oldPos - stagePosition;
	outward.y = 0.0f;
	float radius = outward.Length(); // ステージ中心からプレイヤー位置までの水平距離。
	if (radius < 0.001f) return false;
	outward /= radius; // ベクトルを正規化する。

	// 円周上の左右に手を配置し、攻撃の進行に合わせて動かす。
	Vec3 right{ outward.z, 0.0f, -outward.x };
	float angle = distance_ / radius;

	Vec3 circleCenter{ stagePosition.x, oldPos.y, stagePosition.z };
	Vec3 front = outward * std::cos(angle);
	Vec3 side = right * std::sin(angle);

	attackHands_[0].hitSphere.center = circleCenter + (front - side) * radius;
	attackHands_[1].hitSphere.center = circleCenter + (front + side) * radius;

	if (counter_ < 3.0f) return false;

	for (AttackData& hand : attackHands_)
	{
		if (!hand.isHit) continue;

		// プレイヤーは半径30の球として当たり判定する。
		if (Collision::IsHitSphereSphere(hand.hitSphere, Collision::Sphere{ center, 30.0f }))
		{
			attackHands_[0].isHit = false;
			attackHands_[1].isHit = false;
			DxPlus::Debug::SetFormatString(L"Hit by Boss Hand Attack!");
			return true;
		}
	}
	return false;
}
