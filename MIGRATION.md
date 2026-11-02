# Migration from ConsoleCPP 1.0.3

Install BenScr.Console 1.0.0, remove the old package and include `<benscr/console/console.hpp>`.
The previous `BenScr::Console` class becomes a namespace. Most calls keep their qualified spelling;
type aliases must become namespace aliases. The new package requires C++20.

| Previous API | Replacement | Contract |
| --- | --- | --- |
| `Console::Write(args...)` | `BenScr::Console::Write(args...)` | Same insertion order and standard output; now forwards arguments. |
| `Console::WriteLine(args...)` | `BenScr::Console::WriteLine(args...)` | Same output plus newline; no forced flush. |
| `Console::ReadLine()` | `BenScr::Console::ReadLine()` | Preserves empty-string-at-EOF behavior; `TryReadLine` additionally distinguishes EOF. |
| `Console::ReadKey()` | `BenScr::Console::ReadKey()` | Windows interactive byte semantics retained; redirected EOF now throws rather than reading a nonexistent console. POSIX implementation added. |
| `Console::Clear()` | `BenScr::Console::Clear()` | Replaces shell commands with ANSI clear/home. No controls on redirected stdout; scrollback is preserved. |
| `Console::SetTitle(string)` | `BenScr::Console::SetTitle(string_view)` | Borrows input for the call; accepts printable ASCII up to 1024 bytes. Invalid titles throw before output; redirected output is untouched. |
| `Console::SetCursorPosition(x,y)` | `BenScr::Console::SetCursorPosition(x,y)` | Same zero-based coordinates, now validated in [0,32766]; avoids overflow and malformed control sequences. |

The previous single prebuilt static library did not identify its CRT, architecture or configuration in
NuGet asset selection. The new source-based native package compiles its implementation with the consumer.
Existing Win32 consumers can build source themselves, but this release verifies NuGet consumption on
Windows x64 only. The CMake distribution additionally tests Linux x64. No .NET binding is included.

The old tracked project, Playground, metadata and binary are retained as historical migration material.
They do not participate in the new CMake build or package. The supported entry points are the root
CMakeLists.txt, package/ metadata and tools/ verification scripts.
