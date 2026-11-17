#include "CatViewerScene.h"

#include "ResourceKeys.h"
#include "ResourceManager.h"
#include "SceneManager.h"
#include "DxConv.h"
#include "EffectManager.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

#ifndef NDEBUG
#include "imgui.h"
#endif

namespace
{
    struct Clip
    {
        const wchar_t* resourceKey;
        const char* label;
    };

    struct EffectPreset
    {
        const wchar_t* resourceKey;
        const char* label;
        bool compoundDust;
    };
    const std::array<EffectPreset, 7> EffectPresets{ {
        { ResourceKeys::Effect_WalkDust, u8"ウォーク砂ぼこり・単体 (suna)", false },
        { ResourceKeys::Effect_WalkDust, u8"ウォーク砂ぼこり・仮の複数生成", true },
        { ResourceKeys::Effect_Blink, u8"ブリンク (blink)", false },
        { ResourceKeys::Effect_BossTailWhip, u8"ボス尻尾 (boss_sippo)", false },
        { ResourceKeys::Effect_BossSlam, u8"ボス振り下ろし (boss_tataku)", false },
        { ResourceKeys::Effect_Impact, u8"衝撃波 (wave)", false },
        { ResourceKeys::Effect_Kedama, u8"毛玉 (kedama)", false }
    } };
    constexpr std::size_t MaxPreviewEffects = 128;

    const std::array<Clip, 8> Clips{ {
        { ResourceKeys::Model_PlayerAnim_Idle, u8"待機" },
        { ResourceKeys::Model_PlayerAnim_Walk, u8"歩く" },
        { ResourceKeys::Model_PlayerAnim_Jump, u8"ジャンプ" },
        { ResourceKeys::Model_PlayerAnim_Dodge, u8"回避・二段ジャンプ" },
        { ResourceKeys::Model_PlayerAnim_Attack, u8"攻撃" },
        { ResourceKeys::Model_PlayerAnim_Ult, u8"必殺技" },
        { ResourceKeys::Model_PlayerAnim_SelectMike, u8"ミケの選択モーション" },
        { ResourceKeys::Model_PlayerAnim_SelectSabi, u8"サビの選択モーション" }
    } };
}

void CatViewerScene::Init()
{
    End();
    clipIndex_ = 0;
    playing_ = true;
    loop_ = true;
    playbackSpeed_ = 1.0f;
    showGrid_ = true;
    showCat_ = true;
    effectsPaused_ = false;
    ResetView();
    SetModelKey(modelKey_.empty() ? ResourceKeys::Model_Player.mike : modelKey_);
    DxLib::SetBackgroundColor(42, 46, 55);
    UpdatePresentationView(0.0f);
    StartFadeIn(0.0f);
}

bool CatViewerScene::SetModelKey(const std::wstring& modelKey)
{
    const int source = RM().GetModel(modelKey);
    const int duplicate = source >= 0 ? DxLib::MV1DuplicateModel(source) : -1;
    if (duplicate < 0)
    {
        status_ = u8"モデルを読み込めませんでした。";
        return false;
    }

    const float time = animation_.GetCurrentTime();
    // ModelAnimation must release its attachment before deleting its MV1 owner.
    animation_.Reset();
    if (modelHandle_ >= 0) DxLib::MV1DeleteModel(modelHandle_);
    modelHandle_ = duplicate;
    modelKey_ = modelKey;
    FitDisplayModel();
    AttachClip(time);
    return true;
}

void CatViewerScene::FitDisplayModel()
{
    if (modelHandle_ < 0) return;
    Vec3 minimum{ std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
    Vec3 maximum{ -minimum.x, -minimum.y, -minimum.z };
    const int meshes = DxLib::MV1GetMeshNum(modelHandle_);
    for (int i = 0; i < meshes; ++i)
    {
        const auto low = DxLib::MV1GetMeshMinPosition(modelHandle_, i);
        const auto high = DxLib::MV1GetMeshMaxPosition(modelHandle_, i);
        minimum.x = (std::min)(minimum.x, low.x);
        minimum.y = (std::min)(minimum.y, low.y);
        minimum.z = (std::min)(minimum.z, low.z);
        maximum.x = (std::max)(maximum.x, high.x);
        maximum.y = (std::max)(maximum.y, high.y);
        maximum.z = (std::max)(maximum.z, high.z);
    }
    const float extent = (std::max)({ maximum.x - minimum.x,
        maximum.y - minimum.y, maximum.z - minimum.z });
    if (meshes <= 0 || !std::isfinite(extent) || extent < 0.001f)
    {
        minimum = { -0.5f, 0.0f, -0.5f };
        maximum = { 0.5f, 1.0f, 0.5f };
    }
    const float size = (std::max)({ maximum.x - minimum.x,
        maximum.y - minimum.y, maximum.z - minimum.z, 0.001f });
    const float scale = 150.0f / size;
    const Vec3 center = (minimum + maximum) * 0.5f;
    DxLib::MV1SetScale(modelHandle_, VGet(scale, scale, scale));
    DxLib::MV1SetRotationXYZ(modelHandle_, VGet(0, 0, 0));
    DxLib::MV1SetPosition(modelHandle_, VGet(-center.x * scale, -minimum.y * scale, -center.z * scale));
    focusHeight_ = (maximum.y - minimum.y) * scale * 0.5f;
}

float CatViewerScene::GetClipDuration() const
{
    const int source = RM().GetModel(Clips[clipIndex_].resourceKey);
    if (source < 0 || DxLib::MV1GetAnimNum(source) <= 0) return 0.0f;
    const float duration = DxLib::MV1GetAnimTotalTime(source, 0);
    return std::isfinite(duration) ? (std::max)(duration, 0.0f) : 0.0f;
}

void CatViewerScene::AttachClip(float time)
{
    if (modelHandle_ < 0) return;
    const int source = RM().GetModel(Clips[clipIndex_].resourceKey);
    const auto clip = AnimationHandle::FromIndex(source, 0);
    if (!clip)
    {
        status_ = u8"このアニメーションを読み込めませんでした。";
        return;
    }

    // No blend: seeking and stepping must display exactly the selected pose.
    animation_.Reset();
    animation_.Play3D(modelHandle_, clip, loop_, Const::ANIM_FPS * playbackSpeed_, 0.0f, false);
    animation_.SetTime(std::clamp(time, 0.0f, GetClipDuration()));
    if (playing_) animation_.Start(Const::ANIM_FPS * playbackSpeed_);
    status_.clear();
}

void CatViewerScene::Update(float deltaTime)
{
    // deltaTime already includes global pause / slow motion / single stepping.
    animation_.Update(deltaTime);
    if (animation_.IsFinished()) playing_ = false;
    UpdatePreviewEffects(deltaTime);
}

void CatViewerScene::ResetView()
{
    yaw_ = 35.0f;
    pitch_ = 12.0f;
    distance_ = 360.0f;
}

void CatViewerScene::UpdatePresentationView(float realDeltaTime)
{
    (void)realDeltaTime;
#ifndef NDEBUG
    const bool mouseCaptured = ImGui::GetCurrentContext() && ImGui::GetIO().WantCaptureMouse;
    if (!mouseCaptured && (DxLib::GetMouseInput() & MOUSE_INPUT_RIGHT) != 0)
    {
        const auto mouse = DxPlus::Input::GetMouseDelta();
        yaw_ = std::remainder(yaw_ + mouse.x * 0.2f, 360.0f);
        pitch_ = std::clamp(pitch_ + mouse.y * 0.2f, -75.0f, 75.0f);
    }
#endif
    const float yaw = yaw_ * DxPlus::Deg2Rad;
    const float pitch = pitch_ * DxPlus::Deg2Rad;
    const Vec3 target{ 0, focusHeight_, 0 };
    const Vec3 offset{
        std::sin(yaw) * std::cos(pitch), std::sin(pitch), std::cos(yaw) * std::cos(pitch)
    };
    camera_.Initialize(target + offset * distance_, target);
}

void CatViewerScene::Render() const
{
    DxLib::SetUseVertexShader(-1);
    DxLib::SetUsePixelShader(-1);
    DxLib::MV1SetUseOrigShader(FALSE);
    DxLib::SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    DxLib::SetUseZBuffer3D(TRUE);
    DxLib::SetWriteZBuffer3D(TRUE);
    DxLib::SetUseLighting(TRUE);
    DxLib::SetLightDirection(VGet(-0.3f, -1.0f, -0.5f));
    DxLib::SetGlobalAmbientLight(DxLib::GetColorF(0.35f, 0.35f, 0.35f, 1.0f));
    auto pose = camera_.GetPose();
    pose.fieldOfViewDegrees = 45.0f;
    camera_.ApplyPose(pose);
    if (showGrid_)
    {
        const unsigned int color = DxLib::GetColor(95, 105, 120);
        for (int i = -4; i <= 4; ++i)
        {
            const float position = i * 50.0f;
            DxLib::DrawLine3D(VGet(-200, 0, position), VGet(200, 0, position), color);
            DxLib::DrawLine3D(VGet(position, 0, -200), VGet(position, 0, 200), color);
        }
    }
    if (showCat_ && modelHandle_ >= 0) DxLib::MV1DrawModel(modelHandle_);
    gameContext->GetEffectManager().Draw();
}

void CatViewerScene::DrawImGui()
{
#ifndef NDEBUG
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + (std::max)(500.0f, viewport->WorkSize.x - 520.0f),
        viewport->WorkPos.y + 20), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(500, 640), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(u8"猫のアニメーション鑑賞"))
    {
        DrawEffectImGui();
        ImGui::Separator();
        const wchar_t* const modelKeys[] = {
            ResourceKeys::Model_Player.mike, ResourceKeys::Model_Player.sabi, ResourceKeys::Model_Player.kuro
        };
        const char* const catNames[] = { u8"ミケ", u8"サビ", u8"クロ" };
        int catIndex = 0;
        for (int i = 0; i < 3; ++i) if (GetModelKey() == modelKeys[i]) catIndex = i;
        if (ImGui::Combo(u8"猫の種類", &catIndex, catNames, 3)) SetModelKey(modelKeys[catIndex]);
        ImGui::TextWrapped(u8"種類を変更しても、アニメーションの再生位置・再生状態・カメラの角度を維持します。");
        if (ImGui::Button(u8"この猫で実践テストを開始")) SM().RequestBattleTestCat(catIndex);
        ImGui::TextWrapped(u8"このボタンは実践テストにだけ反映されます。本編のキャラクター選択は通常どおりです。");
        ImGui::Separator();
        if (ImGui::BeginCombo(u8"アニメーション", Clips[clipIndex_].label))
        {
            for (int i = 0; i < static_cast<int>(Clips.size()); ++i)
            {
                const int source = RM().GetModel(Clips[i].resourceKey);
                const bool available = source >= 0 && DxLib::MV1GetAnimNum(source) > 0;
                ImGui::BeginDisabled(!available);
                if (ImGui::Selectable(Clips[i].label, clipIndex_ == i))
                {
                    clipIndex_ = i;
                    AttachClip();
                }
                ImGui::EndDisabled();
            }
            ImGui::EndCombo();
        }
        ImGui::BeginDisabled(modelHandle_ < 0 || GetClipDuration() <= 0.0f);
        if (ImGui::Button(playing_ ? u8"一時停止" : u8"再生"))
        {
            playing_ = !playing_;
            if (playing_) animation_.Start(Const::ANIM_FPS * playbackSpeed_);
            else animation_.Pause();
        }
        ImGui::SameLine();
        if (ImGui::Button(u8"先頭から再生"))
        {
            playing_ = true;
            AttachClip();
        }
        if (ImGui::Checkbox(u8"ループ", &loop_)) AttachClip(animation_.GetCurrentTime());
        if (ImGui::SliderFloat(u8"再生速度", &playbackSpeed_, 0.1f, 2.0f, "%.2fx"))
            animation_.SetSpeedScale(Const::ANIM_FPS * playbackSpeed_);
        float time = animation_.GetCurrentTime();
        const float duration = GetClipDuration();
        if (ImGui::SliderFloat(u8"再生位置（操作すると一時停止）", &time, 0.0f, duration, "%.2f"))
        {
            playing_ = false;
            animation_.SetTime(time);
        }
        ImGui::Text(u8"%.2f / %.2f 秒", animation_.GetCurrentTime() / Const::ANIM_FPS,
            duration / Const::ANIM_FPS);
        ImGui::EndDisabled();
        ImGui::TextWrapped(u8"Q+Eで全体の時間停止。F9で1/60秒進みます。再生速度は全体のゲーム速度に乗算されます。");
        ImGui::Separator();
        ImGui::TextUnformatted(u8"右ドラッグで回転。時間停止中も視点を動かせます。");
        ImGui::SliderFloat(u8"カメラ距離", &distance_, 150.0f, 900.0f, "%.0f");
        ImGui::SliderFloat(u8"カメラ上下角度", &pitch_, -75.0f, 75.0f, "%.0f");
        if (ImGui::Button(u8"正面")) yaw_ = 0.0f;
        ImGui::SameLine();
        if (ImGui::Button(u8"横")) yaw_ = 90.0f;
        ImGui::SameLine();
        if (ImGui::Button(u8"後ろ")) yaw_ = 180.0f;
        ImGui::SameLine();
        if (ImGui::Button(u8"視点リセット")) ResetView();
        ImGui::Checkbox(u8"床のグリッド", &showGrid_);
        if (!status_.empty()) ImGui::TextWrapped("%s", status_.c_str());
    }
    ImGui::End();
#endif
}

void CatViewerScene::PrunePreviewEffects()
{
    previewEffects_.erase(std::remove_if(previewEffects_.begin(), previewEffects_.end(),
        [](int handle) { return IsEffekseer3DEffectPlaying(handle) != 0; }), previewEffects_.end());
}

void CatViewerScene::StopPreviewEffects()
{
    // Wrapper playing handles are different from resource handles. Do not delete
    // or reload shared effect assets, and do not stop another scene's instances.
    PrunePreviewEffects();
    for (int handle : previewEffects_) EffectManager::StopEffect(handle);
    previewEffects_.clear();
    effectsPlaying_ = false;
    effectsPaused_ = false;
    dustRemaining_ = 0.0f;
}

void CatViewerScene::ApplyPreviewEffectSettings()
{
    PrunePreviewEffects();
    for (int handle : previewEffects_)
    {
        // ResourceManager's existing load magnification (15/30) remains intact.
        SetScalePlayingEffekseer3DEffect(handle, effectScale_, effectScale_, effectScale_);
        SetSpeedPlayingEffekseer3DEffect(handle, effectsPaused_ ? 0.0f : effectSpeed_);
    }
}

void CatViewerScene::SpawnPreviewEffects()
{
    PrunePreviewEffects();
    const auto& preset = EffectPresets[effectIndex_];
    const int resource = RM().GetEffect(preset.resourceKey);
    const int count = preset.compoundDust ? dustCount_ : 1;
    const float yaw = effectYawDegrees_ * DxPlus::Deg2Rad;
    for (int i = 0; i < count && previewEffects_.size() < MaxPreviewEffects; ++i)
    {
        Vec3 position{ effectPosition_[0], effectPosition_[1], effectPosition_[2] };
        float direction = yaw;
        if (preset.compoundDust && count > 1)
        {
            direction += 2.0f * DX_PI_F * static_cast<float>(i) / static_cast<float>(count);
            position += Vec3{ std::sin(direction) * dustRadius_, 0, std::cos(direction) * dustRadius_ };
        }
        const int handle = EffectManager::PlayEffect(resource, position, direction);
        if (handle < 0)
        {
            effectStatus_ = u8"エフェクトを再生できませんでした。";
            break;
        }
        previewEffects_.push_back(handle);
    }
    if (previewEffects_.size() >= MaxPreviewEffects)
        effectStatus_ = u8"同時再生は128個までです。終了したものから追加します。";
    ApplyPreviewEffectSettings();
}

void CatViewerScene::PlayPreviewEffect()
{
    StopPreviewEffects();
    effectStatus_.clear();
    effectsPlaying_ = true;
    SpawnPreviewEffects();
    dustRemaining_ = dustInterval_;
    if (previewEffects_.empty()) effectsPlaying_ = false;
}

void CatViewerScene::UpdatePreviewEffects(float deltaTime)
{
    // This wrapper takes seconds. The scene manager already applies Q+E/F9 and
    // global slow motion, independently of the cat animation's local play state.
    gameContext->GetEffectManager().Update(deltaTime);
    PrunePreviewEffects();
    if (!effectsPlaying_ || effectsPaused_) return;
    if (EffectPresets[effectIndex_].compoundDust && repeatEffects_)
    {
        dustRemaining_ -= deltaTime * effectSpeed_;
        if (dustRemaining_ <= 0.0f)
        {
            // Like Player::UpdateWalkDust: overlap fresh one-shot instances.
            // At most one batch per update; a long frame must not flood particles.
            SpawnPreviewEffects();
            dustRemaining_ = dustInterval_;
        }
    }
    else if (previewEffects_.empty())
    {
        if (repeatEffects_) SpawnPreviewEffects();
        else effectsPlaying_ = false;
    }
}

void CatViewerScene::DrawEffectImGui()
{
#ifndef NDEBUG
    if (!ImGui::CollapsingHeader(u8"エフェクト鑑賞", ImGuiTreeNodeFlags_DefaultOpen)) return;
    if (ImGui::BeginCombo(u8"エフェクト", EffectPresets[effectIndex_].label))
    {
        for (int i = 0; i < static_cast<int>(EffectPresets.size()); ++i)
        {
            if (ImGui::Selectable(EffectPresets[i].label, effectIndex_ == i))
            {
                StopPreviewEffects();
                effectStatus_.clear();
                effectIndex_ = i;
                repeatEffects_ = EffectPresets[i].compoundDust;
            }
        }
        ImGui::EndCombo();
    }
    if (ImGui::Button(u8"エフェクト再生 / 最初から")) PlayPreviewEffect();
    ImGui::SameLine();
    if (ImGui::Button(u8"エフェクト停止")) StopPreviewEffects();
    ImGui::BeginDisabled(!effectsPlaying_);
    if (ImGui::Button(effectsPaused_ ? u8"エフェクト再開" : u8"エフェクト一時停止"))
    {
        effectsPaused_ = !effectsPaused_;
        ApplyPreviewEffectSettings();
    }
    ImGui::EndDisabled();
    ImGui::Checkbox(u8"繰り返し再生 / 連続発生", &repeatEffects_);
    bool settingsChanged = ImGui::DragFloat(u8"エフェクト倍率", &effectScale_, 0.01f, 0.01f, 10.0f,
        "%.2fx", ImGuiSliderFlags_AlwaysClamp);
    settingsChanged |= ImGui::SliderFloat(u8"エフェクト速度", &effectSpeed_, 0.05f, 3.0f,
        "%.2fx", ImGuiSliderFlags_AlwaysClamp);
    if (settingsChanged) ApplyPreviewEffectSettings();
    ImGui::DragFloat3(u8"エフェクト発生位置", effectPosition_, 1.0f, -2000.0f, 2000.0f,
        "%.1f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::DragFloat(u8"エフェクト向き", &effectYawDegrees_, 1.0f, -360.0f, 360.0f,
        "%.0f deg", ImGuiSliderFlags_AlwaysClamp);
    if (EffectPresets[effectIndex_].compoundDust)
    {
        ImGui::SliderInt(u8"砂ぼこりの個数（1回分）", &dustCount_, 1, 12);
        ImGui::DragFloat(u8"砂ぼこりの広がり", &dustRadius_, 1.0f, 0.0f, 200.0f,
            "%.1f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::DragFloat(u8"発生間隔（秒）", &dustInterval_, 0.01f, 0.02f, 3.0f,
            "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::TextWrapped(u8"仮版：地面に複数のsunaを重ねて発生。初期値4個 / 0.18秒間隔。個数1・広がり0ならゲームと同じ間隔の単発重ねです。");
    }
    ImGui::Checkbox(u8"猫を表示", &showCat_);
    ImGui::Text(u8"再生中のエフェクト: %d", static_cast<int>(previewEffects_.size()));
    ImGui::TextWrapped(u8"位置・向き・個数は次の発生から反映。Q+Eで全体停止、F9でコマ送り。猫の再生停止とは独立しています。");
    if (!effectStatus_.empty()) ImGui::TextWrapped("%s", effectStatus_.c_str());
#endif
}

void CatViewerScene::End()
{
    StopPreviewEffects();
    animation_.Reset();
    if (modelHandle_ >= 0) DxLib::MV1DeleteModel(modelHandle_);
    modelHandle_ = -1;
}
