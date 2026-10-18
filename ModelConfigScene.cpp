
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
	Vec3 focus{};
	if (selectedActor_)
		focus = selectedActor_->GetPosition();
	else if (selected_)
	{
		const auto* owner = MObjs().GetOwner(*selected_);
		focus = owner
			? selected_->WithParentTransform(owner->GetPosition(), owner->GetRotation()).position
			: selected_->position;
	}

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

				const std::string modelName(object->modelKey.begin(), object->modelKey.end());
				const std::string label = modelName + " (Local)";
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
			// modelKeyをUTF-8に変換して表示
			const std::string modelName(object->modelKey.begin(), object->modelKey.end());
			ImGui::PushID(object);
			// 選択中のオブジェクトをハイライト
			if (ImGui::Selectable(modelName.c_str(), !selectedActor_ && selected_ == object))
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
		bool changed = EditVector3(isLocalModel ? u8"Local Position" : "Position", selected_->position, 1.0f);
		changed |= EditVector3("Rotation (rad)", selected_->rotation, 0.01f);
		changed |= EditVector3("Scale", selected_->scale, 0.1f);
		// Scaleが0以下になった場合は警告を表示
		if (changed && (selected_->scale.x <= 0.0f || selected_->scale.y <= 0.0f || selected_->scale.z <= 0.0f))
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
	// シーンのカメラを設定する
	camera_.ApplyCamera();

	for (const auto* object : objects)
	{
		if (!object) continue;

		if (object->modelType_ == ModelType::Bone)
			modelToonRenderer_.SetVertexShader(ModelType::Bone);
		else
			modelToonRenderer_.SetVertexShader(ModelType::Model);

		const auto* owner = MObjs().GetOwner(*object);
		if (owner)
			object->WithParentTransform(owner->GetPosition(), owner->GetRotation()).Draw();
		else
			object->Draw();
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
			const auto* owner = MObjs().GetOwner(*object);
			if (owner)
				object->WithParentTransform(owner->GetPosition(), owner->GetRotation()).Draw();
			else
				object->Draw();
		}

		return;
	}

	// トゥーン描画開始
	modelToonRenderer_.BeginPass();

	const auto drawActor = [this](const ModelObject& model, ModelType type)
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
				modelToonRenderer_.Draw(model, type);
			}
		};

	for (const auto* object : objects)
	{
		if (!object) continue;
		const auto* owner = MObjs().GetOwner(*object);
		const ModelObject drawObject = owner
			? object->WithParentTransform(owner->GetPosition(), owner->GetRotation())
			: *object;
		drawActor(drawObject, drawObject.modelType_);
	}

	modelToonRenderer_.EndPass();
}