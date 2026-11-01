#pragma once
#pragma once
#include "UIBase.h"
#include "DxPlus/DxPlus.h"
#include "Button.h"
#include <functional>
#include "ButtonLabel.h"

class ButtonUI : public UIBase
{
public:
	ButtonUI(const std::wstring& key, DxPlus::Vec2 pos,
		std::function<void()> onclick,
		DxPlus::Vec2 dir = DxPlus::Vec2(0, 0),
		float motionDuration = 0.0f)
		: UIBase(key, pos, dir, motionDuration),
		button(pos, key, std::move(onclick))
	{
	};
	void Update(float deltaTime) override
	{
		pressed = false;
		UIBase::Update(deltaTime);

		button.SetAnchorPosition(position, center);
		button.SetAlpha(alpha);

		if (state != UIState::Normal)
			return;

		DxPlus::Vec2 mousePos = DxPlus::Input::GetMousePositionF();
		bool clicked = (DxLib::GetMouseInput() & MOUSE_INPUT_LEFT) != 0;
		pressed = button.Update(mousePos, clicked);
	}
	bool IsPressed() const { return pressed; }
	void Draw() override
	{
		if (state == UIState::False) return;

		if (state != UIState::Normal)
			UIBase::Draw();
		else
			button.Draw();

		// âÊëúÇÃíÜêSÇäÓèÄÇ…ï∂éöÇï`âÊ
		text_.Draw(button.GetPosition(), scale, alpha);
	}
	void SetButtonHoverSprite(const std::wstring& key) { button.SetHoverSprite(key); }
	void SetScale(float s)override {
		UIBase::SetScale(s);
		button.SetScale(s);
	};
	void SetText(const std::wstring& text) { text_.SetText(text); }
	const std::wstring& GetText() const { return text_.GetText(); }

	void SetTextFont(int handle) { text_.SetFontHandle(handle); }
	void SetTextColor(int color) { text_.SetColor(color); }
	void SetTextAlign(DxPlus::Text::TextAlign align) { text_.SetAlign(align); }
	void SetTextOffset(DxPlus::Vec2 offset) { text_.SetOffset(offset); }
	void SetTextScale(float scale) { text_.SetScale(scale); }


private:
	Button button;
	ButtonLabel text_;
	bool pressed{ false };
};