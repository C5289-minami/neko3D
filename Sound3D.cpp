#include "Sound3D.h"
#include "DxConv.h"

void Sound3D::Play(int soundHandle, Vec3 position, float radius, int startTimeMs)
{
    // 音源の3D座標を設定
    SetPosition(soundHandle, position);

    Set3DRadiusSoundMem(radius, soundHandle);
    const bool playFromStart = startTimeMs <= 0 || SetSoundCurrentTime(startTimeMs, soundHandle) != 0;

    // 再生
    PlaySoundMem(
        soundHandle,
        DX_PLAYTYPE_BACK,
        playFromStart
    );
}

void Sound3D::SetPosition(int soundHandle, Vec3 position)
{
    if (soundHandle < 0) return;
    Set3DPositionSoundMem(DxConv::ToVECTOR(position), soundHandle);
}

void Sound3D::SetListener(
    Vec3 position,
    Vec3 front
)
{
    Set3DSoundListenerPosAndFrontPos_UpVecY(
        DxConv::ToVECTOR(position),
        DxConv::ToVECTOR(position + front)
    );
}