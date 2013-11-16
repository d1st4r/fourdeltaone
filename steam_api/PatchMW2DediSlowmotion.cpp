// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: An attempt at fixing setSlowMotion for dedicated
//          servers.
//
// Initial author: NTAuthority
// Started: 2012-12-17
// ==========================================================

#include "StdInc.h"
#include "Script.h"

/*
CallHook smSetConfigStringHook1;
DWORD smSetConfigStringHook1Loc = 0x48EC48;

CallHook smSetConfigStringHook2;
DWORD smSetConfigStringHook2Loc = 0x4D19EC;

CallHook smSetConfigStringHook3;
DWORD smSetConfigStringHook3Loc = 0x5F5FE6;

DWORD setSlowMotionRet = 0x59274D;

void __declspec(naked) SMSetConfigStringHookStub()
{
	__asm
	{
		mov eax, [esp + 8h]
		push eax
		push 1029h
		call smSetConfigStringHook1.pOriginal
		add esp, 8h

		mov eax, [esp + 8h]
		sub esp, 1Ch

		push 0 // actually another 1029h, but it doesn't matter here
		
		// jump to the 'set slow motion time values' function
		jmp setSlowMotionRet
	}
}
*/

int slowMotionDelay = 0;

CallHook applySlowMotionHook;
DWORD applySlowMotionHookLoc = 0x60B38A;

void ApplySlowMotionHookStub(int timePassed)
{
	if (slowMotionDelay <= 0)
	{
		__asm
		{
			push timePassed
			call applySlowMotionHook.pOriginal
			add esp, 4h
		}
	}
	else
	{
		slowMotionDelay -= timePassed;
	}
}

StompHook setSlowMotionHook;
DWORD setSlowMotionHookLoc = 0x5F5FF2;

void SetSlowMotionHookFunc()
{
	int duration = 1000;
	float start = Scr_GetFloat(0);
	float end = 1.0f;

	if (Scr_GetNumParam() >= 2)
	{
		end = Scr_GetFloat(1);
	}

	if (Scr_GetNumParam() >= 3)
	{
		duration = Scr_GetFloat(2) * 1000.0;
	}

	int delay = 0;

	if (start > end)
	{
		if (duration < 150)
		{
			delay = duration;
		}
		else
		{
			delay = 150;
		}
	}

	duration = duration - delay;

	// slow motion time setting function
	typedef void (__cdecl * CL_SetSlowMotion_t)(float, float, int);
	CL_SetSlowMotion_t CL_SetSlowMotion = (CL_SetSlowMotion_t)0x446E20;

	CL_SetSlowMotion(start, end, duration);
	slowMotionDelay = delay;

	// set snapshot num to 1 behind (T6 does this, why shouldn't we?)
	for (int i = 0; i < *svs_numclients; i++)
	{
		svs_clients[i].snapNum = (*(DWORD*)0x31D9384) - 1;
	}
}

void PatchMW2_DediSlowmotion()
{
/*
	smSetConfigStringHook1.initialize(smSetConfigStringHook1Loc, SMSetConfigStringHookStub);
	smSetConfigStringHook1.installHook();

	smSetConfigStringHook2.initialize(smSetConfigStringHook2Loc, SMSetConfigStringHookStub);
	smSetConfigStringHook2.installHook();

	smSetConfigStringHook3.initialize(smSetConfigStringHook3Loc, SMSetConfigStringHookStub);
	smSetConfigStringHook3.installHook();
*/

	setSlowMotionHook.initialize(setSlowMotionHookLoc, SetSlowMotionHookFunc);
	setSlowMotionHook.installHook();

	applySlowMotionHook.initialize(applySlowMotionHookLoc, ApplySlowMotionHookStub);
	applySlowMotionHook.installHook();
}