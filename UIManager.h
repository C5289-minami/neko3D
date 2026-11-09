#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "DxPlus/DxPlus.h"
#include "UIBase.h"
#include "ButtonUI.h"
#include "SpriteStudioUI.h"
#include "TwoFrameUIAnimator.h"

class UIManager
{
public:
    UIManager() = default;
    ~UIManager() = default;

    static UIManager& GetInstance()
    {
        static UIManager instance;
        return instance;
    }

    void Init();
    void Update(float deltaTime);
    void Draw(int drawZOrder = -1);

    UIBase* CreateImageUI(
        const std::wstring& id,
        const std::wstring& resourceKey,
        DxPlus::Vec2 position,
        DxPlus::Vec2 moveDirection = DxPlus::Vec2(0, 0),
        float motionDuration = 0.0f
    );

    ButtonUI* CreateButtonUI(
        const std::wstring& id,
        const std::wstring& resourceKey,
        DxPlus::Vec2 position,
        std::function<void()> onClick,
        DxPlus::Vec2 moveDirection = DxPlus::Vec2(0, 0),
        float motionDuration = 0.0f
    );

    SpriteStudioUI* CreateSpriteStudioUI(
        const std::wstring& id,
        const std::wstring& resourceKey,
        DxPlus::Vec2 position,
        const std::string& animationName = "",
        DxPlus::Vec2 moveDirection = DxPlus::Vec2(0, 0),
        float motionDuration = 0.0f
    );

    UIBase* GetUI(const std::wstring& id);
    ButtonUI* GetButtonUI(const std::wstring& id);
    SpriteStudioUI* GetSpriteStudioUI(const std::wstring& id);

    bool RegisterUI(
        const std::wstring& id,
        std::unique_ptr<UIBase> ui
    );

    bool RemoveUI(const std::wstring& id);

    // ========================================================
    // Two-frame animation
    // ========================================================

    TwoFrameUIAnimator* CreateTwoFrameAnimator(
        const std::wstring& id
    );

    TwoFrameUIAnimator* GetTwoFrameAnimator(
        const std::wstring& id
    );

    bool RemoveTwoFrameAnimator(
        const std::wstring& id
    );

    void Clear();

    std::size_t GetUICount() const
    {
        return UIs.size();
    }



private:
    struct UIEntry
    {
        std::wstring id;
        std::unique_ptr<UIBase> object;
    };

    struct TwoFrameAnimatorEntry
    {
        std::wstring id;
        std::unique_ptr<TwoFrameUIAnimator> animator;
    };

    std::vector<UIEntry> UIs;

    std::vector<TwoFrameAnimatorEntry>
        twoFrameAnimators_;
};

inline UIManager& UIM()
{
    return UIManager::GetInstance();
}