#pragma once
#include <DxLib.h>

namespace DxPlus
{
	struct Color
	{
		unsigned char r, g, b, a;

		constexpr Color(
			unsigned char r = 255, 
			unsigned char g = 255, 
			unsigned char b = 255, 
			unsigned char a = 255
		) : r(r), g(g), b(b), a(a) {}

		DxLib::COLOR_U8 ToColorU8() const
		{
			DxLib::COLOR_U8 c{};
			c.r = r;
			c.g = g;
			c.b = b;
			c.a = a;
			return c;
		}
	};
}
