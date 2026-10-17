// =============================
// Scenes/Base/SceneManager.h
// =============================
#pragma once
#include "Scene.h"
#include "GameContext.h"
#include "DebugUI.h"
#include "Consts.h"
#include "./SSPlayer/SS6Player.h"

#include "TitleScene.h"
#include "GameScene.h"
#include "ResultScene.h"
#ifndef NDEBUG
#include "ShaderTestScene.h"
#include "DebugScene.h"
#include "RoomTestScene.h"
#include "StatusTestScene.h"
#include "ModelConfigScene.h"
#include "GameTestScene.h"
#endif

enum class SceneID { 
    Title,
    Game,
    Result,
#ifndef NDEBUG
    ShaderTest,
    SsFlipTest,
	NekoMotionTest,
    StatusTest,
    ModelConfig,
    GameTest,
#endif
};

class SceneManager
{
public:
    static SceneManager& GetInstance()
    {
        static SceneManager instance;
        return instance;
    }

    void Init();
    void Shutdown();
    void Run();
    void SetScene(Scene* newScene);
    Scene* GetScene(SceneID id);
	Scene* GetCurrentScene() { return scene; }
    SceneID GetCurrentSceneID() const {
        if (scene == &titleScene) return SceneID::Title;
        if (scene == &gameScene) return SceneID::Game;
        if (scene == &resultScene) return SceneID::Result;
#ifndef NDEBUG
        if (scene == &shaderTestScene) return SceneID::ShaderTest;
        if (scene == &ssFlipTestScene) return SceneID::SsFlipTest;
		if (scene == &roomTestScene) return SceneID::NekoMotionTest;
		if (scene == &statusTestScene) return SceneID::StatusTest;
        if (scene == &modelConfigScene) return SceneID::ModelConfig;
	    if (scene == &gameTestScene) return SceneID::GameTest;
#endif
        return SceneID::Title; // デフォルトはタイトルシーン
	}

    GameContext& GetGameState() { return gameContext; }

    // コピー禁止
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    struct RunConfig
    {
        bool windowed = true;           // Releaseでも切替可
        bool enableDebugUI = true;      // Debug専用にしたいなら main で false にする
        int fpsCap = Const::FPS_CAP;
    };
    void SetRunConfig(const RunConfig& cfg) { runConfig = cfg; }

private:
    SceneManager() = default;
    ~SceneManager() = default;

    GameContext gameContext;
    TitleScene  titleScene{ &gameContext };
    GameScene   gameScene{ &gameContext };
    ResultScene resultScene{ &gameContext };
#ifndef NDEBUG
    ShaderTest shaderTestScene{ &gameContext };
    TestScene ssFlipTestScene{ &gameContext };
    RoomTestScene roomTestScene{ &gameContext };
	StatusTest statusTestScene{ &gameContext };
    ModelConfigScene modelConfigScene{ &gameContext };
    GameTest gameTestScene{ &gameContext };
#endif
    Scene* scene = nullptr; // 現在のシーン
    bool wasF11Down = false;

    RunConfig runConfig{};
    DebugUI debugUI;
};
inline SceneManager& SM() { return SceneManager::GetInstance(); } // ショートカット
