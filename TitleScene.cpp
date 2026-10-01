// =============================
// Scenes/Title/TitleScene.cpp
// =============================
#include "TitleScene.h"
#include "ResourceManager.h"
#include "SceneManager.h"
#include "ResourceKeys.h"

void TitleScene::Init()
{
    DxLib::SetBackgroundColor(199, 18, 21);
//    frameCount = 0;
    fontHandle = RM().GetFont(ResourceKeys::Font_Title);
    StartFadeIn();

    blinkTimer = BLINK_INTERVAL;
    isPushEnterVisible = false;
}

void TitleScene::Update(float deltaTime)
{
#ifndef NDEBUG
    if (DxLib::CheckHitKey(KEY_INPUT_F1))
    {
        SetNextScene(SceneManager::GetInstance().GetScene(SceneID::SsFlipTest));
        StartFadeOut();
        return;
    }
#endif

    using namespace DxPlus::Input;
    if (GetButtonDown(PLAYER1) & BUTTON_START)
    {
        Scene* gameScene = SceneManager::GetInstance().GetScene(SceneID::Game);
        SetNextScene(gameScene);
        StartFadeOut();
        return;
    }

    // PushEnter‚Ì“_–Å
    blinkTimer -= deltaTime;
    if (blinkTimer <= 0.0f)
    {
        blinkTimer += BLINK_INTERVAL;
        isPushEnterVisible = !isPushEnterVisible;
    }
}

void TitleScene::Render() const
{
    const int white = DxLib::GetColor(255, 255, 255);
    DxPlus::Text::DrawString(L"3D GameProgramming II",
        { DxPlus::CLIENT_WIDTH * 0.5f, DxPlus::CLIENT_HEIGHT * 0.25f },
        white, DxPlus::Text::TextAlign::MIDDLE_CENTER, { 2, 2 }, 0, fontHandle);

    const int yellow = DxLib::GetColor(255, 255, 0);
    if (isPushEnterVisible)
    {
        DxPlus::Text::DrawString(L"Push Enter",
            { DxPlus::CLIENT_WIDTH * 0.5f, DxPlus::CLIENT_HEIGHT * 0.75f },
            yellow, DxPlus::Text::TextAlign::MIDDLE_CENTER, { 1,1 }, 0, fontHandle);
    }

#ifndef NDEBUG
    DxPlus::Text::DrawString(L"F1: SS Flip Test",
        { DxPlus::CLIENT_WIDTH * 0.5f, DxPlus::CLIENT_HEIGHT * 0.85f },
        white, DxPlus::Text::TextAlign::MIDDLE_CENTER, { 0.8f, 0.8f }, 0, fontHandle);
#endif
}
