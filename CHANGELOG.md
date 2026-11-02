# Changelog

## 1.0.0

- Migrate the seven ConsoleCPP entry points to the BenScr.Console package and C++ namespace.
- Add injectable output streams, standard/bright foreground colors and cursor visibility controls.
- Validate coordinates, title bytes and colors before output; suppress controls on redirected stdout.
- Replace shell-based clearing with ANSI controls and restore temporary console modes with RAII.
- Add POSIX single-byte input, CMake installation/exports and source-based native NuGet integration.
- Add contract, standalone-header, redirected-input/output and isolated-package consumer checks.
