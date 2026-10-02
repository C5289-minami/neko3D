// =============================
// 15_System/Player_camera.cpp
// =============================
#include "Player_camera.h"

void Player_camera::Reset()
{
    eye = { 400.0f, 400.0f, 400.0f };
    target = { 0.0f, 0.0f, 0.0f };
    up = { 0.0f, 1.0f, 0.0f };
}
