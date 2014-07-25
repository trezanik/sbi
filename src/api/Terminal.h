#pragma once

/**
 * @file	src/api/Terminal.h
 * @author	James Warren
 * @brief	Win32 console and Linux/Unix terminal colour output formatting
 * @todo	Win32 code complete, unix/linux needs doing
 */


#include <iostream>
#include <iomanip>

#if defined(_WIN32)
#	define WIN32_LEAN_AND_MEAN	// needed for no socket conflicts
#	include <Windows.h>
#	include <conio.h>
#else
#	include <unistd.h>
#	include <termios.h>
#endif

#include "definitions.h"
#include "types.h"



BEGIN_NAMESPACE(APP_NAMESPACE)


/**
 * Console/Terminal output support for streaming colours into standard ostreams.
 *
 * Enables changing colours like:
 @code
 std::cout << fg_white << "White Foreground, " << bg_cyan << "Cyan Background\n";
 @endcode
 *
 * Significantly more legible than outputting, call colour change function,
 * output again, repeat, etc.
 *
 * Callers within this application should not assume the result of a prior call
 * (i.e. any colour could be fg/bg) - you should always set the desired colours
 * on invocation.
 *
 * We use `SetColor` instead of 'real' English as more people tend to understand
 * or expect the US version of the spelling.
 *
 * Since Windows and Linux/Unix command-lines differ so much, each class
 * declaration is contained in its own if preprocessor block.
 *
 * @class Terminal
 */
static class Terminal
{

#if defined(_WIN32)

#	define BG_MASK			( BACKGROUND_BLUE|BACKGROUND_GREEN|BACKGROUND_RED|BACKGROUND_INTENSITY )
#	define FG_MASK			( FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_INTENSITY )

#	define FG_BLACK			0
#	define FG_RED			FOREGROUND_RED
#	define FG_GREEN			FOREGROUND_GREEN
#	define FG_BLUE			FOREGROUND_BLUE
#	define FG_CYAN			(FG_GREEN|FG_BLUE)
#	define FG_MAGENTA		(FG_RED|FG_BLUE)
#	define FG_YELLOW		(FG_RED|FG_GREEN)
#	define FG_WHITE			(FG_RED|FG_GREEN|FG_BLUE)
#	define FG_GREY			(FG_BLACK|FOREGROUND_INTENSITY)
#	define FG_BRIGHT_WHITE		(FG_WHITE|FOREGROUND_INTENSITY)
#	define FG_BRIGHT_RED		(FOREGROUND_RED|FOREGROUND_INTENSITY)
#	define FG_BRIGHT_GREEN		(FOREGROUND_GREEN|FOREGROUND_INTENSITY)
#	define FG_BRIGHT_BLUE		(FOREGROUND_BLUE|FOREGROUND_INTENSITY)
#	define FG_BRIGHT_CYAN		(FG_GREEN|FG_BLUE|FOREGROUND_INTENSITY)
#	define FG_BRIGHT_MAGENTA	(FG_RED|FG_BLUE|FOREGROUND_INTENSITY)
#	define FG_BRIGHT_YELLOW		(FG_RED|FG_GREEN|FOREGROUND_INTENSITY)

#	define BG_BLACK			0
#	define BG_RED			BACKGROUND_RED
#	define BG_GREEN			BACKGROUND_GREEN
#	define BG_BLUE			BACKGROUND_BLUE
#	define BG_CYAN			(BG_GREEN|BG_BLUE)
#	define BG_MAGENTA		(BG_RED|BG_BLUE)
#	define BG_YELLOW		(BG_RED|BG_GREEN)
#	define BG_WHITE			(BG_RED|BG_GREEN|BG_BLUE)
#	define BG_GREY			(BG_BLACK|BACKGROUND_INTENSITY)
#	define BG_BRIGHT_WHITE		(BG_WHITE|BACKGROUND_INTENSITY)
#	define BG_BRIGHT_RED		(BACKGROUND_RED|BACKGROUND_INTENSITY)
#	define BG_BRIGHT_GREEN		(BACKGROUND_GREEN|BACKGROUND_INTENSITY)
#	define BG_BRIGHT_BLUE		(BACKGROUND_BLUE|BACKGROUND_INTENSITY)
#	define BG_BRIGHT_CYAN		(BG_GREEN|BG_BLUE|BACKGROUND_INTENSITY)
#	define BG_BRIGHT_MAGENTA	(BG_RED|BG_BLUE|BACKGROUND_INTENSITY)
#	define BG_BRIGHT_YELLOW		(BG_RED|BG_GREEN|BACKGROUND_INTENSITY)

private:
	HANDLE		_console;
	DWORD		_written;
	uint32_t	_size;
	uint16_t	_default_attrib;
	CONSOLE_SCREEN_BUFFER_INFO	_csbi;

	void
	InternalUpdate()
	{
		GetConsoleScreenBufferInfo(_console, &_csbi);
		_size = _csbi.dwSize.X * _csbi.dwSize.Y;
	}

public:
	Terminal()
	{
		_console = GetStdHandle(STD_OUTPUT_HANDLE);
		GetConsoleScreenBufferInfo(_console, &_csbi);
		_default_attrib = _csbi.wAttributes;
	}


	void
	Clear()
	{
		COORD	coordScreen = { 0, 0 };

		InternalUpdate();
		FillConsoleOutputCharacter(_console, ' ', _size,
			coordScreen, &_written);
		InternalUpdate();
		FillConsoleOutputAttribute(_console, _csbi.wAttributes,
			_size, coordScreen, &_written);
		SetConsoleCursorPosition(_console, coordScreen);
	}


	char
	GetCharacter()
	{
		while ( !_kbhit() )
		{
			return (char)_getch();
		}
	}


	void
	SetColor(
		uint16_t rgb,
		uint16_t mask
	)
	{
		InternalUpdate();
		_csbi.wAttributes &= mask;
		_csbi.wAttributes |= rgb;
		SetConsoleTextAttribute(_console, _csbi.wAttributes);
	}


	void
	SetDefault()
	{
		_csbi.wAttributes = _default_attrib;
		SetConsoleTextAttribute(_console, _csbi.wAttributes);
	}


#else	// _WIN32


/** @todo linux implementation */

#	define BG_MASK			0
#	define FG_MASK			0

#	define FG_BLACK			0
#	define FG_RED			0
#	define FG_GREEN			0
#	define FG_BLUE			0
#	define FG_CYAN			(FG_GREEN|FG_BLUE)
#	define FG_MAGENTA		(FG_RED|FG_BLUE)
#	define FG_YELLOW		(FG_RED|FG_GREEN)
#	define FG_WHITE			(FG_RED|FG_GREEN|FG_BLUE)
#	define FG_GREY			0
#	define FG_BRIGHT_WHITE		0
#	define FG_BRIGHT_RED		0
#	define FG_BRIGHT_GREEN		0
#	define FG_BRIGHT_BLUE		0
#	define FG_BRIGHT_CYAN		0
#	define FG_BRIGHT_MAGENTA	0
#	define FG_BRIGHT_YELLOW		0

#	define BG_BLACK			0
#	define BG_RED			0
#	define BG_GREEN			0
#	define BG_BLUE			0
#	define BG_CYAN			(BG_GREEN|BG_BLUE)
#	define BG_MAGENTA		(BG_RED|BG_BLUE)
#	define BG_YELLOW		(BG_RED|BG_GREEN)
#	define BG_WHITE			(BG_RED|BG_GREEN|BG_BLUE)
#	define BG_GREY			0
#	define BG_BRIGHT_WHITE		0
#	define BG_BRIGHT_RED		0
#	define BG_BRIGHT_GREEN		0
#	define BG_BRIGHT_BLUE		0
#	define BG_BRIGHT_CYAN		0
#	define BG_BRIGHT_MAGENTA	0
#	define BG_BRIGHT_YELLOW		0


private:
	void
	InternalUpdate()
	{
	}

public:
	Terminal()
	{
	}


	void
	Clear()
	{
	}


	char
	GetCharacter()
	{
		// stackoverflow saves the day again: http://stackoverflow.com/questions/421860/c-c-capture-characters-from-standard-input-without-waiting-for-enter-to-be-pr
		char buf = 0;
		struct termios old = {0};
		if (tcgetattr(0, &old) < 0)
			perror("tcsetattr()");
		old.c_lflag &= ~ICANON;
		old.c_lflag &= ~ECHO;
		old.c_cc[VMIN] = 1;
		old.c_cc[VTIME] = 0;
		if (tcsetattr(0, TCSANOW, &old) < 0)
			perror("tcsetattr ICANON");
		if (read(0, &buf, 1) < 0)
			perror ("read()");
		old.c_lflag |= ICANON;
		old.c_lflag |= ECHO;
		if (tcsetattr(0, TCSADRAIN, &old) < 0)
			perror ("tcsetattr ~ICANON");
		return (buf);
	}


	void
	SetColor(
		uint16_t rgb,
		uint16_t mask
	)
	{
	}


	void
	SetDefault()
	{
	}


#endif	// _WIN32

} terminal;



/* only support changing both the foreground + background to their defaults at 
 * the same time; non-defaults need dedicated calls */

#define bg_default	bgfg_default
#define fg_default	bgfg_default

/* function names must match, and to save more spammage, try to retain the same
 * calls like SetColor() and their definitions */

inline std::ostream& bg_black( std::ostream& os )
{
	 os.flush();
	 terminal.SetColor(BG_BLACK, FG_MASK);
	 return os;
}

inline std::ostream& bg_blue( std::ostream& os )
{
	 os.flush();
	 terminal.SetColor(BG_BRIGHT_BLUE, FG_MASK);
	 return os;
}

inline std::ostream& bg_cyan( std::ostream& os )
{
	 os.flush();
	 terminal.SetColor(BG_BRIGHT_CYAN, FG_MASK);
	 return os;
}

inline std::ostream& bgfg_default( std::ostream& os )
{
	 os.flush();
	 terminal.SetDefault();
	 return os;
}

inline std::ostream& bg_green( std::ostream& os )
{
	 os.flush();
	 terminal.SetColor(BG_BRIGHT_GREEN, FG_MASK);
	 return os;
}

inline std::ostream& bg_grey( std::ostream& os )
{
	 os.flush();
	 terminal.SetColor(BG_GREY, FG_MASK);
	 return os;
}

inline std::ostream& bg_magenta( std::ostream& os )
{
	 os.flush();
	 terminal.SetColor(BG_BRIGHT_MAGENTA, FG_MASK);
	 return os;
}

inline std::ostream& bg_red( std::ostream& os )
{
	 os.flush();
	 terminal.SetColor(BG_BRIGHT_RED, FG_MASK);
	 return os;
}

inline std::ostream& bg_white( std::ostream& os )
{
	 os.flush();
	 terminal.SetColor(BG_BRIGHT_WHITE, FG_MASK);
	 return os;
}

inline std::ostream& bg_yellow( std::ostream& os )
{
	 os.flush();
	 terminal.SetColor(BG_BRIGHT_YELLOW, FG_MASK);
	 return os;
}



inline std::ostream& clear( std::ostream& os )
{
	 os.flush();
	 terminal.Clear();
	 return os;
};



inline std::ostream& fg_black( std::ostream& os )
{
	 os.flush();
	 terminal.SetColor(FG_BLACK, BG_MASK);
	 return os;
}

inline std::ostream& fg_blue( std::ostream& os )
{
	 os.flush();
	 terminal.SetColor(FG_BRIGHT_BLUE, BG_MASK);
	 return os;
}

inline std::ostream& fg_cyan( std::ostream& os )
{
	 os.flush();
	 terminal.SetColor(FG_BRIGHT_CYAN, BG_MASK);
	 return os;
}

inline std::ostream& fg_green( std::ostream& os )
{
	 os.flush();
	 terminal.SetColor(FG_BRIGHT_GREEN, BG_MASK);
	 return os;
}

inline std::ostream& fg_grey( std::ostream& os )
{
	 os.flush();
	 terminal.SetColor(FG_GREY, BG_MASK);
	 return os;
}

inline std::ostream& fg_magenta( std::ostream& os )
{
	 os.flush();
	 terminal.SetColor(FG_BRIGHT_MAGENTA, BG_MASK);
	 return os;
}

inline std::ostream& fg_red( std::ostream& os )
{
	 os.flush();
	 terminal.SetColor(FG_BRIGHT_RED, BG_MASK);
	 return os;
}

inline std::ostream& fg_white( std::ostream& os )
{
	 os.flush();
	 terminal.SetColor(FG_BRIGHT_WHITE, BG_MASK);
	 return os;
}

inline std::ostream& fg_yellow( std::ostream& os )
{
	 os.flush();
	 terminal.SetColor(FG_BRIGHT_YELLOW, BG_MASK);
	 return os;
}



END_NAMESPACE
