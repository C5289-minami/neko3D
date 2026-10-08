// =============================
// Scenes/Base/SceneManager.cpp
// =============================
#include "SceneManager.h"
#include "ResourceManager.h"
#include "DxPlus/DxPlus.h"

#include "DebugUI.h"
#include "UIManager.h"

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
    bossTestScene.SetGameContext(&gameContext);
    ssFlipTestScene.SetGameContext(&gameContext);

    scene = &titleScene; // 最初のシーン
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
        case SceneID::BossTest: return &bossTestScene;
        case SceneID::SsFlipTest: return &ssFlipTestScene;
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
                debugUI.EndFrame();
            }
#endif

            DxLib::ScreenFlip();
        }
    }
}
