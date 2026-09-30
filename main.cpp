// =============================
// App/main.cpp
// =============================
#include <crtdbg.h>
#include "DxPlus/DxPlus.h"
#include "SceneManager.h"

#include "imgui.h"
#include "backends/imgui_impl_win32.h"

#include "./SSPlayer/SS6Player.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND hWnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam
);

bool g_raise_imgui_viewports = false;

static LRESULT CALLBACK CustomWinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_INPUT)
    {
        DxPlus::Input::HandleRawInput(lParam);
    }

	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
	{
		return 1; // ImGuiが処理した
	}

	if (msg == WM_ACTIVATEAPP && wParam == TRUE)
		g_raise_imgui_viewports = true;

	if (msg == WM_KEYDOWN && wParam == VK_ESCAPE)
	{
		PostQuitMessage(0);
		return 1;
	}

	return 0;
}

int WINAPI wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int)
 {
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF);
    srand((unsigned int)time(NULL));
	DxLib::SetHookWinProc(CustomWinProc);
	DxLib::SetWindowStyleMode(7);
	DxLib::SetWindowSizeChangeEnableFlag(TRUE,TRUE);

    SceneManager::RunConfig cfg{};
#ifdef NDEBUG
    cfg.enableDebugUI = false;   // ReleaseはDebugUI完全禁止
#else
    cfg.enableDebugUI = true;
#endif

    // ウィンドウモード / フルスクリーンの切り替え
    cfg.windowed = true;

	ss::SSPlatformInit();
	ss::SSSetPlusDirection(ss::PLUS_DOWN, DxPlus::CLIENT_WIDTH, DxPlus::CLIENT_HEIGHT);

    SM().SetRunConfig(cfg);
	SM().Init();
	SM().Run();
	SM().Shutdown();

	return 0;
}
