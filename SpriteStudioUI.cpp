#include "SpriteStudioUI.h"
#include "ResourceManager.h"
#include <algorithm>
#include <cmath>

SpriteStudioUI::SpriteStudioUI(const std::wstring& key, DxPlus::Vec2 pos,
    DxPlus::Vec2 dir, float motionDuration, const std::string& animationName)
    : UIBase(key, pos, dir, motionDuration),
      ownedPlayer(RM().CreateSpriteStudioPlayerInstance(key)),
      player(ownedPlayer.get())
{
    StartAnimation(animationName);
}

SpriteStudioUI::SpriteStudioUI(const std::wstring& key, ss::Player* spriteStudioPlayer, DxPlus::Vec2 pos,
    DxPlus::Vec2 dir, float motionDuration)
    : UIBase(key, pos, dir, motionDuration), player(spriteStudioPlayer)
{
    StartAnimation("");
}

void SpriteStudioUI::StartAnimation(const std::string& animationName)
{
    if (!player) return;

    player->reset();
    const auto animations = RM().GetSpriteStudioAnimationNames(key);
    if (animations.empty()) return;

    std::size_t animationIndex = 0;
    if (!animationName.empty())
    {
        for (std::size_t i = 0; i < animations.size(); ++i)
        {
            const auto& name = animations[i];
            if (name == animationName ||
                (name.size() > animationName.size() &&
                 name.compare(name.size() - animationName.size(), animationName.size(), animationName) == 0 &&
                 name[name.size() - animationName.size() - 1] == '/'))
            {
                animationIndex = i;
                break;
            }
        }
    }
    player->play(animations[animationIndex], 0, 0);
    player->setStep(1.0f);
}

void SpriteStudioUI::SetProgress(float value)
{
    progress = std::clamp(value, 0.0f, 1.0f);
    progressControlled = true;
}

void SpriteStudioUI::PlayAnimation(const std::string& animationName)
{
    progressControlled = false;
    StartAnimation(animationName);
}

void SpriteStudioUI::Update(float deltaTime)
{
    UIBase::Update(deltaTime);
    if (!player) return;

    player->setPosition(position.x, position.y);
    player->setScale(scale, scale);
    player->setAlpha(alpha);
    if (progressControlled)
    {
        const int totalFrames = player->getTotalFrame();
        if (totalFrames > 0)
        {
            player->setFrameNo(static_cast<int>(std::round(progress * (totalFrames - 1))));
        }
        player->update(0.0f);
    }
    else
    {
        player->update(deltaTime);
    }
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
