#include "SpriteStudioUI.h"

SpriteStudioUI::SpriteStudioUI(const std::wstring& key, ss::Player* spriteStudioPlayer, DxPlus::Vec2 pos,
    DxPlus::Vec2 dir, float motionDuration)
    : UIBase(key, pos, dir, motionDuration), player(spriteStudioPlayer)
{
}

void SpriteStudioUI::Update(float deltaTime)
{
    UIBase::Update(deltaTime);
    if (!player) return;

    player->setPosition(position.x, position.y);
    player->setScale(scale, scale);
    player->setAlpha(alpha);
    player->update(deltaTime);
}

void SpriteStudioUI::Draw()
{
    if (player && state != State::False)
    {
        player->draw();
    }
}

void SpriteStudioUI::SetScale(float targetScale)
{
    UIBase::SetScale(targetScale);
    if (player)
    {
        player->setScale(targetScale, targetScale);
    }
}
