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
	// 初期位置・回転・拡大率はJSONファイルから読み込む
	MObjs().LoadActorTransform(*this);
	model_.Load();

	model_.collisionHighlighted = false;
	velocity_ = {};
	activeSkill_ = nullptr;
	SetAnimation(BossAnimType::Idle, true);
	animation_.Reset();
	stateMachine_.Initialize();

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
