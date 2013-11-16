// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Modern Warfare 2 patches: client (in-renderer)
//          console support (IW4M 159)
//
// Initial author: NTAuthority
// Started: 2011-05-21
// ==========================================================

#include "StdInc.h"

typedef void (__cdecl * Field_Clear_t)(void* field);
Field_Clear_t Field_Clear = (Field_Clear_t)0x437EB0;

void Con_ToggleConsole()
{
	// possibly cls.keyCatchers?
	DWORD* menuFlags = (DWORD*)0xB2C538;
	*menuFlags ^= 1;

	// g_consoleField
	Field_Clear((void*)0xA1B6B0);

	// show console output?
	*(BYTE*)0xA15F38 = 0;
}

CallHook clKeyEventToggleConsoleHook1;
DWORD clKeyEventToggleConsoleHook1Loc =	0x4F690C;

StompHook clKeyEventToggleConsoleHook2;
DWORD clKeyEventToggleConsoleHook2Loc = 0x4F65A5;

static void PatchMW2_ClientConsole_Toggle()
{
	clKeyEventToggleConsoleHook1.initialize(clKeyEventToggleConsoleHook1Loc, Con_ToggleConsole);
	clKeyEventToggleConsoleHook1.installHook();

	clKeyEventToggleConsoleHook2.initialize(clKeyEventToggleConsoleHook2Loc, Con_ToggleConsole);
	clKeyEventToggleConsoleHook2.installHook();
}

// for rcon autocomplete
bool RemoteConsole_HasServer();
bool RemoteConsole_LoggedIn();

// base definitions for arena/gametype, to be moved out in the future
struct mapArena_t
{
	char uiName[32];
	char mapName[16];
	char pad[2768];
};

static int* numArenas = (int*)0x62E6930;
//static mapArena_t* arenas = (mapArena_t*)0x62E6934;

struct gameTypeName_t
{
	char gameType[12];
	char uiName[32];
};

static int* numGameTypes = (int*)0x62E50A0;
static gameTypeName_t* gameTypes = (gameTypeName_t*)0x62E50A4;

static const char* dynamicAutoComplete[256];

const char** Con_AutoCompleteList(char* command, int* numResults)
{
	newMapArena_t* arenas = *(newMapArena_t**)0x420717;

	*numResults = 0;
	if (!_stricmp(command, "map") || !strcmp(command, "devmap"))
	{
		if (*numArenas)
		{
			*numResults = *numArenas;

			for (int i = 0; i < *numArenas; i++)
			{
				dynamicAutoComplete[i] = arenas[i].mapName;
			}

			return dynamicAutoComplete;
		}
	}

	if (!_stricmp(command, "g_gametype"))
	{
		if (*numGameTypes)
		{
			*numResults = *numGameTypes;

			for (int i = 0; i < *numGameTypes; i++)
			{
				dynamicAutoComplete[i] = gameTypes[i].gameType;
			}

			return dynamicAutoComplete;
		}
	}

	if (!_stricmp(command, "rcon"))
	{
		int results = 0;

		dynamicAutoComplete[results] = "login";
		results++;

		if (!RemoteConsole_HasServer())
		{
			dynamicAutoComplete[results] = "host";
			results++;
		}

		if (RemoteConsole_LoggedIn())
		{
			dynamicAutoComplete[results] = "logout";
			results++;

			dynamicAutoComplete[results] = "g_gametype";
			results++;

			dynamicAutoComplete[results] = "map";
			results++;

			dynamicAutoComplete[results] = "status";
			results++;

			dynamicAutoComplete[results] = "clientkick";
			results++;

			dynamicAutoComplete[results] = "map_restart";
			results++;

			dynamicAutoComplete[results] = "fast_restart";
			results++;

			dynamicAutoComplete[results] = "map_rotate";
			results++;
		}

		*numResults = results;

		return dynamicAutoComplete;
	}

	return NULL;
}

void ConDrawInput_CustomAutoComplete()
{
	int numResults;
	const char** results;
	char* command = Cmd_Argv(0); // or a dvar, I guess

	if (Cmd_Argc() < 2) return;

	// remove any / or \ in front of the command name
	if (command[0] == '/' || command[0] == '\\') command++;
	
	results = Con_AutoCompleteList(command, &numResults);
	if (numResults)
	{
		__asm
		{
			// call the drawing function
			mov eax, numResults
			push eax
			mov eax, results
			push eax
			mov eax, 5A37C0h
			call eax
			add esp, 08h
		}
	}
}

#pragma optimize("", off)
void __declspec(naked) DrawCmdMatchAlternativeStub()
{
	__asm
	{
		call ConDrawInput_CustomAutoComplete
		mov eax, 5A3EB2h
		jmp eax
	}
}

void __declspec(naked) DrawDvarMatchAlternativeStub()
{
	__asm
	{
		cmp byte ptr [edi+0Ch], 6
		jne customStuff

		mov eax, 5A3DC8h
		jmp eax

customStuff:
		call ConDrawInput_CustomAutoComplete
		mov eax, 5A3DE7h
		jmp eax
	}
}
#pragma optimize("", on)

typedef void (__cdecl * Con_AutoCompleteFromList_t)(const char** list, int numItems, const char* current, char* out, size_t outLen);
Con_AutoCompleteFromList_t Con_AutoCompleteFromList = (Con_AutoCompleteFromList_t)0x403F80;

typedef void (__cdecl * Con_AutoCompleteAddLine_t)(size_t curLen);
Con_AutoCompleteAddLine_t Con_AutoCompleteAddLine = (Con_AutoCompleteAddLine_t)0x5A72B0;

void Con_DoAutoCompleteCmd()
{
	int numResults;
	const char** results;
	char* command = Cmd_Argv(0); // or a dvar, I guess

	// remove any / or \ in front of the command name, again
	if (command[0] == '/' || command[0] == '\\') command++;

	results = Con_AutoCompleteList(command, &numResults);

	if (numResults > 0)
	{
		char buffer[256];
		const char* currentLine = Cmd_Argv(1);

		Con_AutoCompleteFromList(results, numResults, currentLine, buffer, sizeof(buffer));

		__asm lea edi, buffer
		Con_AutoCompleteAddLine(strlen(currentLine));
	}
}

#pragma optimize("", off)
void __declspec(naked) CompleteCmdMatchAlternativeStub()
{
	__asm
	{
		call Con_DoAutoCompleteCmd
		mov eax, 5A73AFh
		jmp eax
	}
}

void __declspec(naked) CompleteDvarMatchAlternativeStub()
{
	__asm
	{
		call Con_DoAutoCompleteCmd
		mov eax, 5A743Bh
		jmp eax
	}
}
#pragma optimize("", on)

StompHook drawCmdMatchAlternativeHook;
DWORD drawCmdMatchAlternativeHookLoc = 0x5A3EB6;
StompHook drawDvarMatchAlternativeHook;
DWORD drawDvarMatchAlternativeHookLoc = 0x5A3DC2;
StompHook completeCmdMatchAlternativeHook;
DWORD completeCmdMatchAlternativeHookLoc = 0x5A73B8;
StompHook completeDvarMatchAlternativeHook;
DWORD completeDvarMatchAlternativeHookLoc = 0x5A7447;

static void PatchMW2_ClientConsole_AutoComplete()
{
	// ConDrawInput_DetailedCmdMatch
	*(BYTE*)0x5A3E9F = 22; // jump target if no files found

	drawCmdMatchAlternativeHook.initialize(drawCmdMatchAlternativeHookLoc, DrawCmdMatchAlternativeStub);
	drawCmdMatchAlternativeHook.installHook();

	// ConDrawInput_DetailedDvarMatch
	drawDvarMatchAlternativeHook.initialize(drawDvarMatchAlternativeHookLoc, DrawDvarMatchAlternativeStub, 6);
	drawDvarMatchAlternativeHook.installHook();

	// cmd autocompletion
	*(BYTE*)0x5A7367 = 80;

	completeCmdMatchAlternativeHook.initialize(completeCmdMatchAlternativeHookLoc, CompleteCmdMatchAlternativeStub);
	completeCmdMatchAlternativeHook.installHook();

	// dvar autocompletion
	*(BYTE*)0x5A73DD = 105;

	completeDvarMatchAlternativeHook.initialize(completeDvarMatchAlternativeHookLoc, CompleteDvarMatchAlternativeStub);
	completeDvarMatchAlternativeHook.installHook();
}

void PatchMW2_ClientConsole()
{
	PatchMW2_ClientConsole_Toggle();
	PatchMW2_ClientConsole_AutoComplete();
}

void ClientConsole_SetAutoComplete()
{
	Cmd_SetAutoComplete("exec", "", "cfg");
}