// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: load out-of-game menu data from in-game.
//
// Initial author: NTAuthority
// Started: 2011-07-16 (copied from M1 code)
// ==========================================================

#include "StdInc.h"

#define IW4M_OLD_CODE

CallHook ingameMenuHook;
DWORD ingameMenuHookLoc = 0x41C178;

dvar_t*& mapname = *(dvar_t**)0x2098DDC;

void IngameMenuHookFunc(void* menuData, void* menuFile, int num)
{
	DWORD ui_addMenu = 0x4533C0;
	DWORD loadMenuFile = 0x641460;

	const char* filename = "ui_mp/menus.txt";

	__asm
	{
		push num
		push menuFile
		push menuData
		call ui_addMenu
		add esp, 0Ch
	}

	// NUI change: only load main menus if not ui_viewer_mp (which runs within the main menu context)
	if (stricmp(mapname->current.string, "ui_viewer_mp"))
	{
		__asm
		{
			push filename
			call loadMenuFile
			add esp, 4h

			push 1
			push eax
			push menuData
			call ui_addMenu
			add esp, 0Ch
		}
	}
}

void __declspec(naked) IngameMenuHookStub()
{
	__asm
	{
		jmp IngameMenuHookFunc
	}
}

void PatchMW2_InGameMenu()
{
	ingameMenuHook.initialize(ingameMenuHookLoc, IngameMenuHookStub);
	ingameMenuHook.installHook();
}