#include "DebugScene.h"
#include "ResourceManager.h"
#include "ResourceKeys.h"
#include "SceneManager.h"
#include "StatusUI.h"

// Scene‚ÌInitˆ—“™‚ÅƒvƒŒƒCƒ„[‰Šú‰»
void TestScene::Init()
{
    DxLib::SetBackgroundColor(gameContext->GetBgRed(), gameContext->GetBgGreen(), gameContext->GetBgBlue());

	StatusUI::InitBoss();
    StatusUI::InitPlayer();

    StartFadeIn();
}
void TestScene::Update(float deltaTime)
{
    DxLib::SetBackgroundColor(gameContext->GetBgRed(), gameContext->GetBgGreen(), gameContext->GetBgBlue());
	static float aaa = 0.0f;
	if (DxLib::CheckHitKey(KEY_INPUT_UP)) {
		aaa += 0.01f;
		if (aaa > 1.0f) aaa = 0.0f;
	}

	StatusUI::SyncStatusBar(L"BossHealthGauge", aaa);
	StatusUI::SyncStatusBar(L"PlayerUltGauge",  aaa);
	StatusUI::SyncStatusBar(L"BossHealthGauge", aaa);
	StatusUI::SyncStatusBar(L"BossDownGauge",   aaa);
}

void TestScene::Render() const
{

	const wchar_t* modeText = flipTestMode == 0 ? L"1: Normal" :
		flipTestMode == 1 ? L"2: Horizontal flip" : L"3: Horizontal + vertical flip";
	DxPlus::Text::DrawString(modeText, { 20.0f, 20.0f }, DxLib::GetColor(255, 255, 255));
	DxPlus::Text::DrawString(L"F2: Return to title", { 20.0f, 50.0f }, DxLib::GetColor(255, 255, 255));
}