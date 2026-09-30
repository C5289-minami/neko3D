// =============================
// Core/GameContext.cpp
// =============================
#include "GameContext.h"
#include "ResourceManager.h"
#include "ResourceKeys.h"
#include "DxConv.h"
#include "Consts.h"

void GameContext::Init()
{
    // 各オブジェクトの初期化
    stage.Init();
	player.Init();
}

void GameContext::Reset()
{
    // 描画先をバックバッファに指定
    DxLib::SetDrawScreen(DX_SCREEN_BACK);

    // 背景色を設定
    DxLib::SetBackgroundColor(bgRed, bgGreen, bgBlue);

    // ライトの向きを設定
    Vec3 lightDir{ -0.3f, -1.0f, -0.5f };
    DxLib::SetLightDirection(DxConv::ToVECTOR(lightDir));
	// 環境光の設定
	DxLib::SetGlobalAmbientLight(DxLib::GetColorF(0.35f, 0.35f, 0.35f,1.0f));

    // カメラを初期状態に戻す
    orbitCamera.Reset();
    wasOrbitControl = false;

    // 各オブジェクトを初期状態に戻す
    stage.Reset();
	player.Reset();
}

void GameContext::Update(float deltaTime)
{
    // 背景色を反映
    DxLib::SetBackgroundColor(bgRed, bgGreen, bgBlue);

    using namespace DxPlus::Input;

    // PLAYER2のR1ボタンを押している間はOrbitCameraを操作する
    const bool orbitControl = (GetButton(PLAYER2) & BUTTON_R1) != 0;

    // OrbitCamera操作を始めた瞬間に、現在の視点情報を渡す
    if (orbitControl && !wasOrbitControl)
    {
        orbitCamera.BeginControl();
        orbitCamera.SetFromLookAt(eye, target);
    }

    // OrbitCamera操作中はプレイヤー更新を行わない
    if (orbitControl)
    {
        orbitCamera.Update(deltaTime);
        wasOrbitControl = true;
        return;
    }

    wasOrbitControl = false;

	// プレイヤーの更新
	player.Update(deltaTime, stage);
}

void GameContext::Draw() const
{
    // OrbitCamera操作中かどうかでカメラ設定を切り替える
    if (wasOrbitControl)
    {
        Vec3 e = orbitCamera.GetEye();
        Vec3 t = orbitCamera.GetTarget();
        Vec3 u = orbitCamera.GetUp();

        DxLib::SetCameraPositionAndTargetAndUpVec(
            DxConv::ToVECTOR(e),
            DxConv::ToVECTOR(t),
            DxConv::ToVECTOR(u)
        );
    }
    else
    {
        DxLib::SetCameraPositionAndTarget_UpVecY(
            DxConv::ToVECTOR(eye),
            DxConv::ToVECTOR(target)
        );
    }

    // 画面をクリア
    DxLib::ClearDrawScreen();

    // 各オブジェクトを描画
    grid.Draw();
    stage.Draw();
	player.Draw();
}
