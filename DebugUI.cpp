// =============================
// DebugUI/DebugUI.cpp
// =============================
#include "DebugUI.h"
#include "DxPlus/DxPlus.h"

#include <d3d11.h>
#include "DxLib.h"
#include "GameContext.h"
#include "Camera.h"

#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx11.h"

#include <cmath>
#include <algorithm>


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

namespace
{
    // ImGuiに渡すときだけVec3を配列に変換する
    bool EditVector3(const char* label, Vec3& value, float speed,
        float minimum = 0.0f, float maximum = 0.0f,
        ImGuiSliderFlags flags = ImGuiSliderFlags_None)
    {
        float components[3] = { value.x, value.y, value.z };
        if (!ImGui::DragFloat3(label, components, speed, minimum, maximum, "%.3f", flags))
            return false;
        value = { components[0], components[1], components[2] };
        return true;
    }
}

void DebugUI::Draw(GameContext& ctx)
{
    // オプション設定
    {
        ImGui::Begin("Option");     // "Option"ウィンドウを開始

        // ゲームシーンの重力を切り替える
        //if (controls.scope == "game")
        //{
        //    bool gravityDisabled = !ctx.IsPlayerGravityEnabled();
        //    if (ImGui::Checkbox(u8"重力を無効化", &gravityDisabled))
        //        ctx.SetPlayerGravityEnabled(!gravityDisabled);
        //}

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

    /*
    if (controls.scope == "game")
    {
        Player& player = ctx.GetPlayer();
        ImGui::Begin("Lunging Settings");
        float moveSpeed = player.GetLungingMoveSpeed();
        float jumpSpeed = player.GetLungingJumpSpeed();
        if (ImGui::DragFloat(u8"移動速度", &moveSpeed, 10.0f, 0.0f, 5000.0f, "%.0f", ImGuiSliderFlags_AlwaysClamp))
            player.SetLungingMoveSpeed(moveSpeed);
        if (ImGui::DragFloat(u8"ジャンプ初速", &jumpSpeed, 10.0f, 0.0f, 5000.0f, "%.0f", ImGuiSliderFlags_AlwaysClamp))
            player.SetLungingJumpSpeed(jumpSpeed);
        if (ImGui::Button(u8"初期値に戻す"))
            player.ResetLungingSpeeds();
        ImGui::TextWrapped(u8"移動速度は即時反映、ジャンプ初速は次のLungingから反映されます。");
        ImGui::TextWrapped(u8"設定は実行中のみ有効です。");
        ImGui::End();
    }
    // 必殺ゲージの仮調整
    if (controls.scope == "game")
    {
        ImGui::Begin("Ultimate debug");
        float gauge = ctx.GetPlayerUltimateGauge();
        ImGui::ProgressBar(gauge / Player::UltimateGaugeMax, ImVec2(-1.0f, 0.0f));
        if (ImGui::SliderFloat(u8"仮ゲージ", &gauge, 0.0f, Player::UltimateGaugeMax, "%.0f"))
            ctx.SetPlayerUltimateGauge(gauge);
        if (ImGui::Button(u8"100にする"))
            ctx.SetPlayerUltimateGauge(Player::UltimateGaugeMax);
        ImGui::TextUnformatted(u8"100で右クリック：必殺技 / 発動後は0");
        ImGui::TextUnformatted(u8"増加方法は未定（現在は手動調整）");
        ImGui::End();
    }
    */
   
   
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
