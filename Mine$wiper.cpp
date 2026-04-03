#include <iostream>
#include <Windows.h>
#include "proc.h"
#include "MinesweeperGame.h"

int main()
{
	while (true)
	{
		if (GetAsyncKeyState(VK_F3))
		{
			MinesweeperGame minesweeper;
			minesweeper.callChangeFieldState();
		}
		else if (GetAsyncKeyState(VK_END))
		{
			break;
		}
	}

	return 0;
}

