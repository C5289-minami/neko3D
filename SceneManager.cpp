// =============================
// Scenes/Base/SceneManager.cpp
// =============================
#include "SceneManager.h"
#include "ResourceManager.h"
#include "DxPlus/DxPlus.h"

#include "DebugUI.h"

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
#endif
    DxPlus::DxWrapper::GetInstance().SetFpsCap(runConfig.fpsCap);

    ResourceManager::GetInstance().LoadAll();
    gameContext.Init();

    titleScene.SetGameContext(&gameContext);
    gameScene.SetGameContext(&gameContext);
    resultScene.SetGameContext(&gameContext);

    scene = &titleScene; // 最初のシーン


    // リソースマネージャーの作成
    ssResMan = ss::ResourceManager::getInstance();


    // ssbpファイルの読み込み (例: "Resources/sample.ssbp")
    ssResMan->addData("./Data/Images/character_template1.ssbp");

    // プレイヤーの作成
    ssPlayer = ss::Player::create();

    // アニメーションデータのセット
    ssPlayer->setData("character_template1");
    //再生するモーションを設定
    ssPlayer->play("character_template_3head/stance");

    //表示位置を設定
    ssPlayer->setPosition(1280 / 2, 720);
    //スケール設定
    ssPlayer->setScale(0.5f, 0.5f);
    //回転を設定
    ssPlayer->setRotation(0.0f, 0.0f, 0.0f);
    //透明度を設定
    ssPlayer->setAlpha(255);
    //反転を設定
    ssPlayer->setFlip(false, false);
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
    }
    return &titleScene;
}

void SceneManager::Run()
{
    if (scene) scene->Init();
    while (DxPlus::GameLoop(true))
    {
        DxPlus::Input::Update();

        if (scene)
        {
            DxLib::ClearDrawScreen();


            float deltaTime = DxPlus::GetDeltaTime();
            scene->Drive(deltaTime);
			ssPlayer->update(deltaTime); 
            scene->Render();
			ssPlayer->draw(); // SS6Playerの描画

            if (scene->IsFinished())
            {
                scene->End();

                Scene* next = scene->GetNextScene();
                scene->SetNextScene(nullptr);

                if (!next) { DxLib::ScreenFlip(); break; }
                SetScene(next);
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
                debugUI.EndFrame();
            }
#endif

            DxLib::ScreenFlip();
        }
    }
}
