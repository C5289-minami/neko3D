#pragma once
#pragma once
#include "UIBase.h"
#include "DxPlus/DxPlus.h"
#include "Button.h"
#include <functional>

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
	}
	void Update(float deltaTime) override
	{
		UIBase::Update(deltaTime);
       button.SetPosition(position);
		button.SetAlpha(this->alpha);
		
		if(state != State::Normal)
		{
			return;
		}
		DxPlus::Vec2 mousePos = DxPlus::Input::GetMousePositionF();
		bool clicked = (DxLib::GetMouseInput() & MOUSE_INPUT_LEFT) != 0;
		button.Update(mousePos, clicked);

	}
	void Draw() override
	{
		if (state == State::False) return;
		if (state != State::Normal) {
			// モーション中は通常の画像を描画
			UIBase::Draw();
		}
		else {
			// 通常時はホバー判定付きのボタン描画
			button.Draw();
		}
	
	}
   void SetButtonHoverSprite(const std::wstring& key) { button.SetHoverSprite(key); }
	void SetScale(float s)override { 
		UIBase::SetScale(s);
		button.SetScale(s); 
	};
private:
	Button button;
};