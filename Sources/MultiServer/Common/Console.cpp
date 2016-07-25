#include "stdafx.h"
#include "console.h"

/*
 * namespace OnlineGameLib::Win32::Console
 */

namespace OnlineGameLib {
	namespace Win32 {
		namespace Console {

		void clrscr()
		{
			COORD coordScreen = { 0, 0 }; 
			DWORD cCharsWritten;
			CONSOLE_SCREEN_BUFFER_INFO csbi;
			DWORD dwConSize;

			HANDLE hConsole = ::GetStdHandle( STD_OUTPUT_HANDLE ); 
			
			::GetConsoleScreenBufferInfo( hConsole, &csbi );
			dwConSize = csbi.dwSize.X * csbi.dwSize.Y; 

			::FillConsoleOutputCharacter( hConsole, TEXT(' '), dwConSize, coordScreen, &cCharsWritten ); 
			::GetConsoleScreenBufferInfo( hConsole, &csbi ); 
			::FillConsoleOutputAttribute( hConsole, csbi.wAttributes, dwConSize, coordScreen, &cCharsWritten ); 
			::SetConsoleCursorPosition( hConsole, coordScreen );
		}

		void gotoxy( int x, int y )
		{
			COORD point;
			
			point.X = x;
			point.Y = y;
			
			::SetConsoleCursorPosition( GetStdHandle( STD_OUTPUT_HANDLE ), point );
		}

		void getxy( int &x, int &y )
		{
			CONSOLE_SCREEN_BUFFER_INFO csbi;

			::GetConsoleScreenBufferInfo( ::GetStdHandle( STD_OUTPUT_HANDLE ), &csbi );

			x = csbi.dwCursorPosition.X;
			y = csbi.dwCursorPosition.Y;
		}

		void setcolor( int color )
		{
			switch ( color )
			{
			case enumWhiteonBlack:	// White on Black
				SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), FOREGROUND_INTENSITY |
					FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE );
				break;

			case enumRedonBlack:	// Red on Black
				SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), FOREGROUND_INTENSITY |
					FOREGROUND_RED );
				break;

			case enumGreenonBlack:	// Green on Black
				SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), FOREGROUND_INTENSITY |
					FOREGROUND_GREEN );
				break;

			case enumYellowonBlack:	// Yellow on Black
				SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), FOREGROUND_INTENSITY |
					FOREGROUND_RED | FOREGROUND_GREEN );
				break;

			case enumBlueonBlack:	// Blue on Black
				SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), FOREGROUND_INTENSITY |
					FOREGROUND_BLUE );
				break;

			case enumMagentaonBlack:	// Magenta on Black
				SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), FOREGROUND_INTENSITY |
					FOREGROUND_RED | FOREGROUND_BLUE );
				break;

			case enumCyanonBlack:	// Cyan on Black
				SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), FOREGROUND_INTENSITY |
					FOREGROUND_GREEN | FOREGROUND_BLUE );
				break;

			case enumBlackonGray:	// Black on Gray
				SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), BACKGROUND_INTENSITY |
					BACKGROUND_INTENSITY );
				break;

			case enumBlackonWhite:	// Black on White
				SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), BACKGROUND_INTENSITY |
					FOREGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE );
				break;

			case enumRedonWhite:	// Red on White
				SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), BACKGROUND_INTENSITY |
					FOREGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE |
					FOREGROUND_RED );
				break;

			case enumGreenonWhite: // Green on White
				SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), BACKGROUND_INTENSITY |
					FOREGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE |
					FOREGROUND_GREEN );
				break;

			case enumYellowonWhite: // Yellow on White
				SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), BACKGROUND_INTENSITY |
					FOREGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE |
					FOREGROUND_RED | FOREGROUND_GREEN );
				break;

			case enumBlueonWhite: // Blue on White
				SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), BACKGROUND_INTENSITY |
					FOREGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE |
					FOREGROUND_BLUE );
				break;

			case enumMagentaonWhite: // Magenta on White
				SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), BACKGROUND_INTENSITY |
					FOREGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE |
					FOREGROUND_RED | FOREGROUND_BLUE );
				break;

			case enumCyanonWhite: // Cyan on White
				SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), BACKGROUND_INTENSITY |
					FOREGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE |
					FOREGROUND_GREEN | FOREGROUND_BLUE );
				break;

			case enumWhiteonWhite: // White on White
				SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), BACKGROUND_INTENSITY |
					FOREGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE |
					FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE );
				break;
				
			case enumDefault:
			default : // White on Black
				SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), FOREGROUND_INTENSITY |
					FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE );
				break;
			}
		}

		} // End of namespace Console
	} // End of namespace Win32
} // End of namespace OnlineGameLib