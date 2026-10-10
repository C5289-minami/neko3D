#include "DebugScene.h"
#include "ResourceManager.h"
#include "ResourceKeys.h"
#include "SceneManager.h"


// SceneのInit処理等でプレイヤー初期化
void TestScene::Init()
{
    // キャラクター
    m_ssPlayer = RM().GetSpriteStudioPlayer(
        ResourceKeys::SpriteStudio_TitleCharacter);

    flipTestMode = 0;

    if (m_ssPlayer)
    {
        m_ssPlayer->reset();

        // アニメーション一覧を自動取得
        const auto animations =
            RM().GetSpriteStudioAnimationNames(
                ResourceKeys::SpriteStudio_TitleCharacter);

        // 「/walk」で終わるアニメーションを探す
        for (std::size_t i = 0; i < animations.size(); ++i)
        {
            const std::string& name = animations[i];
            constexpr char suffix[] = "/walk";
            constexpr std::size_t suffixLength = sizeof(suffix) - 1;

            if (name.size() >= suffixLength &&
                name.compare(
                    name.size() - suffixLength,
                    suffixLength,
                    suffix) == 0)
            {
                // 見つかったインデックスで再生
                RM().PlaySpriteStudioAnimation(
                    ResourceKeys::SpriteStudio_TitleCharacter,
                    i,
                    0,      // 無限ループ
                    1.0f);  // 通常速度

                break;
            }
        }

        m_ssPlayer->setPosition(
            DxPlus::CLIENT_WIDTH * 0.5f,
            DxPlus::CLIENT_HEIGHT * 0.6f);

        m_ssPlayer->setScale(0.5f, 0.5f);
        m_ssPlayer->update(0.0f);
    }

    // ゲージ
    gaugeAnim = RM().GetSpriteStudioPlayer(
        ResourceKeys::SpriteStudio_Gauge);

    if (gaugeAnim)
    {
        gaugeAnim->reset();

        // barパックの「bar」アニメーションを再生
        RM().PlaySpriteStudioAnimation(
            ResourceKeys::SpriteStudio_Gauge,
            1,      // 0: Setup、1: bar
            0,      // 無限ループ
            1.0f);  // 通常速度

        gaugeAnim->setPosition(
            DxPlus::CLIENT_WIDTH * 0.5f,
            DxPlus::CLIENT_HEIGHT * 0.5f);

        gaugeAnim->setScale(0.5f, 0.5f);
        gaugeAnim->update(0.0f);

        gauge.SetPlayer(*gaugeAnim);
    }

    StartFadeIn();
}
void TestScene::Update(float deltaTime)
{
	if (DxLib::CheckHitKey(KEY_INPUT_F2))
	{
		SetNextScene(SM().GetScene(SceneID::Title));
		StartFadeOut();
		return;
	}

	if (DxLib::CheckHitKey(KEY_INPUT_1)) flipTestMode = 0;
	if (DxLib::CheckHitKey(KEY_INPUT_2)) flipTestMode = 1;
	if (DxLib::CheckHitKey(KEY_INPUT_3)) flipTestMode = 2;

	if (!m_ssPlayer) return;

	const float scale = 0.5f;
	m_ssPlayer->setPosition(DxPlus::CLIENT_WIDTH * 0.5f, DxPlus::CLIENT_HEIGHT * 0.5f);
	if (flipTestMode == 0)
	{
		m_ssPlayer->setScale(scale, scale);
	}
	else if (flipTestMode == 1)
	{
		m_ssPlayer->setScale(-scale, scale);
	}
	else
	{
		m_ssPlayer->setScale(-scale, -scale);
	}
	m_ssPlayer->update(deltaTime);

	if (!gaugeAnim) return;
	//gaugeAnim->update(deltaTime);
	static float progress = 0.0f;
	progress += deltaTime; // 進捗率を時間経過で増加させる例
	if (progress > 1.0f) progress = 0.0f; // 進捗率が1を超えたらリセット
	gauge.Update(deltaTime, progress); // 進捗率を0.5に設定
	gaugeAnim->update(0.0f);
}

void TestScene::Render() const
{
	if (!m_ssPlayer) return;
	m_ssPlayer->draw();
	if (!gaugeAnim) return;
	gaugeAnim->draw();

	const wchar_t* modeText = flipTestMode == 0 ? L"1: Normal" :
		flipTestMode == 1 ? L"2: Horizontal flip" : L"3: Horizontal + vertical flip";
	DxPlus::Text::DrawString(modeText, { 20.0f, 20.0f }, DxLib::GetColor(255, 255, 255));
	DxPlus::Text::DrawString(L"F2: Return to title", { 20.0f, 50.0f }, DxLib::GetColor(255, 255, 255));
}