#include <cstdint>
#include <iostream>
#include <Windows.h>
#include "proc.h"
#include "game.h"

using std::uint8_t;

auto isKeyDown(uint8_t vkCode) -> bool
{ return GetAsyncKeyState(vkCode) & 0x8000; }

int main()
{
	while (true)
	{
		if (isKeyDown(VK_F3))
		{
			MinesweeperGame minesweeper;
			minesweeper.callChangeFieldState();
		}
		else if (isKeyDown(VK_END))
		{
			break;
		}
	}

	return 0;
}

