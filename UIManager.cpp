#include "UIManager.h"

#include <algorithm>
#include <utility>

// ============================================================
// UI creation
// ============================================================

UIBase* UIManager::CreateImageUI(
    const std::wstring& id,
    const std::wstring& resourceKey,
    DxPlus::Vec2 position,
    DxPlus::Vec2 moveDirection,
    float motionDuration)
{
    auto ui = std::make_unique<UIBase>(
        resourceKey,
        position,
        moveDirection,
        motionDuration
    );

    UIBase* result = ui.get();

    if (!RegisterUI(id, std::move(ui)))
    {
        return nullptr;
    }

    return result;
}

ButtonUI* UIManager::CreateButtonUI(
    const std::wstring& id,
    const std::wstring& resourceKey,
    DxPlus::Vec2 position,
    std::function<void()> onClick,
    DxPlus::Vec2 moveDirection,
    float motionDuration)
{
    auto ui = std::make_unique<ButtonUI>(
        resourceKey,
        position,
        std::move(onClick),
        moveDirection,
        motionDuration
    );

    ButtonUI* result = ui.get();

    if (!RegisterUI(id, std::move(ui)))
    {
        return nullptr;
    }

    return result;
}

SpriteStudioUI* UIManager::CreateSpriteStudioUI(
    const std::wstring& id,
    const std::wstring& resourceKey,
    DxPlus::Vec2 position,
    const std::string& animationName,
    DxPlus::Vec2 moveDirection,
    float motionDuration)
{
    auto ui = std::make_unique<SpriteStudioUI>(
        resourceKey,
        position,
        moveDirection,
        motionDuration,
        animationName
    );

    SpriteStudioUI* result = ui.get();

    if (!RegisterUI(id, std::move(ui)))
    {
        return nullptr;
    }

    return result;
}

// ============================================================
// UI access
// ============================================================

UIBase* UIManager::GetUI(const std::wstring& id)
{
    const auto it = std::find_if(
        UIs.begin(),
        UIs.end(),
        [&id](const UIEntry& entry)
        {
            return entry.id == id;
        }
    );

    if (it == UIs.end())
    {
        return nullptr;
    }

    return it->object.get();
}

ButtonUI* UIManager::GetButtonUI(const std::wstring& id)
{
    return dynamic_cast<ButtonUI*>(GetUI(id));
}

SpriteStudioUI* UIManager::GetSpriteStudioUI(
    const std::wstring& id)
{
    return dynamic_cast<SpriteStudioUI*>(GetUI(id));
}

// ============================================================
// UI registration
// ============================================================

bool UIManager::RegisterUI(
    const std::wstring& id,
    std::unique_ptr<UIBase> ui)
{
    if (id.empty() || !ui || GetUI(id))
    {
        return false;
    }

    UIs.push_back({
        id,
        std::move(ui)
        });

    return true;
}

bool UIManager::RemoveUI(const std::wstring& id)
{
    const auto it = std::find_if(
        UIs.begin(),
        UIs.end(),
        [&id](const UIEntry& entry)
        {
            return entry.id == id;
        }
    );

    if (it == UIs.end())
    {
        return false;
    }

    /*
        TwoFrameUIAnimatorはUIBaseへの生ポインタを保持している。

        どのAnimatorがこのUIを参照しているかは現在の構造では
        UIManager側から判定できないため、UIを個別削除する場合は、
        先に全Animatorを停止して破棄する。
    */
    for (auto& entry : twoFrameAnimators_)
    {
        if (entry.animator)
        {
            entry.animator->Stop();
        }
    }

    twoFrameAnimators_.clear();

    UIs.erase(it);
    return true;
}

// ============================================================
// Two-frame animator
// ============================================================

TwoFrameUIAnimator* UIManager::CreateTwoFrameAnimator(
    const std::wstring& id)
{
    if (id.empty())
    {
        return nullptr;
    }

    if (auto* existing = GetTwoFrameAnimator(id))
    {
        return existing;
    }

    auto animator =
        std::make_unique<TwoFrameUIAnimator>();

    TwoFrameUIAnimator* result =
        animator.get();

    twoFrameAnimators_.push_back({
        id,
        std::move(animator)
        });

    return result;
}

TwoFrameUIAnimator* UIManager::GetTwoFrameAnimator(
    const std::wstring& id)
{
    const auto it = std::find_if(
        twoFrameAnimators_.begin(),
        twoFrameAnimators_.end(),
        [&id](const TwoFrameAnimatorEntry& entry)
        {
            return entry.id == id;
        }
    );

    if (it == twoFrameAnimators_.end())
    {
        return nullptr;
    }

    return it->animator.get();
}

bool UIManager::RemoveTwoFrameAnimator(
    const std::wstring& id)
{
    const auto it = std::find_if(
        twoFrameAnimators_.begin(),
        twoFrameAnimators_.end(),
        [&id](const TwoFrameAnimatorEntry& entry)
        {
            return entry.id == id;
        }
    );

    if (it == twoFrameAnimators_.end())
    {
        return false;
    }

    if (it->animator)
    {
        it->animator->Stop();
    }

    twoFrameAnimators_.erase(it);
    return true;
}

// ============================================================
// Lifecycle
// ============================================================

void UIManager::Init()
{
    Clear();
}

void UIManager::Update(float deltaTime)
{
    /*
        先にAnimatorを更新する。

        AnimatorがUIの以下の値を変更する。
        ・表示状態
        ・位置
        ・回転
        ・拡大率

        その後にUIBase::Updateを実行する。
    */
    for (auto& entry : twoFrameAnimators_)
    {
        if (entry.animator)
        {
            entry.animator->Update(deltaTime);
        }
    }

    for (auto& entry : UIs)
    {
        if (entry.object)
        {
            entry.object->Update(deltaTime);
        }
    }
}

void UIManager::Draw(int drawZOrder)
{
    std::stable_sort(
        UIs.begin(),
        UIs.end(),
        [](const UIEntry& left, const UIEntry& right)
        {
            if (!left.object)
            {
                return false;
            }

            if (!right.object)
            {
                return true;
            }

            return left.object->GetZOrder() <
                right.object->GetZOrder();
        }
    );

    for (auto& entry : UIs)
    {
        if (!entry.object)
        {
            continue;
        }

        if (drawZOrder != -1 &&
            entry.object->GetZOrder() != drawZOrder)
        {
            continue;
        }

        entry.object->Draw();
    }
}

void UIManager::Clear()
{
    /*
        AnimatorはUIBaseへの生ポインタを保持しているため、
        必ずAnimatorを先に停止して破棄する。
    */
    for (auto& entry : twoFrameAnimators_)
    {
        if (entry.animator)
        {
        }
    }

    twoFrameAnimators_.clear();

    // Animatorを破棄した後でUIを破棄する。
    UIs.clear();
}