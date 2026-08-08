#include "game.h"
#include "common.h"
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <print>
#include <ranges>
#include <string>

MineSwiper::MineSwiper()
{
	attach();
	if (!_process.handle())
	{
		std::println("winmine.exe not found or could not be opened.");
		return;
	}

	readDimensions();
	readBoard();
}

void MineSwiper::attach()
{
	_process = Process(winmine::PE_NAME, winmine::IMAGE_BASE);
}

// board dimensions [ref. 3.2]
void MineSwiper::readDimensions()
{
	const HANDLE    proc = _process.handle();
	const uintptr_t base = _process.modBase();

	const auto width  = memory::readValue<DWORD>(proc, base + winmine::offset::BOARD_WIDTH);
	const auto height = memory::readValue<DWORD>(proc, base + winmine::offset::BOARD_HEIGHT);
	if (!width || !height)
	{
		std::println(stderr, "Failed to read board dimensions (error {})",
			!width ? width.error() : height.error());
		return;
	}

	_board = Board{ static_cast<int>(*width), static_cast<int>(*height) };
}

// modBase + 0x5340 + 32 * row + col
void MineSwiper::readBoard()
{
	namespace rv = std::views;

	const HANDLE    proc = _process.handle();
	const uintptr_t base = _process.modBase();

	for (const int row : rv::iota(1, _board.height() + 1))
	{
		const uintptr_t rowAddr = base + winmine::cellOffset(row, 1);
		memory::read(proc, rowAddr, _board.rowData(row));
	}

	_mines = rv::cartesian_product(rv::iota(1, _board.height() + 1),
	                               rv::iota(1, _board.width() + 1))
		| rv::filter([this](auto rc) {
			const auto [row, col] = rc;
			return (_board[row, col] & winmine::cell::MINE_MASK) != 0;
		})
		| rv::transform([](auto rc) {
			const auto [row, col] = rc;
			return Field{ row, col };
		})
		| std::ranges::to<std::vector>();
}

void MineSwiper::printMines() const
{
	std::println("Mine indices (format: [row, column])");
	for (const Field& mine : _mines)
		std::print("[{}, {}]\t", mine.row, mine.col);
}

// table 3.3.
static char cellChar(byte_t cell)
{
	using namespace winmine::cell;

	if (cell & MINE_MASK)                          
		return (cell & REVEALED_MASK) ? '#' : '*'; // '#' = detonated, '*' = hidden

	const byte_t state = cell & STATE_MASK;       
	if (state >= 0x01 && state <= 0x08)        
		return static_cast<char>('0' + state);

	switch (state)
	{
	case EMPTY:    return (cell & REVEALED_MASK) ? ' ' : '.';
	case FLAG:     return 'F';
	case QUESTION: return '?';
	case BORDER:   return '#';
	default:       return '.';                     
	}
}

// reveals mines hidden from the player
void MineSwiper::printMineMap() const
{
	namespace rv = std::views;

	if (_board.width() <= 0 || _board.height() <= 0)
		return;

	std::println("Minefield map  ('*' mine, '#' detonated, 'F' flag, '?' question mark, '.' covered, digit = adjacent mines):");

	const auto header = rv::iota(1, _board.width() + 1)
		| rv::transform([](int col) { return static_cast<char>('0' + col % 10); })
		| std::ranges::to<std::string>();
	std::println("    {}", header);

	for (auto [i, row] : rv::enumerate(_board.rows()))
		std::println("{:>3} {}", i + 1,
			row | rv::transform(cellChar) | std::ranges::to<std::string>());
}


// flags every mine
// by remotely calling winmine.exe function
// named onRightClickField (+0x374F) function [ref. 3.4]
struct RemoteAlloc {
	HANDLE proc = nullptr;
	void*  ptr  = nullptr;

	~RemoteAlloc() { if (ptr) VirtualFreeEx(proc, ptr, 0, MEM_RELEASE); }
};

std::expected<void, std::string> MineSwiper::flagMines() const
{
	const HANDLE proc = _process.handle();

	// VirtualAllocEx allocates pages of memory
	// 0x1000 (4 KiB) is the smallest page.
	constexpr SIZE_T bufferSize = 0x1000;
	const RemoteAlloc remote{ proc,
		VirtualAllocEx(proc, nullptr, bufferSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE) };
	if (!remote.ptr)
		return std::unexpected("Failed to allocate remote memory");

	constexpr uintptr_t codeOffset = 0x8;
	void* remoteCode = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(remote.ptr) + codeOffset);

	// call onRightClickField(args.row, args.column)
	std::array<unsigned char, 18> shellcode = {
		0xB8, 0, 0, 0, 0,   // mov  eax, <args address>
		0xFF, 0x30,         // push dword [eax]      ; arg1 (row)
		0xFF, 0x70, 0x04,   // push dword [eax + 4]  ; arg2 (column)
		0xB8, 0, 0, 0, 0,   // mov  eax, <function address>
		0xFF, 0xD0,         // call eax
		0xC3,               // ret
	};
	constexpr std::size_t argsImm = 1;   // "mov eax, <args address>"
	constexpr std::size_t funcImm = 11;  // "mov eax, <function address>"

	const auto argsAddr = static_cast<std::uint32_t>(reinterpret_cast<uintptr_t>(remote.ptr));
	const auto funcAddr = static_cast<std::uint32_t>(_process.modBase() + winmine::offset::FN_ON_RIGHT_CLICK_FIELD);
	std::memcpy(&shellcode[argsImm], &argsAddr, sizeof(argsAddr));
	std::memcpy(&shellcode[funcImm], &funcAddr, sizeof(funcAddr));

	if (!WriteProcessMemory(proc, remoteCode, shellcode.data(), shellcode.size(), nullptr))
		return std::unexpected("Failed to inject shellcode");

	struct Args {
		int row;
		int column;
	};

	std::println("Flagging {} mines...", _mines.size());
	for (const Field& mine : _mines)
	{
		const Args args = { mine.row, mine.col };
		if (!WriteProcessMemory(proc, remote.ptr, &args, sizeof(args), nullptr))
			return std::unexpected("Failed to write arguments");

		HANDLE thread = CreateRemoteThread(proc, nullptr, 0,
			reinterpret_cast<LPTHREAD_START_ROUTINE>(remoteCode), nullptr, 0, nullptr);
		if (!thread)
			return std::unexpected("Failed to create remote thread");

		WaitForSingleObject(thread, INFINITE);
		CloseHandle(thread);
	}

	return {};
}
