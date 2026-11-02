#include <benscr/console/console.hpp>
#include <stdexcept>

int main()
{
    namespace Console = BenScr::Console;
    Console::TerminalSession session;
    if (session.IsInteractive() || Console::ReadLine() != "line" || Console::ReadKey() != 'Q')
    {
        throw std::runtime_error("Redirected input/session contract failed.");
    }
    try
    {
        (void)Console::ReadKey();
        return 2;
    }
    catch (const std::ios_base::failure &)
    {
    }
    Console::Clear();
    Console::SetTitle("Test");
    Console::SetCursorPosition(2, 3);
    Console::SetCursorVisible(false);
    Console::SetForeground(Console::Color::Red);
    Console::ResetStyle();
    Console::WriteLine("ok");
}
