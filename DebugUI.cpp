// =============================
// DebugUI/DebugUI.cpp
// =============================
#include "DebugUI.h"
#include "DxPlus/DxPlus.h"

#include <d3d11.h>
#include "DxLib.h"
#include "../GameContext.h"

#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx11.h"

#include <filesystem>

extern bool g_raise_imgui_viewports;

void DebugUI::Init()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    // ===== 日本語フォント追加 =====
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = "./Data/Config/imgui.ini";
    //io.IniFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;   // ←これがDocking ON
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    io.Fonts->AddFontFromFileTTF(
        "./Data/Fonts/Noto_Sans_JP/static/NotoSansJP-Regular.ttf", // ここは置いたパスに合わせる
        18.0f,
        nullptr,
        io.Fonts->GetGlyphRangesJapanese()
    );
    // ===========================

    ImGui_ImplWin32_Init(GetMainWindowHandle());
    auto* device = reinterpret_cast<ID3D11Device*>(const_cast<void*>(DxLib::GetUseDirect3D11Device()));
    auto* context = reinterpret_cast<ID3D11DeviceContext*>(const_cast<void*>(DxLib::GetUseDirect3D11DeviceContext()));
    ImGui_ImplDX11_Init(device, context);

    ImGui::LoadIniSettingsFromDisk("./Data/Config/imgui.ini");
}

void DebugUI::Shutdown()
{
    ImGui::SaveIniSettingsToDisk("./Data/Config/imgui.ini");
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void DebugUI::BeginFrame()
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGuiDockNodeFlags flags = ImGuiDockNodeFlags_PassthruCentralNode;
    ImGui::DockSpaceOverViewport(ImGui::GetID("MainDockSpace"), ImGui::GetMainViewport(), flags);
}

void DebugUI::Draw(GameContext& ctx)
{
    ImGui::Begin("Player debug");
    const Vec3& playerPosition = ctx.GetPlayerPosition();
    float position[3] = { playerPosition.x, playerPosition.y, playerPosition.z };
    if (ImGui::DragFloat3("Position (X, Y, Z)", position, 1.0f, 0.0f, 0.0f, "%.1f"))
        ctx.SetPlayerPosition({ position[0], position[1], position[2] });
    ImGui::End();

    // Option
    {
        ImGui::Begin("Option");     // "Option"ウィンドウを開始

        // 背景色
        {
            float colors[3] = {
                ctx.GetBgRed() / 255.0f,
                ctx.GetBgGreen() / 255.0f,
                ctx.GetBgBlue() / 255.0f
            };
            if (ImGui::ColorEdit3(u8"背景色", colors))
            {
                ctx.SetBgRed(static_cast<int>(colors[0] * 255.0f));
                ctx.SetBgGreen(static_cast<int>(colors[1] * 255.0f));
                ctx.SetBgBlue(static_cast<int>(colors[2] * 255.0f));
            }
        }

        // グリッド
        if (ImGui::TreeNode("Grids"))
        {
            Grid& grid = ctx.GetGrid();

            // XY平面
            {
                bool drawXY = grid.IsDrawXY();
                if (ImGui::Checkbox(u8"XY平面", &drawXY))
                {
                    grid.SetDrawXY(drawXY);
                }
            }

            // YZ平面
            {
                bool drawYZ = grid.IsDrawYZ();
                if (ImGui::Checkbox(u8"YZ平面", &drawYZ))
                {
                    grid.SetDrawYZ(drawYZ);
                }
            }

            // ZX平面
            {
                bool drawZX = grid.IsDrawZX();
                if (ImGui::Checkbox(u8"ZX平面", &drawZX))
                {
                    grid.SetDrawZX(drawZX);
                }
            }

            // HalfCount
            {
                int halfCount = grid.GetHalfCount();
                ImGui::Text("HalfCount");
                ImGui::SameLine();
                if (ImGui::DragInt("##HalfCount", &halfCount))
                {
                    grid.SetHalfCount(halfCount);
                }
            }

            // Spacing
            {
                float spacing = grid.GetSpacing();
                ImGui::Text("Spacing");
                ImGui::SameLine();
                if (ImGui::DragFloat("##Spacing", &spacing))
                {
                    grid.SetSpacing(spacing);
                }
            }

            ImGui::TreePop();
        }

        // フレームレート調節
        {
            int fpsCap = DxPlus::DxWrapper::GetInstance().GetFpsCap();
            ImGui::Text(u8"フレームレート調節");
            ImGui::SameLine();
            if (ImGui::SliderInt("##フレームレート調節", &fpsCap, 30, 480))
            {
                DxPlus::DxWrapper::GetInstance().SetFpsCap(fpsCap);
            }
        }

        ImGui::End();
    }
}

void DebugUI::EndFrame()
{
    ImGuiIO& io = ImGui::GetIO();

    auto* context = reinterpret_cast<ID3D11DeviceContext*>(const_cast<void*>(DxLib::GetUseDirect3D11DeviceContext()));

    ID3D11RenderTargetView* oldRTV = nullptr;
    ID3D11DepthStencilView* oldDSV = nullptr;
    D3D11_VIEWPORT oldVP{};
    UINT vpCount = 1;

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        context->OMGetRenderTargets(1, &oldRTV, &oldDSV);
        context->RSGetViewports(&vpCount, &oldVP);
    }

    // メインウィンドウのImGui描画
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    // 外窓
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();

        // Viewport窓を前面へ（フォーカスは奪わない）
        if (g_raise_imgui_viewports)
        {
            ImGuiPlatformIO& pio = ImGui::GetPlatformIO();
            for (int i = 0; i < pio.Viewports.Size; i++)
            {
                ImGuiViewport* vp = pio.Viewports[i];
                if (vp == ImGui::GetMainViewport()) continue;

                HWND hwnd = (HWND)vp->PlatformHandleRaw;
                if (!hwnd) continue;

                ::ShowWindow(hwnd, SW_SHOWNOACTIVATE);
                ::SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0,
                    SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
            }
            g_raise_imgui_viewports = false;
        }

        // 戻す（DxLibが期待するメインターゲットへ）
        context->OMSetRenderTargets(1, &oldRTV, oldDSV);
        context->RSSetViewports(1, &oldVP);

        if (oldRTV) oldRTV->Release();
        if (oldDSV) oldDSV->Release();
    }
}
