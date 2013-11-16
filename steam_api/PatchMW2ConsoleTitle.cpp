// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: console title changing for dedicated servers
//
// Initial author: Pigophone
// Started: 2012-02-15 (copied from dediclient)
// ==========================================================

#include "StdInc.h"

dvar_t** sv_hostname = (dvar_t**)0x2098D98;

/*
============
StripColors
============
*/

#define Q_IsColorString( p )  ( ( p ) && *( p ) == '^' && *( ( p ) + 1 ) && isdigit( *( ( p ) + 1 ) ) ) // ^[0-9]

void StripColors(const char* in, char* out, int max)
{
	max--; // \0
	int current = 0;
	while (*in != 0 && current < max)
	{
		if (!Q_IsColorString(in))
		{ // u so color
			*out = *in;
			out++;
			current++;
		}
		else
		{
			*in++; // move THE in pointer one more along to remove THE digit character
				   // not ntauthority lol
		}
		*in++;
	}
	*out = '\0';
}
#define CONSOLE_WINDOW *(HWND*)0x64A3288
void SetWindowTitle()
{
	if (!GAME_FLAG(GAME_FLAG_CONSOLE) || CONSOLE_WINDOW == NULL)
	{
		return;
	}
		
	if (!(*sv_hostname))
	{
		SetWindowText(CONSOLE_WINDOW, "IW4 Console");
		return;
	}

	char cleaned[1024];
	StripColors((*sv_hostname)->current.string, cleaned, sizeof(cleaned));
	const char* title = va("IW4 Console - %s", cleaned);
	strcpy_s(cleaned, 1024, title);
	SetWindowText(CONSOLE_WINDOW, cleaned);
}