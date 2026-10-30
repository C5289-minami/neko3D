#include "Boss.h"
#include "Consts.h"
#include "ResourceKeys.h"
#include "ResourceManager.h"

#include <cmath>
#include "DxPlus/Debug.h"
#include "ModelObjectManager.h"

#include <Windows.h>
#include <cstdarg>
#include <cstdio>

#ifndef NDEBUG

namespace
{
	const char* GetVertexTypeName(int type)
	{
		switch (type)
		{
		case DX_MV1_VERTEX_TYPE_1FRAME:  return "1FRAME";
		case DX_MV1_VERTEX_TYPE_4FRAME:  return "4FRAME";
		case DX_MV1_VERTEX_TYPE_8FRAME:  return "8FRAME";
		case DX_MV1_VERTEX_TYPE_FREE_FRAME: return "FREE_FRAME";
		case DX_MV1_VERTEX_TYPE_NMAP_1FRAME: return "NMAP_1FRAME";
		case DX_MV1_VERTEX_TYPE_NMAP_4FRAME: return "NMAP_4FRAME";
		case DX_MV1_VERTEX_TYPE_NMAP_8FRAME: return "NMAP_8FRAME";
		case DX_MV1_VERTEX_TYPE_NMAP_FREE_FRAME: return "NMAP_FREE_FRAME";
		default: return "UNKNOWN";
		}
	}
	void DebugLog(const char* format, ...)
	{
		char buffer[512]{};

		va_list args;
		va_start(args, format);
		vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, format, args);
		va_end(args);

		OutputDebugStringA(buffer);
	}
	void DebugModelVertexTypes(const char* name, int handle)
	{
		if (handle < 0)
		{
			DebugLog("[ModelCheck] %s: invalid handle\n", name);
			return;
		}

		const int frameNum = MV1GetFrameNum(handle);
		const int listNum = MV1GetTriangleListNum(handle);

		DebugLog("\n===== %s Model Check =====\n", name);
		DebugLog("FrameNum: %d\n", frameNum);
		DebugLog("TriangleListNum: %d\n", listNum);

		for (int i = 0; i < listNum; ++i)
		{
			const int type = MV1GetTriangleListVertexType(handle, i);

			DebugLog(
				"List[%d]: Type=%d (%s), Vertices=%d\n",
				i,
				type,
				GetVertexTypeName(type),
				MV1GetTriangleListVertexNum(handle, i)
			);
		}
	}
}

#endif
Boss::Boss()
	: ActorBase("Boss"), stateMachine_(*this)
{
	model_.SetOwner(*this);
	model_.SetModelKey(ResourceKeys::Model_Boss);

	model_.SetObjectId("boss_model");
	model_.SetObjectName("Boss");

	currentAnimIndex_ = 0;
}

void Boss::Init()
{
	activeSkill_ = nullptr;
	stateMachine_.Initialize();

	model_.SetModelType(ModelType::Bone);
	
	Reset();
#ifndef NDEBUG
	DebugModelVertexTypes("Boss", RM().GetModel(model_.GetModelKey()));
#endif
}

void Boss::Reset()
{
	counter_ = 0.0f;
	distance_ = 200.0f;
	handAttackActive_ = false;
	// 初期位置・回転・拡大率はJSONファイルから読み込む
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

	// 更新を止めても描画できるように初期ポーズを用意する
	animation_.Play3D(RM().GetModel(model_.GetModelKey()), currentAnimIndex_, currentAnimLoop_, Const::ANIM_FPS);
	animation_.Update(0.0f);
}

void Boss::Update(float deltaTime)
{

	stateMachine_.Tick(deltaTime);
	position_ += velocity_ * deltaTime;

	const int modelHandle = RM().GetModel(model_.GetModelKey());
	animation_.Play3D(modelHandle, currentAnimIndex_, currentAnimLoop_, Const::ANIM_FPS);
	animation_.Update(deltaTime);

	DxPlus::Debug::SetString(L"Kで歩き / スキルはBoss::UseSkillから発動");
}

void Boss::Draw() const
{
	const ModelObject worldModel = model_.WithParentTransform(GetPosition(), GetRotation());
	worldModel.ApplyCollisionColor();
	worldModel.Draw();
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
	pitch_ = std::atan2(direction.x, direction.z);
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

float Boss::GetAnimTotalTime(BossAnimType::Type type) const
{
	float totalTime{};
	float totalFlame{};
	int index = static_cast<int>(type);
	int modelHandle = RM().GetModel(model_.GetModelKey());

	totalFlame = DxLib::MV1GetAnimTotalTime(modelHandle, index);
	totalTime = totalFlame / Const::ANIM_FPS; // 秒数に変換
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

	Vec3 center = playerPosition + Vec3::Up() * 30.0f;// プレイヤーの頭上30.0fの位置を中心にする
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

	// ステージ中心から、狙う位置へ向かう方向を求める。
	Vec3 outward = oldPos - stagePosition;
	outward.y = 0.0f;
	float radius = outward.Length();// ステージ中心から狙う位置までの距離
	if (radius < 0.001f) return false;
	outward /= radius;// 正規化

	// 同じ円周の左右に置いて、間隔が縮まると狙う位置へ近づく。

	Vec3 right{ outward.z, 0.0f, -outward.x };
	float angle = distance_ / radius;

	Vec3 circleCenter{ stagePosition.x, oldPos.y, stagePosition.z };
	Vec3 front = outward * std::cos(angle);
	Vec3 side = right * std::sin(angle);

	attackHands_[0].hitSphere.center = circleCenter + (front - side) * radius;
	attackHands_[1].hitSphere.center = circleCenter + (front + side) * radius;

	if (counter_ < 3.0f) return false;

	for(AttackData& hand : attackHands_)
	{
		if (!hand.isHit)continue;

		if (Collision::IsHitSphereSphere(hand.hitSphere, Collision::Sphere{ center, 30.0f }))// プレイヤーの当たり判定は半径30.0fの球と仮定
		{
			attackHands_[0].isHit = false;
			attackHands_[1].isHit = false;
			DxPlus::Debug::SetFormatString(L"Hit by Boss Hand Attack!");
			return true;
			
		}

	}
	return false;
}

bool Boss::UseSkill(IAttackSkill& skill)
{
	if (stateMachine_.GetCurrentType() == BossStateType::Attack)
	{
		return false;
	}

	activeSkill_ = &skill;
	stateMachine_.ChangeState(BossStateType::Attack);
	if (stateMachine_.GetCurrentType() != BossStateType::Attack)
	{
		activeSkill_ = nullptr;
		return false;
	}
	return true;
}
