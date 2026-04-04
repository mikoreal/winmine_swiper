#pragma once

#include <iostream>
#include <vector>
#include <cstddef>
#include <Windows.h>
#include "proc.h"

using std::vector;
using std::byte;

class MinesweeperGame {

private:
    static constexpr wchar_t PROC_NAME[] = L"winmine.exe";
    static constexpr uintptr_t CHANGE_FIELD_STATE_FUNCTION_OFFSET = 0x374F;
    //onRightClickField(int row, int column)    .text:0100374F    Length: 0x92

    //0x2EAB; //changeFieldState(int row, int column, char state)	.text:0x01002EAB	Length: 0x2A

    struct HeightWidth {
        int height;
        int width;
    };

    struct GameConfig {
        HeightWidth heightWidth;
    } config;

    struct ProcInfo {
        DWORD procId;
        uintptr_t modBase;
        HANDLE hProcess;
    } procInfo;

    struct GameInfo {
        vector<vector<byte>> minefieldVec;
        vector<HeightWidth> mineIdxVec;
    } gameInfo;

private:
    void loadMines();
    void loadProcHandle();
    void loadConfig();

public:
    void printMines();
    void callChangeFieldState();

    MinesweeperGame();
};