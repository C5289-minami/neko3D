#include "TutorialChecklist.h"
#include "UIManager.h"

#include <algorithm>

namespace
{
	float randamOffsetX = 100.0f;
}

void TutorialChecklist::Init()
{
	items.clear();

	for (const auto& stampId : stampIds)
		UIM().RemoveUI(stampId);
	stampIds.clear();
	isVisible = false;

	panelPosition = DxPlus::Vec2(DxPlus::CLIENT_WIDTH * 0.25f, DxPlus::CLIENT_HEIGHT * 0.5f);


	if (!UIM().GetUI(L"TutorialList"))
	{
		auto* list = UIM().CreateImageUI(L"TutorialList", ResourceKeys::TutorialBack, panelPosition);
		list->SetVisible(false);
	}

}

void TutorialChecklist::Update(float deltaTime)
{
	if (!isVisible) return;
	(void)deltaTime;
}

void TutorialChecklist::Draw() const
{
	if (!isVisible || items.empty()) return;
}

void TutorialChecklist::AddItem(const std::wstring& text, DxPlus::Vec2 stampPosition)
{
	ChecklistItem item;
	item.text = text;
	item.stampPosition = stampPosition;
	item.stampPosition.x += (DxLib::GetRand(randamOffsetX * 2.0f) - randamOffsetX); // ランダムなXオフセットを追加

	items.push_back(std::move(item));
}

void TutorialChecklist::CompleteItem(std::size_t index)
{
	if (index >= items.size()) return;

	auto& item = items[index];

	if (item.completed) return;
	item.completed = true;

	const std::wstring stampId = L"TutorialStamp" + std::to_wstring(index);
	if (auto* stamp = UIM().GetUI(stampId))
	{
		stamp->SetPosition(panelPosition + item.stampPosition);
		stamp->SetVisible(isVisible);
	}
	else
	{
      auto* createdStamp = UIM().CreateImageUI(
			stampId,
			ResourceKeys::TutorialStamp,
			panelPosition + item.stampPosition);
       createdStamp->SetVisible(isVisible);
	}

	stampIds.push_back(stampId);
}

void TutorialChecklist::SetVisible(bool visible)
{
	if (isVisible == visible) return;
	isVisible = visible;



	if (auto* list = UIM().GetUI(L"TutorialList"))
	{
		if (!isVisible)list->Hide();
		else list->SetVisible(true);
	}
    for (const auto& stampId : stampIds)
	{
       if (auto* stamp = UIM().GetUI(stampId))
		{
			if (!isVisible) stamp->Hide();
			else stamp->SetVisible(true);
		}
	}


}