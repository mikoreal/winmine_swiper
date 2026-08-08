#include <array>
#include <cstdint>
#include <print>
#include <string_view>
#include <Windows.h>
#include "process_memory.hpp"
#include "game.h"

using std::uint8_t;

constexpr std::string_view BANNER = R"art(
           _                    _
__      __(_) _ __   _ __ ___  (_) _ __    ___
\ \ /\ / /| || '_ \ | '_ ` _ \ | || '_ \  / _ \
 \ V  V / | || | | || | | | | || || | | ||  __/
  \_/\_/  |_||_| |_||_| |_| |_||_||_| |_| \___|
            _             _
           | | __      __(_) _ __    ___   _ __
          / __)\ \ /\ / /| || '_ \  / _ \ | '__|
          \__ \ \ V  V / | || |_) ||  __/ | |
          (   /  \_/\_/  |_|| .__/  \___| |_|
           |_|              |_|

                            made by @mikoreal
)art";

auto isKeyDown(uint8_t vkCode) -> bool { return GetAsyncKeyState(vkCode) & 0x8000; }

auto wasKeyPressed(uint8_t vkCode) -> bool
{
	static std::array<bool, 256> wasDown{};
	const bool isDown = isKeyDown(vkCode);
	const bool pressed = isDown && !wasDown[vkCode];
	wasDown[vkCode] = isDown;
	return pressed;
}

int main()
{
	std::println("{}", BANNER);

	while (true)
	{
		if (wasKeyPressed(VK_F2))
		{
			MineSwiper minesweeper;
			minesweeper.printMineMap();
		}
		else if (wasKeyPressed(VK_F3))
		{
			MineSwiper minesweeper;
			if (const auto result = minesweeper.flagMines(); !result)
				std::println(stderr, "{}", result.error());
		}
		else if (isKeyDown(VK_END))
		{
			break;
		}

		Sleep(10);
	}

	return 0;
}
