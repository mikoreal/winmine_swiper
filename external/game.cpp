#include "game.h"
#include "proc.h"
#include <print>

MinesweeperGame::MinesweeperGame()
{
	MinesweeperGame::loadProcHandle();
	MinesweeperGame::loadConfig();
	MinesweeperGame::loadMines();
}

void MinesweeperGame::loadProcHandle()
{
	this->procInfo.procId = GetProcId(PROC_NAME);
	this->procInfo.modBase = GetModuleBaseAddress(this->procInfo.procId, PROC_NAME);
	this->procInfo.hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, this->procInfo.procId);
}

void MinesweeperGame::loadConfig()
{
	// height 0x56A8
	// width 0x5334
	uintptr_t heightWidthAddys[2] = { 0x56A8, 0x5334 };

	ReadProcessMemory(this->procInfo.hProcess, (BYTE*)(this->procInfo.modBase + heightWidthAddys[0]), &this->config.heightWidth.height, sizeof(this->config.heightWidth.height), nullptr);
	ReadProcessMemory(this->procInfo.hProcess, (BYTE*)(this->procInfo.modBase + heightWidthAddys[1]), &this->config.heightWidth.width, sizeof(this->config.heightWidth.width), nullptr);

	//std::cout << "Loaded config\nheight: " << this->config.heightWidth.height << "\nwidth: " << this->config.heightWidth.width << '\n';
}

// rows are separated by 0x10 byte
// +0x20 bytes offset each row
void MinesweeperGame::loadMines()
{
	uintptr_t pFirstElementCurrentRow = 0x5361;
	size_t rowLength = this->config.heightWidth.width;

	for (int i = 0; i < this->config.heightWidth.height; ++i)
	{
		this->gameInfo.minefieldVec.push_back(
			Memory::ReadBytes(this->procInfo.hProcess, this->procInfo.modBase + pFirstElementCurrentRow, rowLength)
		);

		pFirstElementCurrentRow += 0x20;
	}

	for (int i = 0; i < this->config.heightWidth.height; ++i)
	{
		for (int j = 0; j < rowLength; ++j)
		{
			std::byte element = this->gameInfo.minefieldVec.at(i).at(j);
			int value = std::to_integer<int>(element);

			if (value == 0x8F)
			{
				// winmine mine table starts indexing at 1
				this->gameInfo.mineIdxVec.push_back({ i + 1, j + 1 });
			}
		}
	}
}

void MinesweeperGame::printMines()
{
	std::println("Mines indices:");
	for (auto element : this->gameInfo.mineIdxVec)
	{
		std::print("[{}, {}]\t", element.height, element.width);
	}
}

void MinesweeperGame::callChangeFieldState()
{
	HANDLE hProc{ this->procInfo.hProcess };

	// why the fuck is this 0x1000?
	DWORD dwBufferSize = 0x1000;

	void* pMemory = VirtualAllocEx(hProc, NULL, dwBufferSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!pMemory)
	{
		std::print("Failed to allocate memory");
		return;
	}

	//printf("Allocated memory: %p\nPress any key to continue...\n", pMemory);
	//std::cin.get();

	// why is this 0x200?
	constexpr DWORD dwCodeSize = 0x200;

	unsigned char shellcode[dwCodeSize] = {
		0xB8, 0x00 ,0x00, 0x00, 0x00,	//mov eax, 0	  // move address of arguments into eax
		0xFF, 0x30,						//push [eax]	  // push first arg
		0xFF, 0x70, 0x04,				//push [eax+4]	  // push second 
		0xB8, 0x00, 0x00, 0x00, 0x00,	//mov eax, 0	  // move address of target function into eax
		0xFF, 0xD0,						//call eax		  // call function
		//	0x83, 0xC4, 0x08,			//commented out because its actually stdcall //add esp, 8	  // clean stack (cdecl function, we're responsible for this)
		0xC3,							//ret			  // return
		//	0x90						//nop			  // just for alignment.
	};

	int scArgsOffset = 1;
	int scFuncAddress = 11;

	*(uintptr_t*)(&shellcode[scArgsOffset]) = (uintptr_t)pMemory;
	*(uintptr_t*)(&shellcode[scFuncAddress]) = this->procInfo.modBase + CHANGE_FIELD_STATE_FUNCTION_OFFSET;

	struct _Args {
		int row{}; //arg1
		int column{}; //arg2
		//char a3{}; // arg3
		//int retn{}; // retu rn
	};

	printf("Writing shellcode to memory 1337 :DDD\n");
	void* pCode = (void*)((uintptr_t)pMemory + 0x8); // 2 args both 4 bytes
	if (!WriteProcessMemory(hProc, pCode, shellcode, dwCodeSize, nullptr))
	{
		printf("Mission failed while injecting shellcode...");
		//VirtualFreeEx(hProc, pMemory, dwBufferSize, MEM_RELEASE);
		VirtualFreeEx(hProc, pMemory, NULL, MEM_RELEASE);
		return;
	}

	for (auto targetField : this->gameInfo.mineIdxVec)
	{
		_Args args = { targetField.height, targetField.width };

		std::println("Writing args {{ {}, {} }} to process memory...", args.column, args.row);
		if (!WriteProcessMemory(hProc, pMemory, &args, sizeof(args), nullptr))
		{
			printf("Mission failed while injecting args...");
			//Passing MEM_RELEASE and a non-zero dwSize parameter to VirtualFree is not allowed.
			//This results in the failure of this call.
			//VirtualFreeEx(hProc, pMemory, dwBufferSize, MEM_RELEASE);
			VirtualFreeEx(hProc, pMemory, NULL, MEM_RELEASE);
			return;
		}

		DWORD threadAddy{};
		HANDLE hThread = CreateRemoteThread(hProc, NULL, NULL, (LPTHREAD_START_ROUTINE)pCode, NULL, CREATE_SUSPENDED, &threadAddy);
		if (!hThread)
		{
			printf("Mission failed while creating remote thread...");
			VirtualFreeEx(hProc, pMemory, NULL, MEM_RELEASE);
			return;
		}

		ResumeThread(hThread);
		WaitForSingleObject(hThread, INFINITE);
	}

	VirtualFreeEx(hProc, pMemory, NULL, MEM_RELEASE);
}
