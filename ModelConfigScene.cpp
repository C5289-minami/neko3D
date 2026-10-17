
#include "ModelConfigScene.h"

#include "ModelObjectManager.h"
#include "SceneManager.h"
#include "imgui.h"

#include <algorithm>
#include <string>

namespace
{
	// ImGuiでVec3を編集するためのヘルパー関数
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
    // 背景色
    DxLib::SetBackgroundColor(gameContext->GetBgRed(), gameContext->GetBgGreen(), gameContext->GetBgBlue());

	//cameraの初期化
    camera_.Initialize({ 0.0f, 150.0f, -1000.0f }, {});
    
	// 最初のオブジェクトを選択
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
        camera_.Update(deltaTime);
    }
    else SetMouseDispFlag(TRUE);
}

void ModelConfigScene::Render() const
{
    camera_.ApplyCamera();
    MObjs().Draw();
}

void ModelConfigScene::DrawImGui()
{
#ifndef NDEBUG


    const auto& objects = MObjs().GetObjects();
    ImGui::Begin(u8"ModelObject設定");
    ImGui::TextWrapped(u8"各シーンなどからMObjs().Add(object)したModelObjectを表示・編集する");
    ImGui::TextWrapped(u8"Free Camera: 右クリで稼働 / WASD・QEで移動 / マウスで視点 / Shiftで加速 / Fで選択対象へ / Rで視点リセット");

    // 編集用がない場合
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
			// modelKeyをUTF-8に変換して表示
            const std::string label(object->modelKey.begin(), object->modelKey.end());
            ImGui::PushID(object);
			// 選択中のオブジェクトをハイライト
            if (ImGui::Selectable(label.c_str(), selected_ == object)) selected_ = object;
            ImGui::PopID();
        }
    }

	// 選択中のオブジェクトが有効な場合のみ編集UIを表示
    if (selected_ && std::find(objects.begin(), objects.end(), selected_) != objects.end())
    {
        ImGui::Separator();
		// 選択中のオブジェクトのプロパティを編集
        bool changed = EditVector3("Position", selected_->position, 1.0f);
        changed |= EditVector3("Rotation (rad)", selected_->rotation, 0.01f);
        changed |= EditVector3("Scale", selected_->scale, 0.1f);
		// Scaleが0以下になった場合は警告を表示
        if (changed && (selected_->scale.x <= 0.0f || selected_->scale.y <= 0.0f || selected_->scale.z <= 0.0f))
            status_ = u8"Scaleは0より大きい値にしてください。";
		// 保存ボタン
        if (ImGui::Button(u8"選択中を保存"))
            status_ = selected_->Save() ? u8"保存しました。" : u8"保存に失敗しました。";
        ImGui::SameLine();
		// すべて保存ボタン
        if (ImGui::Button(u8"すべて保存"))
            status_ = MObjs().SaveAll() ? u8"保存しました。" : u8"保存に失敗しました。";
    }

	// ステータス表示
    if (!status_.empty()) ImGui::TextUnformatted(status_.c_str());
    ImGui::End();
#endif // !NDEBUG
}
