#include "ModelConfigScene.h"

#include "ModelObjectManager.h"
#include "SceneManager.h"
#include "imgui.h"
#include "GameContext.h"


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
	std::string GetObjectLabel(const ModelObject& object)
	{
		if (!object.GetObjectName().empty())
			return object.GetObjectName();

		if (!object.GetObjectId().empty())
			return object.GetObjectId();

		const auto& key = object.GetModelKey();
		return std::string(key.begin(), key.end());
	}
}

void ModelConfigScene::Init()
{
	// 背景色
	DxLib::SetBackgroundColor(gameContext->GetBgRed(), gameContext->GetBgGreen(), gameContext->GetBgBlue());

	//cameraの初期化
	camera_.Initialize({ 0.0f, 150.0f, -1000.0f }, {});
	camera_.SetMoveShiftRatio(5.0f);

	// 最初のオブジェクトを選択
	const auto& objects = MObjs().GetObjects();
	const auto& actors = MObjs().GetActors();
	selectedActor_ = actors.empty() ? nullptr : actors.front().second;
	selected_ = selectedActor_ || objects.empty() ? nullptr : objects.front();

	for (const auto& [name, actor] : actors)
		if (actor) actor->Init();
	for (auto* object : objects)
		if (object) object->Init();

	// トゥーン描画に必要なシェーダーと設定バッファーを初期化
	if (!modelToonRenderer_.Init())
		DxPlus::Utils::FatalError(L"modelToonRendererの初期化失敗");
	if (!shaderEffects_.Init(DxPlus::CLIENT_WIDTH, DxPlus::CLIENT_HEIGHT))
		DxPlus::Utils::FatalError(L"shaderEffectsの初期化失敗");

	// toon描画とアウトライン描画を有効にする
	shaderEffects_.GetSettings().toonEnabled = true;
	shaderEffects_.GetSettings().outlineEnabled = true;


	StartFadeIn();
}

void ModelConfigScene::Update(float deltaTime)
{

	// --- 右クリックを押している間のみカメラ操作 ---
	if (!camera_.IsSceneViewActive() && (DxLib::GetMouseInput() & MOUSE_INPUT_RIGHT) != 0)
	{
		SetMouseDispFlag(FALSE);
		camera_.Update(deltaTime);
	}
	else SetMouseDispFlag(TRUE);

	// --- F2 でシェーダー全体のON/OFFを切り替える ---
	static bool shaderToggleWasDown = false;
	const bool shaderToggleDown =
		DxLib::CheckHitKey(KEY_INPUT_F2) != 0;

	if (shaderToggleDown && !shaderToggleWasDown)
	{
		shaderEffects_.SetEnabled(
			!shaderEffects_.IsEnabled()
		);
	}

	shaderToggleWasDown = shaderToggleDown;
}

void ModelConfigScene::Render() const
{
	if (shaderEffects_.IsEnabled())
	{
		shaderEffects_.Render(
			[this](bool toonEnabled) {
				DrawScene(toonEnabled);
			},
			[this] {
				RenderDepth();
			}
		);
	}
	else
	{
		DrawScene(false);
	}
}

void ModelConfigScene::DrawImGui()
{
#ifndef NDEBUG


	const auto& objects = MObjs().GetObjects();
	const auto& actors = MObjs().GetActors();
	ImGui::Begin(u8"ModelObject設定");
	ImGui::TextWrapped(u8"Model Configの使い方メモ");
	ImGui::TextWrapped(u8"  1.	Debugビルドで起動し、タイトル画面でF5を押す。");
	ImGui::TextWrapped(u8"  2.	左のツリーでPlayerを開く。");
	ImGui::TextWrapped(u8"  3.	編集したい対象を選ぶ。");
	ImGui::TextWrapped(u8"  4.	保存する。");
	ImGui::TextWrapped(u8"  5.	右クリック中にカメラ操作。WASD・QEで移動、マウスで視点を操作する。");
	ImGui::TextWrapped(u8"   設定はData / Config / model_objects.jsonに保存される。");
	for (const auto& [name, actor] : actors)
	{
		if (!actor) continue;
		ImGui::PushID(actor);
		const ImGuiTreeNodeFlags actorFlags = ImGuiTreeNodeFlags_DefaultOpen |
			(selectedActor_ == actor ? ImGuiTreeNodeFlags_Selected : 0);
		const bool expanded = ImGui::TreeNodeEx(name.c_str(), actorFlags);
		if (ImGui::IsItemClicked())
		{
			selectedActor_ = actor;
			selected_ = nullptr;
		}
		if (expanded)
		{
			for (auto* object : objects)
			{
				if (!object || MObjs().GetOwner(*object) != actor) continue;

				const std::string label = GetObjectLabel(*object);
				ImGui::PushID(object);
				if (ImGui::Selectable(label.c_str(), selected_ == object))
				{
					selected_ = object;
					selectedActor_ = nullptr;
				}
				ImGui::PopID();
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
	}

	if (actors.empty())
		ImGui::TextDisabled(u8"登録されたActorはありません。");

	bool hasUnownedObjects = false;
	for (auto* object : objects)
		if (object && !MObjs().GetOwner(*object)) hasUnownedObjects = true;

	if (hasUnownedObjects && ImGui::TreeNodeEx(u8"Other ModelObjects", ImGuiTreeNodeFlags_DefaultOpen))
	{
		for (auto* object : objects)
		{
			if (!object || MObjs().GetOwner(*object)) continue;
			const std::string label = GetObjectLabel(*object);
			ImGui::PushID(object);
			// 選択中のオブジェクトをハイライト
			if (ImGui::Selectable(label.c_str(),
				!selectedActor_ && selected_ == object))
			{
				selected_ = object;
				selectedActor_ = nullptr;
			}
			ImGui::PopID();
		}
		ImGui::TreePop();
	}

	if (selectedActor_)
	{
		ImGui::Separator();
		ImGui::TextUnformatted(u8"Actor Transform (World)");
		Vec3 position = selectedActor_->GetPosition();
		if (EditVector3(u8"World Position", position, 1.0f)) selectedActor_->SetPosition(position);
		float rotation[2]{ selectedActor_->GetPitch(), selectedActor_->GetYaw() };
		if (ImGui::DragFloat2(u8"World Rotation (Pitch/Yaw rad)", rotation, 0.01f))
		{
			selectedActor_->SetPitch(rotation[0]);
			selectedActor_->SetYaw(rotation[1]);
		}
		if (ImGui::Button(u8"Actor変換をJSONに保存"))
		{
			status_ = MObjs().SaveActorTransform(*selectedActor_)
				? u8"Actor変換をmodel_objects.jsonに保存しました。"
				: u8"Actor変換の保存に失敗しました。";
		}
	}

	// 選択中のオブジェクトが有効な場合のみ編集UIを表示
	if (!selectedActor_ && selected_ && std::find(objects.begin(), objects.end(), selected_) != objects.end())
	{
		ImGui::Separator();
		// 選択中のオブジェクトのプロパティを編集
		const bool isLocalModel = MObjs().GetOwner(*selected_) != nullptr;
      Vec3 position = selected_->GetPosition();
		Vec3 rotation = selected_->GetRotation();
		Vec3 scale = selected_->GetScale();
		const bool positionChanged = EditVector3(isLocalModel ? u8"Local Position" : "Position", position, 1.0f);
		const bool rotationChanged = EditVector3("Rotation (rad)", rotation, 0.01f);
		const bool scaleChanged = EditVector3("Scale", scale, 0.1f);
		const bool changed = positionChanged || rotationChanged || scaleChanged;
		if (positionChanged) selected_->SetPosition(position);
		if (rotationChanged) selected_->SetRotation(rotation);
		if (scaleChanged) selected_->SetScale(scale);
		// Scaleが0以下になった場合は警告を表示
        if (changed && (scale.x <= 0.0f || scale.y <= 0.0f || scale.z <= 0.0f))
			status_ = u8"Scaleは0より大きい値にしてください。";
		// 保存ボタン
		if (ImGui::Button(u8"選択中を保存"))
			status_ = selected_->Save() ? u8"保存しました。" : u8"保存に失敗しました。";
	}

	if (ImGui::Button(u8"すべて保存"))
		status_ = MObjs().SaveAll() ? u8"保存しました。" : u8"保存に失敗しました。";

	// ステータス表示
	if (!status_.empty()) ImGui::TextUnformatted(status_.c_str());
	ImGui::End();
#endif // !NDEBUG
}

void ModelConfigScene::RenderDepth() const
{
	const auto& objects = MObjs().GetObjects();

	camera_.ApplyCamera();

	for (const auto* object : objects)
	{
		if (!object) continue;

		modelToonRenderer_.DrawDepth(*object);
	}
}
void ModelConfigScene::DrawScene(bool toonEnabled) const
{
	const auto& objects = MObjs().GetObjects();

	// カメラを設定する
	camera_.ApplyCamera();
	// トゥーンシェーダーを使わない場合
	if (!toonEnabled)
	{
		SetUsePixelShader(-1);
		SetUseVertexShader(-1);

		for (const auto* object : objects)
		{
			if (!object) continue;
			object->Draw();
		}

		return;
	}

	// トゥーン描画開始
	modelToonRenderer_.BeginPass();

	const auto drawActor = [this](const ModelObject& model)
		{
			if (model.collisionHighlighted)
			{
				SetUsePixelShader(-1);
				SetUseVertexShader(-1);
				MV1SetUseOrigShader(FALSE);
				model.Draw();
				MV1SetUseOrigShader(TRUE);
			}
			else
			{
				modelToonRenderer_.Draw(model);
			}
		};

	for (const auto* object : objects)
	{
		if (!object) continue;
		drawActor(*object);
	}

	modelToonRenderer_.EndPass();
}