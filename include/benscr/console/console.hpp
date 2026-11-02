#ifndef BENSCR_CONSOLE_CONSOLE_HPP
#define BENSCR_CONSOLE_CONSOLE_HPP

#include <iostream>
#include <ostream>
#include <string>
#include <string_view>
#include <utility>

namespace BenScr::Console
{
/// Standard ANSI foreground colors. Default restores the terminal foreground.
enum class Color
{
    Black = 30,
    Red,
    Green,
    Yellow,
    Blue,
    Magenta,
    Cyan,
    White,
    Default = 39,
    BrightBlack = 90,
    BrightRed,
    BrightGreen,
    BrightYellow,
    BrightBlue,
    BrightMagenta,
    BrightCyan,
    BrightWhite
};

/// Borrowed output stream; controls are opt-in. The stream must outlive this object.
/// Calls on the same stream require external synchronization.
class TerminalWriter
{
  public:
    explicit TerminalWriter(std::ostream &output, bool enableControl = false) noexcept;
    [[nodiscard]] bool IsControlEnabled() const noexcept;
    template <typename... Args> void Write(Args &&...args)
    {
        (output << ... << std::forward<Args>(args));
    }
    template <typename... Args> void WriteLine(Args &&...args)
    {
        Write(std::forward<Args>(args)...);
        output.put('\n');
    }
    /// Clears the visible screen and homes the cursor, preserving scrollback.
    void Clear();
    /// Zero-based coordinates in [0,32766]; invalid values throw out_of_range.
    void SetCursorPosition(int x, int y);
    /// Printable ASCII, at most 1024 bytes. Controls and escape injection are rejected.
    void SetTitle(std::string_view title);
    /// Invalid enum values throw invalid_argument even when controls are disabled.
    void SetForeground(Color color);
    /// Resets all ANSI styles; does not restore an earlier non-default style.
    void ResetStyle();
    void SetCursorVisible(bool visible);
    void Flush();

  private:
    void Control(std::string_view sequence);
    std::ostream &output;
    bool enableControl;
};

/// Enables Windows stdout virtual-terminal processing temporarily, restoring its mode on exit.
/// On POSIX detects a terminal whose TERM is set and is not dumb. Redirected output is disabled.
/// Noncopyable/nonmovable; synchronize overlapping sessions and external console mode changes.
class TerminalSession
{
  public:
    TerminalSession() noexcept;
    ~TerminalSession() noexcept;
    TerminalSession(const TerminalSession &) = delete;
    TerminalSession &operator=(const TerminalSession &) = delete;
    TerminalSession(TerminalSession &&) = delete;
    TerminalSession &operator=(TerminalSession &&) = delete;
    [[nodiscard]] bool IsInteractive() const noexcept;

  private:
#ifdef _WIN32
    void *handle = nullptr;
    unsigned long originalMode = 0;
    bool restoreMode = false;
#endif
    bool interactive = false;
};

template <typename... Args> void Write(Args &&...args)
{
    (std::cout << ... << std::forward<Args>(args));
}
template <typename... Args> void WriteLine(Args &&...args)
{
    Write(std::forward<Args>(args)...);
    std::cout.put('\n');
}
/// Reads a line without its newline, or empty at EOF (legacy behavior).
[[nodiscard]] std::string ReadLine();
/// Distinguishes EOF/failure from an empty line; a final unterminated line succeeds.
[[nodiscard]] bool TryReadLine(std::istream &input, std::string &line);
/// Reads one byte. Interactive input temporarily disables echo/canonical mode.
/// Redirected input uses std::cin. EOF/errors throw ios_base::failure/system_error.
/// UTF-8 characters and Windows extended keys may require several calls.
[[nodiscard]] char ReadKey();
void Clear();
void SetTitle(std::string_view title);
void SetCursorPosition(int x, int y);
void SetForeground(Color color);
void ResetStyle();
void SetCursorVisible(bool visible);
} // namespace BenScr::Console
#endif
