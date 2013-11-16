// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Log a dvar infostring during InitGame logging for
//          compatibility with legacy log parsing utilities.
//
// Initial author: NTAuthority
// Started: 2011-05-21 (copied from PatchMW2Status.cpp)
// ==========================================================

#include "StdInc.h"
#include "PatchMW2Voting.h"

CallHook logInitGameHook;
DWORD logInitGameHookLoc = 0x48ED70;

void LogInitGameHookFunc()
{
	char infostring[2048];

	strncpy(infostring, Dvar_InfoString_Big(1024), 2048);

	G_LogPrintf("InitGame: %s\n", infostring);

	Vote_OnInitGame();
}

void __declspec(naked) LogInitGameHookStub()
{
	__asm jmp LogInitGameHookFunc
}

void PatchMW2_LogInitGame()
{
	logInitGameHook.initialize(logInitGameHookLoc, LogInitGameHookStub);
	logInitGameHook.installHook();
}