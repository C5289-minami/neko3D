#include "SoundManager.h"
#include "Volume.h"
#include "Sound3D.h"

void SoundManager::PlayBGM(int handle, int volume)
{
    V().PlayBGM(handle, volume);
}

void SoundManager::PlaySE(int handle, int volume)
{
    V().PlaySE(handle, volume);
}

void SoundManager::PlaySE3D(int handle, Vec3 position, float radius, int startTimeMs)
{
    if (handle < 0) return;

    float bgm, se;
    V().GetVolume(bgm, se);

    ChangeVolumeSoundMem(static_cast<int>(se * Const::MAX_VOICE), handle);

    Sound3D::Play(handle, position, radius, startTimeMs);
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