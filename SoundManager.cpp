#include "SoundManager.h"
#include "Volume.h"
#include "Sound3D.h"

void SoundManager::PlayBGM(int handle)
{
    V().PlayBGM(handle);
}

void SoundManager::PlaySE(int handle)
{
    V().PlaySE(handle);
}

void SoundManager::PlaySE3D(int handle, Vec3 position, float radius)
{
    if (handle < 0) return;

    float bgm, se;
    V().GetVolume(bgm, se);

    ChangeVolumeSoundMem(static_cast<int>(se * 255.0f), handle);

    Sound3D::Play(handle, position, radius);
}

void SoundManager::StopBGM(int handle)
{
    V().StopBGM(handle);
}

void SoundManager::StopSE(int handle)
{
    V().StopSE(handle);
}

void SoundManager::SetListener(Vec3 position, Vec3 front)
{
    Sound3D::SetListener(position, front);
}