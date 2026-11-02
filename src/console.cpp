#include <benscr/console/console.hpp>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <system_error>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <conio.h>
#include <io.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

namespace BenScr::Console
{
TerminalWriter::TerminalWriter(std::ostream &output, bool enableControl) noexcept
    : output(output), enableControl(enableControl)
{
}
bool TerminalWriter::IsControlEnabled() const noexcept
{
    return enableControl;
}
void TerminalWriter::Control(std::string_view sequence)
{
    if (enableControl)
    {
        output.write(sequence.data(), static_cast<std::streamsize>(sequence.size()));
    }
}
void TerminalWriter::Clear()
{
    Control("\x1b[2J\x1b[H");
}
void TerminalWriter::SetCursorPosition(int x, int y)
{
    constexpr int kMaximumCoordinate = 32766;
    if (x < 0 || y < 0 || x > kMaximumCoordinate || y > kMaximumCoordinate)
    {
        throw std::out_of_range("Cursor coordinates must be in [0,32766].");
    }
    Control("\x1b[" + std::to_string(y + 1) + ";" + std::to_string(x + 1) + "H");
}
void TerminalWriter::SetTitle(std::string_view title)
{
    constexpr std::size_t kMaximumTitleLength = 1024;
    if (title.size() > kMaximumTitleLength)
    {
        throw std::length_error("Terminal title exceeds 1024 bytes.");
    }
    for (const unsigned char character : title)
    {
        if (character < 32 || character > 126)
        {
            throw std::invalid_argument("Terminal titles must contain printable ASCII only.");
        }
    }
    Control("\x1b]0;" + std::string(title) + "\x07");
}
void TerminalWriter::SetForeground(Color color)
{
    const int value = static_cast<int>(color);
    if (!((value >= 30 && value <= 37) || value == 39 || (value >= 90 && value <= 97)))
    {
        throw std::invalid_argument("Invalid foreground color.");
    }
    Control("\x1b[" + std::to_string(value) + "m");
}
void TerminalWriter::ResetStyle()
{
    Control("\x1b[0m");
}
void TerminalWriter::SetCursorVisible(bool visible)
{
    Control(visible ? "\x1b[?25h" : "\x1b[?25l");
}
void TerminalWriter::Flush()
{
    output.flush();
}
TerminalSession::TerminalSession() noexcept
{
#ifdef _WIN32
    HANDLE outputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (outputHandle != INVALID_HANDLE_VALUE && outputHandle != nullptr &&
        GetConsoleMode(outputHandle, &mode) != 0)
    {
        constexpr DWORD kRequiredMode =
            ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        if ((mode & kRequiredMode) == kRequiredMode ||
            SetConsoleMode(outputHandle, mode | kRequiredMode) != 0)
        {
            handle = outputHandle;
            originalMode = mode;
            restoreMode = (mode & kRequiredMode) != kRequiredMode;
            interactive = true;
        }
    }
#else
    const char *term = std::getenv("TERM");
    interactive =
        ::isatty(STDOUT_FILENO) == 1 && term != nullptr && std::string_view(term) != "dumb";
#endif
}
TerminalSession::~TerminalSession() noexcept
{
#ifdef _WIN32
    if (restoreMode)
    {
        SetConsoleMode(handle, originalMode);
    }
#endif
}
bool TerminalSession::IsInteractive() const noexcept
{
    return interactive;
}
bool TryReadLine(std::istream &input, std::string &line)
{
    return static_cast<bool>(std::getline(input, line));
}
std::string ReadLine()
{
    std::string line;
    std::getline(std::cin, line);
    return line;
}
char ReadKey()
{
#ifdef _WIN32
    if (_isatty(_fileno(stdin)) != 0)
    {
        return static_cast<char>(_getch());
    }
#else
    if (::isatty(STDIN_FILENO) == 1)
    {
        termios original{};
        if (::tcgetattr(STDIN_FILENO, &original) != 0)
        {
            throw std::system_error(errno, std::generic_category(), "Cannot read terminal mode");
        }
        termios raw = original;
        raw.c_lflag &= static_cast<tcflag_t>(~(ICANON | ECHO));
        raw.c_cc[VMIN] = 1;
        raw.c_cc[VTIME] = 0;
        if (::tcsetattr(STDIN_FILENO, TCSANOW, &raw) != 0)
        {
            throw std::system_error(errno, std::generic_category(), "Cannot set terminal mode");
        }
        struct RestoreInput
        {
            termios Original;
            ~RestoreInput()
            {
                ::tcsetattr(STDIN_FILENO, TCSANOW, &Original);
            }
        } restore{original};
        char result{};
        ssize_t count;
        do
        {
            count = ::read(STDIN_FILENO, &result, 1);
        } while (count < 0 && errno == EINTR);
        if (count < 0)
        {
            throw std::system_error(errno, std::generic_category(), "Cannot read terminal byte");
        }
        if (count == 0)
        {
            throw std::ios_base::failure("End of terminal input.");
        }
        return result;
    }
#endif
    const auto value = std::cin.get();
    if (value == std::char_traits<char>::eof())
    {
        throw std::ios_base::failure("End of console input.");
    }
    return static_cast<char>(value);
}
namespace Detail
{
template <typename Operation> void WithStandardOutput(Operation operation)
{
    TerminalSession session;
    TerminalWriter writer(std::cout, session.IsInteractive());
    operation(writer);
    writer.Flush();
}
} // namespace Detail
void Clear()
{
    Detail::WithStandardOutput([](TerminalWriter &writer) { writer.Clear(); });
}
void SetTitle(std::string_view title)
{
    Detail::WithStandardOutput([title](TerminalWriter &writer) { writer.SetTitle(title); });
}
void SetCursorPosition(int x, int y)
{
    Detail::WithStandardOutput([x, y](TerminalWriter &writer) { writer.SetCursorPosition(x, y); });
}
void SetForeground(Color color)
{
    Detail::WithStandardOutput([color](TerminalWriter &writer) { writer.SetForeground(color); });
}
void ResetStyle()
{
    Detail::WithStandardOutput([](TerminalWriter &writer) { writer.ResetStyle(); });
}
void SetCursorVisible(bool visible)
{
    Detail::WithStandardOutput([visible](TerminalWriter &writer)
                               { writer.SetCursorVisible(visible); });
}
} // namespace BenScr::Console
