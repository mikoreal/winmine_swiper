#pragma once

#include <cstddef>
#include <expected>
#include <ranges>
#include <span>
#include <string>
#include <utility>
#include <vector>
#include "common.h"
#include "process_memory.hpp"

// a rectangle grid of raw cell bytes copied from the game process memory
// stored as one flat buffer - exactly like the game implements board
class Board {
public:
	Board() = default;
	Board(int width, int height)
		: _width{ width },
		_height{ height },
		_cells(static_cast<std::size_t>(width) * height)
	{}

	int width()  const { return _width; }
	int height() const { return _height; }

	template <class Self>
	auto&& operator[](this Self&& self, int row, int col)
	{
		return std::forward<Self>(self)._cells.at(self.index(row, col));
	}

	std::span<byte_t> rowData(int row)
	{
		return { _cells.data() + index(row, 1), static_cast<std::size_t>(_width) };
	}

	auto rows() const { return _cells | std::views::chunk(_width); }

private:
	std::size_t index(int row, int col) const
	{
		return static_cast<std::size_t>(row - 1) * _width + (col - 1);
	}

	int _width  = 0;
	int _height = 0;
	std::vector<byte_t> _cells;
};

class MineSwiper {
public:
	MineSwiper();

	void printMines() const;
	void printMineMap() const;
	std::expected<void, std::string> flagMines() const;

private:
	void attach();          // open a handle to the 'winmine.exe' process
	void readDimensions();  // read board width and height        [ref. 3.2]
	void readBoard();       // read the cell array, collect mines [ref. 3.3]

	struct Field { int row; int col; };

	Process _process;
	Board   _board;
	std::vector<Field> _mines;
};
