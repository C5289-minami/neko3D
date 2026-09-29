#pragma once
#include <functional>
#include <vector> 
#include "DxPlus/DxPlus.h"
#include "ResourceKeys.h"
#include "Scene.h"
#include "Button.h"
#include "UIBase.h"

enum class GameUIType
{
	Default,		// デフォルトのUI（ゲーム中のUI）
	Pause,			// ポーズUI
	GameOver,		// ゲームオーバーUI
	None,			// UIなし
};

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

	void AddUI(std::unique_ptr<UIBase> ui)
	{
		UIs.push_back(std::move(ui));
	}
	/*void AddButton(std::unique_ptr<Button> button)
	{
		buttons.push_back(std::move(button));
	}*/
	GameUIType GetCurrentUIType() const { return currentUIType; }
	void SetCurrentUIType(GameUIType type) { currentUIType = type; }


private:
	std::vector<std::unique_ptr<UIBase>> UIs;
	//std::vector<std::unique_ptr<Button>> buttons;
	GameUIType currentUIType{ GameUIType::None };
};

inline UIManager& UIM()
{
	return UIManager::GetInstance();
}