// ============================================================================
// OIC教材用モジュール - 大阪情報コンピュータ専門学校
// 作成者：Y.Tanaka
// このファイルは授業用教材として作成されています。
// ============================================================================

#include "Input.h"
#include "DxLib.h"
#include "DxWrapper.h"
#include "InputManager.h"

namespace
{
    DxPlus::Vec2Int pendingMouseDelta{};
    DxPlus::Vec2Int mouseDelta{};

    bool mouseCaptured = false;
#ifdef _DEBUG
    bool mouseCaptureEnabled = false;
#else
    bool mouseCaptureEnabled = true;
#endif
}

namespace DxPlus::Input
{
    void Initialize()
    {
        DxWrapper::GetInstance().GetInputManager().Initialize();
        DxLib::SetMouseDispFlag(FALSE);

        {
            RAWINPUTDEVICE device{};

            device.usUsagePage = 0x01;   // Generic Desktop
            device.usUsage = 0x02;   // Mouse
            device.dwFlags = 0;
            device.hwndTarget = DxLib::GetMainWindowHandle();

            if (!RegisterRawInputDevices(
                &device,
                1,
                sizeof(device)))
            {
                OutputDebugStringW(
                    L"RegisterRawInputDevices failed.\n");
            }
        }
    }

    void Update()
    {
        // このフレームで使う移動量を確定
        mouseDelta = pendingMouseDelta;

        // 次フレーム分を蓄積するためリセット
        pendingMouseDelta = {};

        DxWrapper::GetInstance().GetInputManager().Update();

        const HWND hwnd = DxLib::GetMainWindowHandle();
        const bool active = hwnd != nullptr && GetForegroundWindow() == hwnd;
        SetMouseCapture(mouseCaptureEnabled && active && !DxLib::GetMouseDispFlag());
    }

    void HandleRawInput(LPARAM lParam)
    {
        RAWINPUT raw{};
        UINT size = sizeof(raw);

        if (GetRawInputData(
            reinterpret_cast<HRAWINPUT>(lParam),
            RID_INPUT,
            &raw,
            &size,
            sizeof(RAWINPUTHEADER)) == static_cast<UINT>(-1))
        {
            return;
        }

        if (raw.header.dwType != RIM_TYPEMOUSE)
        {
            return;
        }

        // 普通のマウスは相対移動
        if ((raw.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE) == 0)
        {
            pendingMouseDelta.x +=
                raw.data.mouse.lLastX;

            pendingMouseDelta.y +=
                raw.data.mouse.lLastY;
        }
    }

    Vec2Int GetMouseDelta()
    {
        return mouseDelta;
    }

    void SetMouseCapture(bool capture)
    {
        // 状態が変わっていなければ何もしない
        if (mouseCaptured == capture)
            return;

        if (!capture)
        {
            // 拘束解除
            ClipCursor(nullptr);

            mouseCaptured = false;
            return;
        }

        const HWND hwnd = DxLib::GetMainWindowHandle();
        if (hwnd == nullptr)
            return;

        // ゲームのクライアント領域を取得
        RECT rect{};
        GetClientRect(hwnd, &rect);

        POINT leftTop{
            rect.left,
            rect.top
        };

        POINT rightBottom{
            rect.right,
            rect.bottom
        };

        // Client座標 → Screen座標
        ClientToScreen(hwnd, &leftTop);
        ClientToScreen(hwnd, &rightBottom);

        RECT clipRect{
            leftTop.x,
            leftTop.y,
            rightBottom.x,
            rightBottom.y
        };

        // ゲーム画面内に拘束
        ClipCursor(&clipRect);

        mouseCaptured = true;
    }

    void SetMouseCaptureEnabled(bool enabled)
    {
        mouseCaptureEnabled = enabled;
    }

    int GetButton(int playerIndex)
    {
        return DxWrapper::GetInstance().GetInputManager().GetButton(playerIndex);
    }

    int GetButtonDown(int playerIndex)
    {
        return DxWrapper::GetInstance().GetInputManager().GetButtonDown(playerIndex);
    }

    int GetButtonUp(int playerIndex)
    {
        return DxWrapper::GetInstance().GetInputManager().GetButtonUp(playerIndex);
    }

    Vec2 GetLStick(int playerIndex)
    {
        return DxWrapper::GetInstance().GetInputManager().GetLeftStick(playerIndex);
    }

    Vec2 GetRStick(int playerIndex)
    {
        return DxWrapper::GetInstance().GetInputManager().GetRightStick(playerIndex);
    }

    float GetLTrigger(int playerIndex)
    {
        return DxWrapper::GetInstance().GetInputManager().GetLeftTrigger(playerIndex);
    }

    float GetRTrigger(int playerIndex)
    {
        return DxWrapper::GetInstance().GetInputManager().GetRightTrigger(playerIndex);
    }

    Vec2Int GetMousePosition()
    {
        int mouseX, mouseY;
        DxLib::GetMousePoint(&mouseX, &mouseY);
        return Vector2<int>(mouseX, mouseY);
    }
    Vec2 GetMousePositionF()
    {
        int mouseX, mouseY;
        DxLib::GetMousePoint(&mouseX, &mouseY);
        return Vector2<float>(static_cast<float>(mouseX), static_cast<float>(mouseY));
    }
}
