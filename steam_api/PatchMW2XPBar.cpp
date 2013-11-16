// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: patches for the xpbar menu's onlinegame display
//
// Initial author: NTAuthority
// Started: 2011-07-06 (copied to IW4M 2012-05-18)
// ==========================================================

#include "StdInc.h"

CallHook uiDvarIntHook;
DWORD uiDvarIntHookLoc = 0x62A2A7;

static const char* sOnlinegame = "onlinegame";

dvar_t** xblive_privatematch = (dvar_t**)0x649E714;

bool Live_IsPrivateMatch()
{
	return (*xblive_privatematch)->current.boolean;
}

void __declspec(naked) UIDvarIntHookStub()
{
	__asm
	{
		push sOnlinegame
		push [esp + 8h]
		call _stricmp
		add esp, 8h
		test eax, eax
		jz returnNone
		jmp uiDvarIntHook.pOriginal

returnNone:
		call Live_IsPrivateMatch
		test al, al

		jnz returnNoneActually
		jmp uiDvarIntHook.pOriginal

returnNoneActually:
		mov eax, 649E660h
		retn
	}
}

void PatchMW2_XPBar()
{
	uiDvarIntHook.initialize(uiDvarIntHookLoc, UIDvarIntHookStub);
	uiDvarIntHook.installHook();
}