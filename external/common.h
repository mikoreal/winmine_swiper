#pragma once
#include <cstdint>

using byte_t = std::uint8_t;

namespace winmine {
    static constexpr wchar_t PE_NAME[] = L"winmine.exe";
    static constexpr uintptr_t IMAGE_BASE = 0x1000000;

    // stride - fixed space between rows in minefield array memory 
    static constexpr uintptr_t ROW_STRIDE = 0x20; // stride size is always 32B (0x20 in hex) independent of the game difficulty [ref. 3.3]

    namespace offset {
        static constexpr uintptr_t BOARD_WIDTH  = 0x5334; // board width (number of columns)  [ref. 3.2]
        static constexpr uintptr_t BOARD_HEIGHT = 0x5338; // board height (number of rows)    [ref. 3.2]
        static constexpr uintptr_t MINEFIELD    = 0x5340; // minefield byte array(cell[0][0]) [ref. 3.3]
        
        static constexpr uintptr_t FN_ON_RIGHT_CLICK_FIELD = 0x374F; // onRightClickField(int row, int column) @.text:0x0100374F [ref. 3.4]
    }

    // formula to access value of a cell [ref. 3.3]
    //  out: returns the cell offset relative to the module base
    //  
    //  note: indices start at 1 not 0 !!!
    static constexpr uintptr_t cellOffset(int row, int col) {
        return offset::MINEFIELD + ROW_STRIDE * row + col;
    }

    // mapped cell byte meanings [ref. 3.3]
    namespace cell {
        // high 3 bits is physical property of cell
        static constexpr byte_t MINE_MASK     = 0x80; // mine is hidden under the cell
        static constexpr byte_t REVEALED_MASK = 0x40; // cell uncovered by the player
        
        static constexpr byte_t STATE_MASK = 0x1F;
        
        // low 5 bits is visual state
        static constexpr byte_t EMPTY      = 0x00; // empty, no adjacent mines
        static constexpr byte_t MINE_SHOWN = 0x0A; // mine revealed after loss
        static constexpr byte_t WRONG_FLAG = 0x0B; // incorrectly flagged cell
        static constexpr byte_t MINE_HIT   = 0x0C; // LOSS: detonated mine
        static constexpr byte_t QUESTION   = 0x0D; // cell marked with a question mark
        static constexpr byte_t FLAG       = 0x0E; // cell marked with a flag
        static constexpr byte_t UNREVEALED = 0x0F; // DEFAULT: unrevealed cell
        static constexpr byte_t BORDER     = 0x10; // invisible board margin
    }
}
