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
        ImGui::TextUnformatted("Normal Camera Settings");
        float fieldOfView = Camera::Camera::GetFieldOfView();
        if (ImGui::SliderFloat(u8"視野角 (度)", &fieldOfView, 10.0f, 150.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp))
            Camera::Camera::SetFieldOfView(fieldOfView);
        if (ImGui::Button(u8"視野角を初期値に戻す"))
            Camera::Camera::SetFieldOfView(Camera::Camera::DefaultFieldOfView);
        ImGui::TextWrapped(u8"通常・デバッグカメラに共通で反映されます。設定は実行中のみ有効です。");
        Vec3 position = camera.getPosition();
        if (EditVector3(u8"位置 X / Y / Z", position, 1.0f) &&
            std::isfinite(position.x) && std::isfinite(position.y) && std::isfinite(position.z))
            camera.setPosition(position);
        if (camera.getAnglesDegrees && camera.setAnglesDegrees)
        {
            Vec3 angles = camera.getAnglesDegrees();
            if (EditVector3(camera.followsTarget ? u8"角度オフセット X / Y / Z (度)" : u8"アングル X / Y / Z (度)", angles, 0.5f))
                camera.setAnglesDegrees(angles);
            ImGui::TextWrapped(u8"X: 上下 / Y: 左右 / Z: 傾き。通常カメラの設定を変更します。" );
        }
        if (camera.followsTarget)
            ImGui::TextWrapped(u8"位置はワールド座標で調整します。追従を続けながら位置と角度を調整できます。Reset cameraで初期化します。" );
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
            float uniformScale = transform.scale.x;
            if (ImGui::DragFloat("Uniform scale", &uniformScale, 0.01f, 0.001f, 100000.0f,
                "%.3f", ImGuiSliderFlags_AlwaysClamp))
            {
                transform.scale = transform.scale * (uniformScale / transform.scale.x);
                changed = true;
            }
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
    if (controls.drawAdditional) controls.drawAdditional();
    // オプション設定
    {
        ImGui::Begin("Option");     // "Option"ウィンドウを開始

        // ゲームシーンの重力を切り替える
        if (controls.scope == "game")
        {
            bool gravityDisabled = !ctx.IsPlayerGravityEnabled();
            if (ImGui::Checkbox(u8"重力を無効化", &gravityDisabled))
                ctx.SetPlayerGravityEnabled(!gravityDisabled);
        }

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
