#include <benscr/console/console.hpp>
#include <functional>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace Console = BenScr::Console;

void Check(bool value)
{
    if (!value)
    {
        throw std::runtime_error("Contract failed.");
    }
}
template <typename Exception, typename F> void Throws(F operation)
{
    try
    {
        operation();
    }
    catch (const Exception &)
    {
        return;
    }
    throw std::runtime_error("Expected exception was not thrown.");
}

class Redirect
{
  public:
    Redirect(std::ios &stream, std::streambuf *replacement)
        : stream(stream), original(stream.rdbuf(replacement))
    {
    }
    ~Redirect()
    {
        stream.rdbuf(original);
    }

  private:
    std::ios &stream;
    std::streambuf *original;
};

int main()
{
    static_assert(!std::is_copy_constructible_v<Console::TerminalSession>);
    static_assert(!std::is_move_constructible_v<Console::TerminalSession>);
    const std::vector<std::pair<const char *, std::function<void()>>> cases = {
        {"mixed write",
         []
         {
             std::ostringstream output;
             Console::TerminalWriter writer(output);
             writer.Write("value=", 42, ' ', 1.5);
             Check(output.str() == "value=42 1.5");
         }},
        {"empty writes",
         []
         {
             std::ostringstream output;
             Console::TerminalWriter writer(output);
             writer.Write();
             writer.WriteLine();
             Check(output.str() == "\n");
         }},
        {"stream formatting",
         []
         {
             std::ostringstream output;
             output << std::hex;
             Console::TerminalWriter writer(output);
             writer.Write(255);
             Check(output.str() == "ff");
         }},
        {"binary text preserved",
         []
         {
             std::ostringstream output;
             Console::TerminalWriter writer(output);
             writer.Write(std::string("a\0b", 3));
             Check(output.str() == std::string("a\0b", 3));
         }},
        {"plain controls suppressed",
         []
         {
             std::ostringstream output;
             Console::TerminalWriter writer(output);
             Check(!writer.IsControlEnabled());
             writer.Clear();
             writer.SetTitle("title");
             writer.SetCursorPosition(0, 0);
             writer.SetForeground(Console::Color::Red);
             writer.SetCursorVisible(false);
             writer.ResetStyle();
             writer.Flush();
             Check(output.str().empty());
         }},
        {"clear screen",
         []
         {
             std::ostringstream output;
             Console::TerminalWriter writer(output, true);
             Check(writer.IsControlEnabled());
             writer.Clear();
             Check(output.str() == "\x1b[2J\x1b[H");
         }},
        {"cursor origin",
         []
         {
             std::ostringstream output;
             Console::TerminalWriter writer(output, true);
             writer.SetCursorPosition(0, 0);
             Check(output.str() == "\x1b[1;1H");
         }},
        {"cursor ordering and decimal independent of stream",
         []
         {
             std::ostringstream output;
             output << std::hex;
             Console::TerminalWriter writer(output, true);
             writer.SetCursorPosition(14, 31);
             Check(output.str() == "\x1b[32;15H");
         }},
        {"cursor inclusive maximum",
         []
         {
             std::ostringstream output;
             Console::TerminalWriter writer(output, true);
             writer.SetCursorPosition(32766, 32766);
             Check(output.str() == "\x1b[32767;32767H");
         }},
        {"invalid cursor bounds",
         []
         {
             std::ostringstream output;
             Console::TerminalWriter writer(output, true);
             for (int bad :
                  {-1, 32767, std::numeric_limits<int>::max(), std::numeric_limits<int>::min()})
             {
                 Throws<std::out_of_range>([&] { writer.SetCursorPosition(bad, 0); });
                 Throws<std::out_of_range>([&] { writer.SetCursorPosition(0, bad); });
             }
             Check(output.str().empty());
         }},
        {"title encoding",
         []
         {
             std::ostringstream output;
             Console::TerminalWriter writer(output, true);
             writer.SetTitle("status: 50%");
             Check(output.str() == "\x1b]0;status: 50%\x07");
         }},
        {"empty title",
         []
         {
             std::ostringstream output;
             Console::TerminalWriter writer(output, true);
             writer.SetTitle("");
             Check(output.str() == "\x1b]0;\x07");
         }},
        {"title rejects control injection",
         []
         {
             std::ostringstream output;
             Console::TerminalWriter writer(output, true);
             for (int byte = 0; byte < 256; ++byte)
             {
                 if (byte >= 32 && byte <= 126)
                 {
                     continue;
                 }
                 Throws<std::invalid_argument>(
                     [&] { writer.SetTitle(std::string(1, static_cast<char>(byte))); });
             }
             Check(output.str().empty());
         }},
        {"title length bound",
         []
         {
             std::ostringstream output;
             Console::TerminalWriter writer(output, true);
             writer.SetTitle(std::string(1024, 'x'));
             Check(output.str().size() == 1029);
             const auto before = output.str();
             Throws<std::length_error>([&] { writer.SetTitle(std::string(1025, 'x')); });
             Check(before == output.str());
         }},
        {"all colors",
         []
         {
             for (int value : {30, 31, 32, 33, 34, 35, 36, 37, 39, 90, 91, 92, 93, 94, 95, 96, 97})
             {
                 std::ostringstream output;
                 Console::TerminalWriter writer(output, true);
                 writer.SetForeground(static_cast<Console::Color>(value));
                 Check(output.str() == "\x1b[" + std::to_string(value) + "m");
             }
         }},
        {"invalid color",
         []
         {
             std::ostringstream output;
             Console::TerminalWriter writer(output, true);
             for (int value : {-1, 0, 29, 38, 40, 89, 98, 1000})
             {
                 Throws<std::invalid_argument>(
                     [&] { writer.SetForeground(static_cast<Console::Color>(value)); });
             }
             Check(output.str().empty());
         }},
        {"validation with disabled controls",
         []
         {
             std::ostringstream output;
             Console::TerminalWriter writer(output);
             Throws<std::invalid_argument>([&] { writer.SetTitle("bad\x1b"); });
             Throws<std::out_of_range>([&] { writer.SetCursorPosition(-1, 0); });
             Throws<std::invalid_argument>(
                 [&] { writer.SetForeground(static_cast<Console::Color>(1)); });
         }},
        {"reset and cursor visibility",
         []
         {
             std::ostringstream output;
             Console::TerminalWriter writer(output, true);
             writer.SetCursorVisible(false);
             writer.SetCursorVisible(true);
             writer.ResetStyle();
             Check(output.str() == "\x1b[?25l\x1b[?25h\x1b[0m");
         }},
        {"stream errors propagate",
         []
         {
             std::ostringstream output;
             output.setstate(std::ios::badbit);
             Throws<std::ios_base::failure>([&] { output.exceptions(std::ios::badbit); });
             Console::TerminalWriter writer(output, true);
             Throws<std::ios_base::failure>([&] { writer.Clear(); });
         }},
        {"line reading including last line",
         []
         {
             std::istringstream input("first\n\nlast");
             std::string line;
             Check(Console::TryReadLine(input, line) && line == "first");
             Check(Console::TryReadLine(input, line) && line.empty());
             Check(Console::TryReadLine(input, line) && line == "last");
             Check(!Console::TryReadLine(input, line));
         }},
        {"empty input",
         []
         {
             std::istringstream input;
             std::string line = "previous";
             Check(!Console::TryReadLine(input, line));
             Check(line.empty());
         }},
        {"readline stream errors",
         []
         {
             std::istringstream input;
             input.exceptions(std::ios::failbit);
             std::string line;
             Throws<std::ios_base::failure>([&] { (void)Console::TryReadLine(input, line); });
         }},
        {"standard write wrappers",
         []
         {
             std::ostringstream output;
             Redirect redirect(std::cout, output.rdbuf());
             Console::Write("abc", 42);
             Console::WriteLine("!");
             Check(output.str() == "abc42!\n");
         }},
        {"legacy readline",
         []
         {
             std::istringstream input("first\nlast");
             Redirect redirect(std::cin, input.rdbuf());
             Check(Console::ReadLine() == "first");
             Check(Console::ReadLine() == "last");
             Check(Console::ReadLine().empty());
             std::cin.clear();
         }},
        {"session smoke", []
         {
             Console::TerminalSession session;
             (void)session.IsInteractive();
         }}};
    std::size_t passed = 0;
    for (const auto &[name, operation] : cases)
    {
        try
        {
            operation();
            ++passed;
            std::cout << "PASS " << name << '\n';
        }
        catch (const std::exception &error)
        {
            std::cerr << "FAIL " << name << ": " << error.what() << '\n';
            return 1;
        }
    }
    std::cout << passed << " contract cases passed.\n";
}
