#include "../Include/Console.hpp"
#include <conio.h>  

namespace BenScr {
	std::string Console::ReadLine() {
		std::string input;
		std::getline(std::cin, input);
		return input;
	}

	char Console::ReadKey() {
		return _getch();
	}

	void Console::Clear() {
		{
#ifdef _WIN32
			system("cls");
#else
			system("clear");
#endif
		}
	}

	void Console::SetTitle(const std::string& title)
	{
		std::cout << "\033]0;" << title << "\007";
	}

	void Console::SetCursorPosition(int x, int y)
	{
		std::cout << "\033[" << (y + 1) << ";" << (x + 1) << "H";
	}
}