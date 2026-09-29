#include "EffectManager.h"
#include "EffekseerForDXLib.h"
#include "ResourceManager.h"
#include "ResourceKeys.h"

void EffectManager::Load()
{
	// NULL
}

void EffectManager::Update(float deltaTime)
{
	UpdateEffekseer3D(deltaTime);
}

void EffectManager::Draw() const
{
	Effekseer_Sync3DSetting();// 3D描画設定をEffekseerに反映
	DrawEffekseer3D();
}

void EffectManager::Finalize()
{
	// NULL
}

void EffectManager::PlayHit(int key,const Vec3& position,float yaw)
{
	if(key < 0) return;
	// エフェクトを再生する
	int playingHandle = PlayEffekseer3DEffect(key);
	if(playingHandle < 0) return;
	SetRotationPlayingEffekseer3DEffect(playingHandle,0.0f, yaw,0.0f);
	SetPosPlayingEffekseer3DEffect(playingHandle, position.x, position.y, position.z);
}
