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

#include <cmath>

#include "ToonSettings.h"

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

	// トゥーン設定を読み込む
    {
        ToonSettingsManager::Load();
    }
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

void DebugUI::DrawTransformEditor(const DebugSceneControls& controls)
{
    if (transformScope_ != controls.scope)
    {
        transformScope_ = controls.scope;
        transformStatus_.clear();
        selectedTargetId_ = controls.targets.empty() ? "" : controls.targets.front().id;
        cameraSelected_ = false;
    }

    ImGui::SetNextWindowSize(ImVec2(260.0f, 400.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("Hierarchy");
    if (controls.targets.empty())
        ImGui::TextWrapped(u8"このシーンには編集対象がありません。");
    if (controls.isPaused && controls.setPaused)
    {
        bool paused = controls.isPaused();
        if (ImGui::Checkbox(u8"オブジェクトの更新を停止", &paused))
            controls.setPaused(paused);
    }
    ImGui::Separator();
    for (const auto& target : controls.targets)
    {
        ImGui::PushID(target.id.c_str());
        if (ImGui::Selectable(target.name.c_str(), !cameraSelected_ && selectedTargetId_ == target.id))
        {
            selectedTargetId_ = target.id;
            cameraSelected_ = false;
        }
        ImGui::PopID();
    }
    if (controls.camera && ImGui::Selectable("Camera", cameraSelected_))
        cameraSelected_ = true;

    if (!controls.targets.empty())
    {
        ImGui::Separator();
        if (ImGui::Button(u8"シーンをソースに保存"))
            TransformSettings::Save(controls.targets, transformStatus_);
        if (ImGui::Button(u8"シーンの初期値に戻す"))
            TransformSettings::ResetToDefaults(controls.targets, transformStatus_);
    }
    ImGui::End();

    ImGui::SetNextWindowSize(ImVec2(520.0f, 440.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("Inspector");
    if (cameraSelected_ && controls.camera)
    {
        const auto& camera = *controls.camera;
        ImGui::Text("Camera: %s", camera.isActive() ? "Debug (Alt + Enter)" : "Normal");
        Vec3 position = camera.getPosition();
        if (EditVector3("Position", position, 1.0f) &&
            std::isfinite(position.x) && std::isfinite(position.y) && std::isfinite(position.z))
            camera.setPosition(position);
        if (ImGui::Button("Reset camera")) camera.reset();
        ImGui::SameLine();
        if (ImGui::Button("Focus player")) camera.focusPlayer();
    }
    else
    {
        const DebugTransformTarget* selected = nullptr;
        for (const auto& target : controls.targets)
            if (target.id == selectedTargetId_) { selected = &target; break; }

        if (selected)
        {
            ImGui::TextUnformatted(selected->name.c_str());
            auto transform = selected->get();
            ImGui::Separator();
            bool changed = EditVector3("Position", transform.position, 1.0f);
            changed |= EditVector3("Rotation (rad)", transform.rotation, 0.01f);
            changed |= EditVector3("Scale", transform.scale, 0.1f, 0.001f, 100000.0f,
                ImGuiSliderFlags_AlwaysClamp);
            if (changed)
            {
                if (TransformSettings::IsValid(transform)) selected->set(transform);
                else transformStatus_ = "Invalid transform: use finite values and positive scale.";
            }
            if (ImGui::Button(u8"ソースに保存")) TransformSettings::Save({ *selected }, transformStatus_);
            ImGui::SameLine();
            if (ImGui::Button(u8"初期値に戻す")) TransformSettings::ResetToDefaults({ *selected }, transformStatus_);
        }
        else ImGui::TextDisabled(u8"Hierarchyから対象を選択してください。");
    }
    if (controls.camera && controls.camera->isActive())
        ImGui::TextWrapped(u8"デバッグカメラ中も編集・保存できます。オブジェクトの更新は自動停止します。");
    ImGui::Separator();
    ImGui::TextWrapped("Source: %s", TransformSettings::SourceFilePath().c_str());
    ImGui::TextWrapped(u8"保存した初期値は、再ビルドして次回起動すると反映されます。");
    const auto& status = transformStatus_.empty() ? controls.initialStatus : transformStatus_;
    if (!status.empty()) ImGui::TextWrapped("%s", status.c_str());
    ImGui::End();
}

void DebugUI::Draw(GameContext& ctx, const DebugSceneControls& controls)
{
    DrawTransformEditor(controls);
    // オプション設定
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

	// トゥーン設定
    {
        ImGui::Begin("Toon Settings");

        ImGui::ColorEdit3(
            u8"影色",
            g_toonSettings.shadowColor
        );

        ImGui::SliderFloat(
            u8"明るい境界",
            &g_toonSettings.thresholds[0],
            0.0f, 1.0f
        );

        ImGui::SliderFloat(
            u8"通常境界",
            &g_toonSettings.thresholds[1],
            0.0f, 1.0f
        );

        ImGui::SliderFloat(
            u8"暗い境界",
            &g_toonSettings.thresholds[2],
            0.0f, 1.0f
        );

        ImGui::SliderFloat(
            u8"影の暗さ",
            &g_toonSettings.shadow[0],
            0.0f, 1.0f
        );

        ImGui::SliderFloat(
            u8"影色の強さ",
            &g_toonSettings.shadow[1],
            0.0f, 1.0f
        );

        if (ImGui::Button(u8"保存"))
        {
            ToonSettingsManager::Save();
        }

        ImGui::SameLine();

        if (ImGui::Button(u8"読み込み"))
        {
            ToonSettingsManager::Load();
        }
        ImGui::SameLine();
        if (ImGui::Button(u8"初期値に戻す"))
        {
            ToonSettingsManager::Reset();
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
