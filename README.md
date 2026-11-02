# BenScr.Console

C++20 terminal output, input and validated ANSI controls, with no third-party runtime dependencies.

Created by Ben Schneider — https://github.com/Ben-Scr

```cpp
#include <benscr/console/console.hpp>
namespace Console = BenScr::Console;

Console::TerminalSession session;
Console::TerminalWriter terminal(std::cout, session.IsInteractive());
terminal.SetForeground(Console::Color::Green);
terminal.WriteLine("Answer: ", 42);
terminal.ResetStyle();
terminal.Flush();
```

## Installation

In a Visual Studio C++ project using packages.config:

```powershell
Install-Package BenScr.Console -Version 1.0.0
```

Use C++20. The package supplies source and the include directory through
build/native/BenScr.Console.props and BenScr.Console.targets. It compiles console.cpp with your
project, so your architecture, Debug/Release configuration and CRT selection remain consistent.
No prebuilt .lib or portable-PDB symbol archive is used. Do not also compile console.cpp manually.
Native C++ projects use packages.config; this package is not a managed .NET assembly.

CMake users build and install the source checkout:

```sh
cmake -S . -B out/build -DCMAKE_BUILD_TYPE=Release
cmake --build out/build --config Release
cmake --install out/build --config Release --prefix /your/prefix
```

Then consume the relocated installation:

```cmake
find_package(BenScrConsole 1 CONFIG REQUIRED)
target_link_libraries(MyApp PRIVATE BenScr::Console)
```

## Behavior

- TerminalWriter borrows its stream. Controls are disabled by default, making injected log streams plain.
  Write/WriteLine preserve text and the stream's formatting; newline does not imply a flush.
- Clear clears the visible screen and homes the cursor, preserving scrollback. It never launches a shell.
- Cursor positions are zero-based and limited to 0..32766 inclusive. Coordinates use decimal ANSI encoding
  even when the output stream is configured for hexadecimal formatting.
- Titles contain 0..1024 printable ASCII bytes. Control characters, escape sequences and non-ASCII bytes
  are rejected before writing. Ordinary text output accepts arbitrary stream data.
- Color supports the standard eight colors, eight bright colors and the terminal's default foreground.
  ResetStyle resets all styling; it does not restore an earlier custom style.
- Standard control functions detect redirected stdout and emit nothing there. TerminalSession enables
  Windows virtual-terminal processing and restores its previous console mode on destruction. On POSIX it
  requires a terminal and TERM other than dumb. A missing TERM disables controls.
- TerminalSession does not restore cursor visibility, title or colors. Explicitly reset what your program
  changes. Destruction performs best-effort OS mode restoration and does not throw.
- TryReadLine distinguishes an empty line from EOF; a final unterminated line succeeds. Stream exception
  settings are respected. ReadLine retains the legacy empty-string-at-EOF behavior.
- ReadKey returns a single byte, not a Unicode character/key event. Interactive Windows input uses _getch;
  POSIX temporarily disables ICANON/ECHO and restores termios afterward. Redirected input uses std::cin.
  Do not mix buffered std::cin reads with interactive POSIX ReadKey. EOF and I/O errors throw.
- Shared streams, terminal mode changes and standard input require external synchronization. Sessions must
  be nested lexically, not destroyed out of order. The library starts no threads or processes.

## Build and checks

Requires CMake 3.28+, C++20, PowerShell 7 and .NET SDK 10.0.303 for packaging. Windows requires MSVC v143
and a Windows SDK. NuGet verification downloads the pinned, checksum-checked NuGet.CommandLine 7.9.0.
The C++ library has no NuGet or .NET runtime dependency.

```powershell
pwsh -File tools/Verify.ps1 -Generator 'Visual Studio 17 2022' -Toolset v143
# After publication, verify the public feed independently:
pwsh -File tools/VerifyNuGetConsumer.ps1 -Published
```

The verifier runs 25 contract cases, standalone-header and redirected-standard-stream tests, executes
an example, tests a relocated CMake installation, audits exact package contents and builds/runs an
isolated packages.config/vcxproj consumer in Release and Debug on Windows x64. CI also exercises Linux
x64 with GCC and Clang, including ASan/UBSan. Interactive human-terminal behavior is not automated;
terminal-specific rendering and keyboard encodings depend on the host. ARM and other OSes are untested.
Tests/examples default off when included as a CMake subproject.

See [migration](https://github.com/Ben-Scr/BenScr.Console/blob/master/MIGRATION.md),
[release procedure](https://github.com/Ben-Scr/BenScr.Console/blob/master/RELEASE.md) and
[provenance](https://github.com/Ben-Scr/BenScr.Console/blob/master/docs/PROVENANCE.md).
