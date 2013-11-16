// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Modern Warfare 2 patches: r_noborder for render
//          window without borders (and proper height for
//          text scaling)
//
// Initial author: NTAuthority
// Started: 2011-05-21
// ==========================================================

#include "StdInc.h"

StompHook windowedWindowStyleHook;
DWORD windowedWindowStyleHookLoc = 0x507643;

dvar_t* r_noborder;

void __declspec(naked) WindowedWindowStyleHookStub()
{
	if (r_noborder->current.boolean)
	{
		__asm mov ebp, WS_VISIBLE | WS_POPUP
	}
	else
	{
		__asm mov ebp, WS_VISIBLE | WS_SYSMENU | WS_CAPTION
	}

	__asm retn
}

void PatchMW2_NoBorder()
{
	r_noborder = Dvar_RegisterBool("r_noborder", true, DVAR_FLAG_SAVED, "Do not use a border in windowed mode");

	windowedWindowStyleHook.initialize(windowedWindowStyleHookLoc, WindowedWindowStyleHookStub, 5, false);
	windowedWindowStyleHook.installHook();
}