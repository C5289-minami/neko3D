// ============================================================================
// OIC教材用モジュール - 大阪情報コンピュータ専門学校
// 作成者：Y.Tanaka
// このファイルは授業用教材として作成されています。
// ============================================================================

#include "DxWrapper.h"
#include <string>
#include <mmsystem.h>
#include "DxLib.h"
#include "Utils.h"
#pragma comment(lib, "winmm.lib")

namespace DxPlus
{
    DxWrapper::DxWrapper()
        :inputManager(std::make_unique<InputManager>())
    {
    }

    int DxWrapper::Initialize(int clientWidth, int clientHeight, bool bWindowed)
    {
        DxLib::SetUseCharCodeFormat(DX_CHARCODEFORMAT_UTF8);       // 使用する文字コードを utf8 に設定
        DxLib::SetGraphMode(clientWidth, clientHeight, 32);
        DxLib::ChangeWindowMode(bWindowed);
        DxLib::SetUseDXArchiveFlag(FALSE);
        DxLib::SetUse3DFlag(TRUE);
        DxLib::SetAlwaysRunFlag(TRUE);
        DxLib::SetWaitVSyncFlag(enableVSync);
        fpsCap = enableVSync ? 0 : fpsCap;

        // DirectX 11を使用する設定に変更
        DxLib::SetUseDirect3DVersion(DX_DIRECT3D_11);
        DxLib::SetUseTSFFlag(TRUE);

        DxLib::SetBackgroundColor(0, 0, 0);

        if (DxLib::DxLib_Init() == -1)
        {
            DxPlus::Utils::FatalError(L"DxLib initialization failed!");
        }

        // フルスクリーンとウィンドウの切り替え時にリソースが消えないようにする
        DxLib::SetChangeScreenModeGraphicsSystemResetFlag(FALSE);

        // Zバッファを有効にする
        DxLib::SetUseZBuffer3D(TRUE);

        // Zバッファへの書き込みを有効にする
        DxLib::SetWriteZBuffer3D(TRUE);

        DxLib::SetWindowText(L"DxPlus");
        DxLib::SetDrawScreen(DX_SCREEN_BACK);

        timeBeginPeriod(1);

        return 0;
    }

    bool DxWrapper::GameLoop(bool showFps)
    {
        // DxLibのメッセージ処理（WM_QUIT相当）
        if (DxLib::ProcessMessage() != 0) return false;

        // 余裕がある時だけ fpsCap を超えないよう待つ
        timer.Tick(static_cast<double>(fpsCap));

        // フレームレートをタイトルバーに表示
        if (showFps) DrawFrameStats();

        return true; // フレーム実行可能
    }

    void DxWrapper::Finalize()
    {
        timeEndPeriod(1);
        DxLib::DxLib_End();
    }

    void DxWrapper::DrawFrameStats()
    {
        static double elapsed = 0.0;

        elapsed += timer.GetRawDeltaTime();

        if (elapsed < 0.25)
            return;

        elapsed = 0.0;

        double fps = 1.0 / timer.GetRawDeltaTime();

        std::wstring title =
            L"DxPlus - FPS: " +
            std::to_wstring(static_cast<int>(fps));

        DxLib::SetWindowText(title.c_str());
    }
}
