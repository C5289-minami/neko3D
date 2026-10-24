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
#endif

	DxPlus::Initialize(w, h, runConfig.windowed);
	DxLib::SetMouseDispFlag(TRUE);

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

	scene = &titleScene; // 最初のシーン

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
			scene->SetNextScene(&titleScene);
			scene->StartFadeOut();
		}
		wasF11Down = f11Down;

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

			DxPlus::Debug::Draw();
			scene->DrawFadeOverlay();

#ifndef NDEBUG

			//DebugUI
			if (runConfig.enableDebugUI && runConfig.windowed)
			{
				debugUI.BeginFrame();
				debugUI.Draw(gameContext);
				scene->DrawImGui();
				debugUI.EndFrame();
			}
#endif

			DxLib::ScreenFlip();
		}
	}
}
