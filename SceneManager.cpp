// =============================
// Scenes/Base/SceneManager.cpp
// =============================
#include "SceneManager.h"
#include "ResourceManager.h"
#include "DxPlus/DxPlus.h"
#include "Collision.h"
#include "DebugUI.h"
#include "UIManager.h"
#include "ToonSettings.h"
#include "OutlineSettings.h"

#include <vector>
#include <array>
#ifndef NDEBUG
#include "imgui.h"
#endif

void SceneManager::Init()
{
#ifdef _DEBUG
	Collision::ClearDebugCanWatch();
#endif
	const int w = DxPlus::CLIENT_WIDTH;
	const int h = DxPlus::CLIENT_HEIGHT;

	// ウィンドウを作成する前に表示サイズを設定する。
#ifndef NDEBUG
	DxLib::SetWindowSize(static_cast<int>(w * 0.7f), static_cast<int>(h * 0.7f));
#else
	DxLib::SetWindowSize(w, h);
	runConfig.windowed = false; // Releaseはフルスクリーン固定
#endif

	DxPlus::Initialize(w, h, runConfig.windowed);
	DxPlus::Input::SetMouseCaptureEnabled(!runConfig.windowed);
#ifndef NDEBUG
	// Debug のときだけ、さらに windowed のときだけ DebugUI を許可
	if (runConfig.enableDebugUI && runConfig.windowed)
	{
		debugUI.Init();
	}
#endif
	DxPlus::DxWrapper::GetInstance().SetFpsCap(runConfig.fpsCap);

	ResourceManager::GetInstance().LoadAll();
	gameContext.Init();

	std::vector<Scene*> scenes{
		&titleScene,
		&gameScene,
		&resultScene
	};
#ifndef NDEBUG
	scenes.insert(scenes.end(), {
		 &shaderTestScene,
		 &ssFlipTestScene,
		 &roomTestScene,
		 &statusTestScene,
		 &modelConfigScene,
		 &gameTestScene
		});
#endif
	for (Scene* currentScene : scenes)
		currentScene->SetGameContext(&gameContext);

#ifndef NDEBUG
	scene = &titleScene; // 最初のシーン
#else
	scene = &gameScene; // 最初のシーン
#endif // !NDEBUG

	// 設定ファイルのロード
	ToonSettingsManager::Load();
	OutlineSettingsManager::Load();
}

void SceneManager::Shutdown()
{
#ifdef _DEBUG
	// 参照先のキャラクターやリソースを解放する前に登録を解除する。
	Collision::ClearDebugCanWatch();
#endif
	// UI が所有する SpriteStudio Player を、参照先リソースより先に破棄する。
	UIM().Clear();
	// 判定用の複製モデルを、元モデルと DxLib より先に解放する。
	gameScene.End();
	gameContext.GetStage().GetCollision().Clear();
	ResourceManager::GetInstance().UnloadAll();

#ifndef NDEBUG
	if (runConfig.enableDebugUI && runConfig.windowed)
	{
		debugUI.Shutdown();
	}
#endif

	DxPlus::Shutdown();
}

void SceneManager::SetScene(Scene* newScene)
{
	if (!newScene || newScene == scene) return;
#ifndef NDEBUG
	const auto usesDebugCursor = [this](const Scene* target)
	{
		return target == &shaderTestScene || target == &modelConfigScene ||
			target == &roomTestScene || target == &statusTestScene;
	};
	const bool showPointer = usesDebugCursor(newScene);
	if (usesDebugCursor(scene) != showPointer)
		DxLib::SetMouseDispFlag(showPointer ? TRUE : FALSE);
#endif
	scene = newScene; // 破棄しない＝常駐
}

Scene* SceneManager::GetScene(SceneID id)
{
	switch (id)
	{
	case SceneID::Title:    return &titleScene;
	case SceneID::Game:     return &gameScene;
	case SceneID::Result:   return &resultScene;
#ifndef NDEBUG
	case SceneID::ShaderTest: return &shaderTestScene;
	case SceneID::SsFlipTest: return &ssFlipTestScene;
	case SceneID::NekoMotionTest: return &roomTestScene;
	case SceneID::StatusTest: return &statusTestScene;
	case SceneID::ModelConfig: return &modelConfigScene;
	case SceneID::GameTest: return &gameTestScene;
#endif // !NDEBUG
	}
	return &titleScene;
}

namespace
{
#ifndef NDEBUG
    constexpr std::array<SceneID, 7> PresentationOrder{
        SceneID::SsFlipTest, SceneID::ShaderTest, SceneID::NekoMotionTest,
        SceneID::StatusTest, SceneID::ModelConfig, SceneID::Game, SceneID::GameTest
    };
    const char* const PresentationNames[] = {
        "F1: SpriteStudio", "F2: Shader", "F3: Motion / Room",
        "F4: Status", "F5: Model Config", "F6: Game", "Enter: Battle Test"
    };
#endif
}

void SceneManager::SwitchSceneImmediate(Scene* target)
{
    if (!target) return;
    if (scene) { scene->End(); scene->SetNextScene(nullptr); }
    gameContext.StopTailWhipSound();
    gameContext.ClearUltimateProjectiles();
    Collision::ResetTransparentModels();
    SetScene(target);
    UIM().Init();
    scene->Init();
    scene->SetNextScene(nullptr);
    scene->StartFadeIn(0.0f);
}

void SceneManager::AdvancePresentation(int direction)
{
#ifndef NDEBUG
    int index = -1;
    for (int i = 0; i < static_cast<int>(PresentationOrder.size()); ++i)
        if (GetScene(PresentationOrder[i]) == scene) index = i;
    const int next = index < 0 ? (direction > 0 ? 0 : 6) : index + direction;
    pendingScene = next < 0 || next >= static_cast<int>(PresentationOrder.size())
        ? GetScene(SceneID::Title) : GetScene(PresentationOrder[next]);
#else
    (void)direction;
#endif
}

void SceneManager::DrawPresentationControls()
{
#ifndef NDEBUG
    if (!runConfig.presentationMode) return;
    ImGui::Begin(u8"発表コントロール");
    ImGui::TextUnformatted(u8"F11: 選択画面 / M: 次 / Shift+M: 前 / F10: UI表示");
    int index = -1;
    for (int i = 0; i < static_cast<int>(PresentationOrder.size()); ++i)
        if (GetScene(PresentationOrder[i]) == scene) index = i;
    ImGui::Text(u8"現在: %s", index >= 0 ? PresentationNames[index] : "Scene Menu / Result");
    ImGui::Text(u8"次: %s", index < 6 ? PresentationNames[index + 1] : "Scene Menu");
    if (ImGui::Button(u8"選択画面へ (F11)")) RequestPresentationScene(SceneID::Title);
    ImGui::SameLine();
    if (ImGui::Button(u8"前へ")) AdvancePresentation(-1);
    ImGui::SameLine();
    if (ImGui::Button(u8"次へ (M)")) AdvancePresentation(1);
    if (ImGui::BeginCombo(u8"直接移動", u8"シーンを選択"))
    {
        for (int i = 0; i < static_cast<int>(PresentationOrder.size()); ++i)
            if (ImGui::Selectable(PresentationNames[i], index == i))
                RequestPresentationScene(PresentationOrder[i]);
        ImGui::EndCombo();
    }
    ImGui::End();
#endif
}

void SceneManager::Run()
{
	if (scene)
	{
		UIM().Init();
		scene->Init();
	}
	while (DxPlus::GameLoop(true))
	{
		DxPlus::Input::Update();

		const bool f11Down = DxLib::CheckHitKey(KEY_INPUT_F11) != 0;
		if (f11Down && !wasF11Down && scene && scene != &titleScene)
		{
            pendingScene = &titleScene;
		}
        wasF11Down = f11Down;
#ifndef NDEBUG
        const bool mDown = DxLib::CheckHitKey(KEY_INPUT_M) != 0;
        const bool f10Down = DxLib::CheckHitKey(KEY_INPUT_F10) != 0;
        const bool capturing = ImGui::GetCurrentContext() && ImGui::GetIO().WantCaptureKeyboard;
        if (runConfig.presentationMode && !capturing)
        {
            if (mDown && !wasMDown)
                AdvancePresentation(DxLib::CheckHitKey(KEY_INPUT_LSHIFT) ||
                    DxLib::CheckHitKey(KEY_INPUT_RSHIFT) ? -1 : 1);
            if (f10Down && !wasF10Down) showPresentationUI = !showPresentationUI;
        }
        wasMDown = mDown;
        wasF10Down = f10Down;
#endif
        if (pendingScene)
        {
            Scene* target = pendingScene;
            pendingScene = nullptr;
            SwitchSceneImmediate(target);
        }

		if (scene)
		{
			DxLib::ClearDrawScreen();


			float deltaTime = DxPlus::GetDeltaTime();
			scene->Drive(deltaTime);
#ifdef _DEBUG
			Collision::UpdateDebug();
#endif
			UIM().Update(deltaTime);
			scene->Render();
#ifdef _DEBUG
			Collision::DrawDebug();
#endif
			UIM().Draw();

			if (scene->IsFinished())
			{
				scene->End();

				Scene* next = scene->GetNextScene();
				scene->SetNextScene(nullptr);

				if (!next) { DxLib::ScreenFlip(); break; }
				SetScene(next);
				UIM().Init();
				next->Init();
			}
#ifndef NDEBUG
            if (showPresentationUI) DxPlus::Debug::Draw();
#endif
			scene->DrawFadeOverlay();

#ifndef NDEBUG

			//DebugUI
			if (runConfig.enableDebugUI && runConfig.windowed)
			{
				debugUI.BeginFrame();
                if (showPresentationUI)
                {
                    debugUI.Draw(gameContext);
                    scene->DrawImGui();
                    DrawPresentationControls();
                }
				debugUI.EndFrame();
			}
#endif

			DxLib::ScreenFlip();
		}
	}
}
