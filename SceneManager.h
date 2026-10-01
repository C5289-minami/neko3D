// =============================
// Scenes/Base/SceneManager.h
// =============================
#pragma once
#include "Scene.h"
#include "GameContext.h"
#include "TitleScene.h"
#include "GameScene.h"
#include "ResultScene.h"
#include "BossTestScene.h"
#include "DebugUI.h"
#include "Consts.h"
#include "./SSPlayer/SS6Player.h"

enum class SceneID { Title, Game, Result, BossTest };

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
        if (scene == &bossTestScene) return SceneID::BossTest;
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
    BossTestScene bossTestScene{ &gameContext };

    Scene* scene = nullptr; // 現在のシーン

    RunConfig runConfig{};
    DebugUI debugUI;
};
inline SceneManager& SM() { return SceneManager::GetInstance(); } // ショートカット
