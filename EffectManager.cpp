#include "EffectManager.h"
#include "EffekseerForDXLib.h"
#include "ResourceManager.h"
#include "ResourceKeys.h"


void EffectManager::Update(float deltaTime)
{
	UpdateEffekseer3D(deltaTime);
}

void EffectManager::Draw() const
{
	Effekseer_Sync3DSetting();// 3D描画設定をEffekseerに反映
	DrawEffekseer3D();
}


int EffectManager::PlayEffect(int key, const Vec3& position, float yaw)
{
	if (key < 0) return -1;
	// エフェクトを再生する
	const int playingHandle = PlayEffekseer3DEffect(key);
	if (playingHandle < 0) return -1;
	SetRotationPlayingEffekseer3DEffect(playingHandle, 0.0f, yaw, 0.0f);
	SetPosPlayingEffekseer3DEffect(playingHandle, position.x, position.y, position.z);
	return playingHandle;
}

bool EffectManager::MoveEffect(int handle, const Vec3& position)
{
	if (handle < 0 || IsEffekseer3DEffectPlaying(handle) != 0) return false;
	return SetPosPlayingEffekseer3DEffect(handle, position.x, position.y, position.z) == 0;
}

void EffectManager::StopEffect(int handle)
{
	if (handle >= 0)
		StopEffekseer3DEffect(handle);
}

void EffectManager::PlayHit(int key,const Vec3& position,float yaw)
{
	PlayEffect(key, position, yaw);
}
