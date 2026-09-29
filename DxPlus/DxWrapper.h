// ============================================================================
// OIC教材用モジュール - 大阪情報コンピュータ専門学校
// 作成者：Y.Tanaka
// このファイルは授業用教材として作成されています。
// ============================================================================

#pragma once
#include <memory>
#include "InputManager.h"
#include "HighResolutionTimer.h"

namespace DxPlus
{
    // 前方宣言
    class InputManager;

    class DxWrapper
    {
    private:
        DxWrapper();
        DxWrapper(const DxWrapper&) = delete;               // コピーコンストラクタを削除
        DxWrapper(DxWrapper&&) = delete;                    // ムーブコンストラクタを削除
        DxWrapper& operator=(const DxWrapper&) = delete;    // コピー代入演算子を削除
        DxWrapper& operator=(DxWrapper&&) = delete;         // ムーブ代入演算子を削除
    public:
        static DxWrapper& GetInstance()
        {
            static DxWrapper instance;
            return instance;
        }

        InputManager& GetInputManager() { return *inputManager; }
        HighResolutionTimer& GetHighResolutionTimer() { return timer; }

        int Initialize(int clientWidth, int clientHeight, bool bWindowed);
        bool GameLoop(bool showFps = false);
        void Finalize();

        double GetDeltaTime() const { return timer.GetDeltaTime(); }
        double GetRawDeltaTime() const { return timer.GetRawDeltaTime(); }
        void DrawFrameStats();

        int GetFpsCap() const { return fpsCap; }
        void SetFpsCap(int fps) { fpsCap = fps; }

    private:
        std::unique_ptr<InputManager> inputManager;
        HighResolutionTimer timer;

        double prevTime{ 0.0f };

        BOOL enableVSync{ FALSE };
        int fpsCap{ 60 };
    };
}
