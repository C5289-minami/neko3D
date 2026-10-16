
#include "ModelConfigScene.h"

#include "ModelObjectManager.h"
#include "SceneManager.h"
#include "imgui.h"

#include <algorithm>
#include <string>

namespace
{
    bool EditVector3(const char* label, Vec3& value, float speed)
    {
        float components[] = { value.x, value.y, value.z };
        if (!ImGui::DragFloat3(label, components, speed)) return false;
        value = { components[0], components[1], components[2] };
        return true;
    }
}

void ModelConfigScene::Init()
{
    DxLib::SetBackgroundColor(gameContext->GetBgRed(), gameContext->GetBgGreen(), gameContext->GetBgBlue());
    camera_.Initialize({ 0.0f, 150.0f, -1000.0f }, {});
    const auto& objects = MObjs().GetObjects();
    selected_ = objects.empty() ? nullptr : objects.front();
    StartFadeIn();
}

void ModelConfigScene::Update(float deltaTime)
{
    Vec3 focus{};
    if (selected_) focus = selected_->position;
    // --- 右クリックを押している間のみカメラ操作 ---
    if (!camera_.IsSceneViewActive() && (DxLib::GetMouseInput() & MOUSE_INPUT_RIGHT) != 0)
    {
        SetMouseDispFlag(FALSE);
        camera_.InputUpdate(deltaTime);
    }
    else SetMouseDispFlag(TRUE);

    if (DxLib::CheckHitKey(KEY_INPUT_F2))
    {
        SetNextScene(SM().GetScene(SceneID::Title));
        StartFadeOut();
    }
}

void ModelConfigScene::Render() const
{
    camera_.ApplyCamera();
    MObjs().Draw();
    DxPlus::Text::DrawString(L"F2: Return  |  Alt+Enter: Free Camera",
        { 20.0f, 20.0f }, DxLib::GetColor(255, 255, 255));
}

void ModelConfigScene::DrawImGui()
{
    const auto& objects = MObjs().GetObjects();
    ImGui::Begin(u8"ModelObject設定");
    ImGui::TextWrapped(u8"各シーンなどからMObjs().Add(object)したModelObjectを表示・編集します。");
    ImGui::TextWrapped(u8"Free Camera: Alt+Enterで切替 / WASD・QEで移動 / マウスで視点 / Shiftで加速 / Fで選択対象へ / Rで視点リセット");

    if (objects.empty())
    {
        ImGui::TextWrapped(u8"登録されたModelObjectがありません。ModelObjectを作成した場所でMObjs().Add(object)を呼んでください。");
        selected_ = nullptr;
    }
    else
    {
        for (auto* object : objects)
        {
            if (!object) continue;
            const std::string label(object->modelKey.begin(), object->modelKey.end());
            ImGui::PushID(object);
            if (ImGui::Selectable(label.c_str(), selected_ == object)) selected_ = object;
            ImGui::PopID();
        }
    }

    if (selected_ && std::find(objects.begin(), objects.end(), selected_) != objects.end())
    {
        ImGui::Separator();
        bool changed = EditVector3("Position", selected_->position, 1.0f);
        changed |= EditVector3("Rotation (rad)", selected_->rotation, 0.01f);
        changed |= EditVector3("Scale", selected_->scale, 0.1f);
        if (changed && (selected_->scale.x <= 0.0f || selected_->scale.y <= 0.0f || selected_->scale.z <= 0.0f))
            status_ = u8"Scaleは0より大きい値にしてください。";
        if (ImGui::Button(u8"選択中を保存"))
            status_ = selected_->Save() ? u8"保存しました。" : u8"保存に失敗しました。";
        ImGui::SameLine();
        if (ImGui::Button(u8"すべて保存"))
            status_ = MObjs().SaveAll() ? u8"保存しました。" : u8"保存に失敗しました。";
    }
    if (!status_.empty()) ImGui::TextUnformatted(status_.c_str());
    ImGui::End();
}
