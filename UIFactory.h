#pragma once
#include "UIManager.h"

// ‚±‚±‚ÍUI‚ğ‚Ü‚Æ‚ß‚Äì¬‚·‚éƒNƒ‰ƒX
class UIFactory
{
public:
	static void CreateTitleUI(UIManager& manager);
	static void CreateGameUI(UIManager& manager);
	static void CreateResultUI(UIManager& manager);
	inline static bool isOpenRule = true;
};
