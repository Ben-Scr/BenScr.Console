#include <benscr/console/console.hpp>
#include <sstream>
#include <stdexcept>

int main()
{
    namespace Console = BenScr::Console;
    std::ostringstream log;
    Console::TerminalWriter writer(log);
    writer.SetForeground(Console::Color::Green);
    writer.WriteLine("Result: ", 42);
    writer.ResetStyle();
    if (log.str() != "Result: 42\n")
    {
        throw std::runtime_error("Plain log contract failed.");
    }
    Console::TerminalSession session;
    Console::TerminalWriter terminal(std::cout, session.IsInteractive());
    terminal.SetForeground(Console::Color::Green);
    terminal.Write(log.str());
    terminal.ResetStyle();
    terminal.Flush();
}
