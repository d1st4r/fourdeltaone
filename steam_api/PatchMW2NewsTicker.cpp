// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: support code for the main menu news ticker
//
// Initial author: NTAuthority
// Started: 2012-04-19
// ==========================================================

#include "StdInc.h"

CallHook newsTickerGetTextHook;
DWORD newsTickerGetTextHookLoc = 0x6388C1;

char* Auth_GetUsername();
char ticker[1024];
const char* NewsTicker_GetText(const char* garbage)
{
	SYSTEMTIME time;
	GetSystemTime(&time);

	if (time.wYear > 2012 || time.wMonth > 8 || time.wDay > 18)
	{
		if (!ticker[0])
		{
			strcpy(ticker, va("Nuketown DLC now available from the Store in the Main Menu!", Auth_GetUsername()));
		}		
	}
	else
	{
		SYSTEMTIME ttime;
		ttime.wYear = 2012;
		ttime.wMonth = 8;
		ttime.wDay = 18;
		ttime.wHour = 18;
		ttime.wMinute = 0;
		ttime.wSecond = 0;
		ttime.wMilliseconds = 0;

		FILETIME targetFTime;
		ULARGE_INTEGER target;
		SystemTimeToFileTime(&ttime, &targetFTime);

		target.HighPart = targetFTime.dwHighDateTime;
		target.LowPart = targetFTime.dwLowDateTime;

		FILETIME curFTime;
		ULARGE_INTEGER cur;
		SystemTimeToFileTime(&time, &curFTime);

		cur.HighPart = curFTime.dwHighDateTime;
		cur.LowPart = curFTime.dwLowDateTime;

		int seconds = ((target.QuadPart - cur.QuadPart) / 10000000);

		if (seconds < 0)
		{
			seconds = 0;
		}

		_snprintf(ticker, sizeof(ticker), "IW5M (MW3) Launch Event at http://fourdeltaone.net/ - %02d:%02d:%02d remain...", (seconds / 3600), (seconds % 3600) / 60, (seconds % 60));
	}

	return ticker;
}

void PatchMW2_NewsTicker()
{
	// hook for getting the news ticker string
	*(WORD*)0x6388BB = 0x9090; // skip the "if (item->text[0] == '@')" localize check

	// replace the localize function
	newsTickerGetTextHook.initialize(newsTickerGetTextHookLoc, NewsTicker_GetText);
	newsTickerGetTextHook.installHook();

	// make newsfeed (ticker) menu items not cut off based on safe area
	memset((void*)0x63892D, 0x90, 5);
}