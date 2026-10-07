#include "Sound3D.h"
#include "DxConv.h"

void Sound3D::Play(int soundHandle, Vec3 position,float radius)
{
    // âπåπÇÃ3Dç¿ïWÇê›íË
    Set3DPositionSoundMem(
		DxConv::ToVECTOR(position),
		soundHandle
    );

    Set3DRadiusSoundMem(radius, soundHandle);

    // çƒê∂
    PlaySoundMem(
        soundHandle,
        DX_PLAYTYPE_BACK
    );
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