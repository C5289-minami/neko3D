// =============================
// Scenes/Base/SceneManager.cpp
// =============================
#include "SceneManager.h"
#include "ResourceManager.h"
#include "DxPlus/DxPlus.h"

#include "DebugUI.h"
#include "UIManager.h"
#include "ToonSettings.h"
#include "OutlineSettings.h"

void SceneManager::Init()
{
    const int w = DxPlus::CLIENT_WIDTH;
    const int h = DxPlus::CLIENT_HEIGHT;

    DxPlus::Initialize(w, h, runConfig.windowed);
    DxLib::SetMouseDispFlag(TRUE);

#ifndef NDEBUG
    // Debug のときだけ、さらに windowed のときだけ DebugUI を許可
    if (runConfig.enableDebugUI && runConfig.windowed)
    {
        debugUI.Init();
    }
	DxLib::SetWindowSize(1280, 720);
#endif
    DxPlus::DxWrapper::GetInstance().SetFpsCap(runConfig.fpsCap);

    ResourceManager::GetInstance().LoadAll();
    gameContext.Init();

    titleScene.SetGameContext(&gameContext);
    gameScene.SetGameContext(&gameContext);
    resultScene.SetGameContext(&gameContext);
#ifndef NDEBUG
    shaderTestScene.SetGameContext(&gameContext);
    ssFlipTestScene.SetGameContext(&gameContext);
	nekoMotionTestScene.SetGameContext(&gameContext);
	statusTestScene.SetGameContext(&gameContext);
  modelConfigScene.SetGameContext(&gameContext);
#endif

    scene = &titleScene; // 最初のシーン

	// 設定ファイルのロード
    ToonSettingsManager::Load();
    OutlineSettingsManager::Load();
}

void SceneManager::Shutdown()
{
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
        case SceneID::ShaderTestScene: return &shaderTestScene;
        case SceneID::SsFlipTest: return &ssFlipTestScene;
		case SceneID::NekoMotionTest: return &nekoMotionTestScene;
		case SceneID::StatusTestScene: return &statusTestScene;
       case SceneID::ModelConfig: return &modelConfigScene;
#endif // !NDEBUG
    }
    return &titleScene;
}

void SceneManager::Run()
{
    if (scene)
    {
        scene->Init();
        UIM().Init();
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
            UIM().Update(deltaTime);
            scene->Render();
            UIM().Draw();

            if (scene->IsFinished())
            {
                scene->End();

                Scene* next = scene->GetNextScene();
                scene->SetNextScene(nullptr);

                if (!next) { DxLib::ScreenFlip(); break; }
                SetScene(next);
                next->Init();
                UIM().Init();
            }

            DxPlus::Debug::Draw();
            scene->DrawFadeOverlay();

#ifndef NDEBUG
            //DebugUI
            if (runConfig.enableDebugUI && runConfig.windowed)
            {
                debugUI.BeginFrame();
                debugUI.Draw(gameContext, scene->GetDebugControls());
                scene->DrawImGui();
                debugUI.EndFrame();
            }
#endif

            DxLib::ScreenFlip();
        }
    }
}
