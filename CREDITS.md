# Credits

None of the reverse engineering in [the writeup](hacking-minesweeper-writeup/hacking-minesweeper-writeup.md)
would have been possible without the tools below. Every one of them was used in its free or
open-source edition.

## Reverse engineering

**[IDA Free](https://hex-rays.com/ida-free)** — Hex-Rays
The Interactive Disassembler: disassembler, decompiler and debugger for Windows executables.
Free for non-commercial use. This is the tool the analysis rests on. Its cross-reference search
turned "somewhere in the binary" into two call sites; its decompiler turned `sub_1003940` into
`return rand() % a1`; its debugger and IDC scripting captured the board dimensions and the mine
indices at runtime; and its database is where every rename and comment
(`rand_0`, `width`, `height`, `mineRowIdx`, `onRightClickField`) was recorded.

**[CrySearch](https://www.crysearch.nl/)** — evolution536
Open-source memory scanner and editor for Windows.
Source: [bitbucket.org/evolution536/crysearch-memory-scanner](https://bitbucket.org/evolution536/crysearch-memory-scanner)
Used to read the minefield array live, to correlate every visual state of a cell with its byte
value, and — via its write breakpoints — to catch the instruction at `1002ECB` that led to the
flag-placement function.

**[PE-bear](https://github.com/hasherezade/pe-bear)** — hasherezade
PE format inspector, used to examine the headers of `winmine.exe`.

## Development

**[Visual Studio Community 2022](https://visualstudio.microsoft.com/)** — Microsoft
Free for non-commercial use. The tool in `external/` is built with the v143 toolset.

**C++23** — the tool targets `/std:c++latest` and uses the standard library's ranges,
`std::expected` and `std::print` facilities.

## Analysis environment

**[QEMU](https://www.qemu.org/)** and **[libvirt](https://libvirt.org/)**
Used on an [Arch Linux](https://archlinux.org/) host to build two virtual machines: a temporary
Windows XP install, used solely to extract `winmine.exe` from `C:\Windows\System32\`, and a
Windows 11 machine where all of the analysis was actually carried out. Windows' backward
compatibility means the XP-era binary runs and can be debugged on a modern system.

## Documentation and media

**[MiKTeX](https://miktex.org/)** — the LaTeX distribution used to typeset the writeup.

**[FFmpeg](https://ffmpeg.org/)** — used to encode the demo recording in the README.

## The target

`winmine.exe` and `msvcrt.dll` are Microsoft software. Minesweeper shipped with Windows from
1990 until Windows Vista; the build analysed here is the Windows XP one, extracted from a
retail installation CD from my own archive.

The binary is **not** redistributed in this repository. The disassembly and decompiler output
reproduced in the writeup appear for the purpose of documenting the analysis. The target was
chosen because it is a single-player offline game with no anti-cheat, no online component and
no leaderboard — small enough to be understood completely.

## Bibliography

As cited in the original thesis:

1. IDA Free — <https://hex-rays.com/ida-free>
2. CrySearch — <https://www.crysearch.nl/>
3. CrySearch (open source) — <https://bitbucket.org/evolution536/crysearch-memory-scanner>
