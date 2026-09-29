// =============================
// Gameplay/Debug/Grid.cpp
// =============================
#include "Grid.h"
#include "DxLib.h"
#include "DxConv.h"

void Grid::Draw() const
{
    const float size = halfCount * spacing;

    for (int i = -halfCount; i <= halfCount; ++i)
    {
        const float x = i * spacing;
        const float y = i * spacing;
        const float z = i * spacing;

        if (IsDrawXY())
        {
            DxLib::DrawLine3D(
                DxConv::ToVECTOR({ -size, y, 0 }), 
                DxConv::ToVECTOR({  size, y, 0 }), 
                DxLib::GetColor(255,255,255));

            DxLib::DrawLine3D(
                DxConv::ToVECTOR({ x, -size, 0 }),
                DxConv::ToVECTOR({ x,  size, 0 }),
                DxLib::GetColor(255,255,255)
            );
        }

        if (IsDrawYZ())
        {
            DxLib::DrawLine3D(
                DxConv::ToVECTOR({ 0, -size, z }),
                DxConv::ToVECTOR({ 0,  size, z }),
                DxLib::GetColor(255,255,255)
            );

            DxLib::DrawLine3D(
                DxConv::ToVECTOR({ 0, y, -size }),
                DxConv::ToVECTOR({ 0, y,  size }),
                DxLib::GetColor(255,255,255)
            );
        }

        if (IsDrawZX())
        {
            DxLib::DrawLine3D(
                DxConv::ToVECTOR({ -size, 0, z }),
                DxConv::ToVECTOR({ size, 0, z }),
                DxLib::GetColor(255, 255, 255)
            );

            DxLib::DrawLine3D(
                DxConv::ToVECTOR({ x, 0, -size }),
                DxConv::ToVECTOR({ x, 0,  size }),
                DxLib::GetColor(255, 255, 255)
            );
        }
    }

    // 3DÀ•WŽ²‚ð•`‰æ
    DxLib::DrawLine3D(DxConv::ToVECTOR({ 0, 0, 0 }), DxConv::ToVECTOR({ size, 0, 0 }), GetColor(255, 0, 0));
    DxLib::DrawLine3D(DxConv::ToVECTOR({ 0, 0, 0 }), DxConv::ToVECTOR({ 0, size, 0 }), GetColor(0, 255, 0));
    DxLib::DrawLine3D(DxConv::ToVECTOR({ 0, 0, 0 }), DxConv::ToVECTOR({ 0, 0, size }), GetColor(0, 0, 255));
}
