#include "UIManager.h"
#include "SceneManager.h"
#include "ResourceKeys.h"
#include "Button.h"
#include "DxPlus/DxPlus.h"
#include <algorithm>
#include "ButtonUI.h"
#include <vector>

UIBase* UIManager::CreateImageUI(const std::wstring& id, const std::wstring& resourceKey,
	DxPlus::Vec2 position, DxPlus::Vec2 moveDirection, float motionDuration)
{
	auto ui = std::make_unique<UIBase>(resourceKey, position, moveDirection, motionDuration);
	auto* result = ui.get();
	if (!RegisterUI(id, std::move(ui))) return nullptr;
	return result;
}

ButtonUI* UIManager::CreateButtonUI(const std::wstring& id, const std::wstring& resourceKey,
	DxPlus::Vec2 position, std::function<void()> onClick,
	DxPlus::Vec2 moveDirection, float motionDuration)
{
	auto ui = std::make_unique<ButtonUI>(resourceKey, position, std::move(onClick), moveDirection, motionDuration);
	auto* result = ui.get();
	if (!RegisterUI(id, std::move(ui))) return nullptr;
	return result;
}

SpriteStudioUI* UIManager::CreateSpriteStudioUI(const std::wstring& id,
	const std::wstring& resourceKey, DxPlus::Vec2 position, const std::string& animationName,
	DxPlus::Vec2 moveDirection, float motionDuration)
{
	auto ui = std::make_unique<SpriteStudioUI>(
		resourceKey, position, moveDirection, motionDuration, animationName);
   auto* result = ui.get();
	if (!RegisterUI(id, std::move(ui))) return nullptr;
	return result;
}

UIBase* UIManager::GetUI(const std::wstring& id)
{
	for (auto& ui : UIs)
	{
        if (ui.id == id)
			return ui.object.get();
	}
	return nullptr;
}

SpriteStudioUI* UIManager::GetSpriteStudioUI(const std::wstring& id)
{
	return dynamic_cast<SpriteStudioUI*>(GetUI(id));
}

ButtonUI* UIManager::GetButtonUI(const std::wstring& id)
{
	return dynamic_cast<ButtonUI*>(GetUI(id));
}

bool UIManager::RegisterUI(const std::wstring& id, std::unique_ptr<UIBase> ui)
{
	if (id.empty() || !ui || GetUI(id)) return false;
	UIs.push_back({ id, std::move(ui) });
	return true;
}

bool UIManager::RemoveUI(const std::wstring& id)
{
   const auto it = std::find_if(UIs.begin(), UIs.end(), [&id](const auto& ui) {
		return ui.id == id;
	});
	if (it == UIs.end()) return false;
	UIs.erase(it);
	return true;
}

void UIManager::Clear()
{
	UIs.clear();
}

void UIManager::Init()
{
    Clear();
}

void UIManager::Update(float deltaTime)
{
	std::vector<std::function<void()>> pendingCallbacks;

	// すべてのUIを更新
	for (auto& ui : UIs)
	{
		ui.object->Update(deltaTime);

		if (auto* button = dynamic_cast<ButtonUI*>(ui.object.get()))
		{
			auto callback = button->TakePendingTrigger();

			if (callback)
				pendingCallbacks.push_back(std::move(callback));
		}
	}

	// UI更新がすべて終わってからコールバックを実行
	for (auto& callback : pendingCallbacks)
		callback();
}
void UIManager::Draw(int drawZOrder)
{
  std::stable_sort(UIs.begin(), UIs.end(), [](const auto& left, const auto& right) {
      return left.object->GetZOrder() < right.object->GetZOrder();
	});
	for (auto& ui : UIs) {
        if (drawZOrder == -1 || ui.object->GetZOrder() == drawZOrder) {
			ui.object->Draw();
		}
	}
}


