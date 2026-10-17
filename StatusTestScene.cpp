#include "StatusTestScene.h"
#include "GameContext.h"
#include "ResourceKeys.h"
#include "Collision.h"

#include <algorithm>

#ifndef NDEBUG
#include "imgui.h"
#endif // !NDEBUG


void StatusTestScene::Init()
{
	// 背景色
	DxLib::SetBackgroundColor(gameContext->GetBgRed(), gameContext->GetBgGreen(), gameContext->GetBgBlue());

	camera_.Reset();
	freeCamera_.Reset();
	activeCamera_ = &freeCamera_;
	activeCamera_->ApplyCamera();

	// トゥーン描画に必要なシェーダーと設定バッファーを初期化
	if (!modelToonRenderer_.Init())
		DxPlus::Utils::FatalError(L"modelToonRendererの初期化失敗");
	if (!shaderEffects_.Init(DxPlus::CLIENT_WIDTH, DxPlus::CLIENT_HEIGHT))
		DxPlus::Utils::FatalError(L"shaderEffectsの初期化失敗");

	// toon描画とアウトライン描画を有効にする
	shaderEffects_.GetSettings().toonEnabled = true;
	shaderEffects_.GetSettings().outlineEnabled = true;

	testModel_.modelKey = ResourceKeys::Model_Test;

	player_.Init();
    player_.Reset();
	player_.SetPosition({ 0.0f, 100.0f, 0.0f });

	if (!CharSettingsManager::Load())
		DxPlus::Utils::FatalError(L"キャラクター情報の読み込みに失敗");
}

void StatusTestScene::Update(float deltaTime)
{
	// 背景色
	DxLib::SetBackgroundColor(gameContext->GetBgRed(), gameContext->GetBgGreen(), gameContext->GetBgBlue());

	float scale = 1.0f;
	testModel_.scale = { scale, scale, scale };

	// --- 右クリックを押している間のみカメラ操作 ---
	if ((DxLib::GetMouseInput() & MOUSE_INPUT_RIGHT) != 0)
	{
		SetMouseDispFlag(FALSE);
		activeCamera_->Update(deltaTime);
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

	player_.Update(deltaTime);
	if (player_.GetPosition().y < 0.0f)
	{
		player_.SetPosition({ player_.GetPosition().x, 0.0f, player_.GetPosition().z });
		player_.SetGround(true);
	}

	Collision::UpdateModelContact(player_.GetModelObject(), testModel_);

}

void StatusTestScene::DrawImGui()
{
  #ifndef NDEBUG
	ImGui::Begin("Status Test Scene");
	{
		ImGui::Text("F2: Toggle Shader Effects");

		// キャラクターの追加とStatus設定
		if (ImGui::Button("Add Character", ImVec2(-1, 0)))
		{
			CharData newChar{};
         newChar.name = u8"キャラクター " + std::to_string(charSettings_.GetCount() + 1);
			charSettings_.Add(newChar);
		}
		for (size_t i = 0; i < charSettings_.GetCount(); ++i)
		{
            ImGui::PushID(static_cast<int>(i));
			CharData& charData = charSettings_.Get(i);
            const std::string headerLabel = charData.name + "###CharacterHeader";
			if (ImGui::CollapsingHeader(headerLabel.c_str()))
			{
                char nameBuffer[256]{};
				const size_t nameLength = std::min(charData.name.size(), sizeof(nameBuffer) - 1);
				std::copy_n(charData.name.data(), nameLength, nameBuffer);
				ImGui::Text(u8"名前"); ImGui::SameLine();
               if (ImGui::InputText(u8"###名前", nameBuffer, sizeof(nameBuffer)))
					charData.name = nameBuffer;
				ImGui::Text(u8"最大HP"); ImGui::SameLine();
				ImGui::InputFloat(u8"###最大HP",   &charData.maxHealth);
				ImGui::Text(u8"移動速度"); ImGui::SameLine();
				ImGui::InputFloat(u8"###移動速度",   &charData.moveSpeed);
				ImGui::Text(u8"攻撃倍率"); ImGui::SameLine();
				ImGui::InputFloat(u8"###攻撃倍率", &charData.attackRatio);
				ImGui::Text(u8"攻撃速度"); ImGui::SameLine();
				ImGui::InputFloat(u8"###攻撃速度", &charData.attackSpeed);
				if (ImGui::Button("Remove Character"))
				{
					charSettings_.Remove(i);
					ImGui::PopID();
					break;
				}
			}
           ImGui::PopID();
		}
		// 保存、読み込み
		if (ImGui::Button("Save", ImVec2(-1, 0)))
		{
			if (!CharSettingsManager::Save())
				DxPlus::Utils::FatalError(L"キャラクター情報の保存に失敗");
		}
		if (ImGui::Button("Load", ImVec2(-1, 0)))
		{
			if (!CharSettingsManager::Load())
				DxPlus::Utils::FatalError(L"キャラクター情報の読み込みに失敗");
		}
	}
	ImGui::End();
   #endif
}

void StatusTestScene::Render() const
{
	activeCamera_->ApplyCamera();

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


void StatusTestScene::RenderDepth() const
{
	// シーンのカメラを設定する
	activeCamera_->ApplyCamera();
	// Boneなしシェーダー
	modelToonRenderer_.SetVertexShader(ModelToonType::Model);

	testModel_.Draw();

	// Bone対応の頂点シェーダー
	modelToonRenderer_.SetVertexShader(ModelToonType::Bone);
	player_.Draw();

}

void StatusTestScene::DrawScene(bool toonEnabled) const
{
	// シーンのカメラを設定する
	activeCamera_->ApplyCamera();

	// つかわないもの
	grid_.Draw();

	// トゥーンシェーダーを使わない場合
	if (!toonEnabled)
	{
		SetUsePixelShader(-1);
		SetUseVertexShader(-1);

		testModel_.Draw();
		player_.Draw();


		return;
	}

	// トゥーン描画開始
	modelToonRenderer_.BeginPass();

	const auto drawActor = [this](const ModelObject& model, ModelToonType type)
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
	drawActor(testModel_, ModelToonType::Model);
	drawActor(player_.GetModelObject(), ModelToonType::Bone);


	modelToonRenderer_.EndPass();
}
