// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: prevents cg_fov overriding
//
// Initial author: NTAuthority
// Started: 2012-05-18
// ==========================================================

#include "StdInc.h"

CallHook setClientDvarHook;
DWORD setClientDvarHookLoc = 0x59386A;

static const char* sCG_fov = "cg_fov";
static const char* s65 = "65";

dvar_t** cg_fov = (dvar_t**)0x9FBE24;

static void CG_SetFOVToNon65(const char* a, const char* value)
{
	dvar_t* fov = *cg_fov;
	fov->current.value = (float)atof(value);
}

void CG_ResetFOV()
{
	dvar_t* fov = *cg_fov;
	fov->current.value = fov->latched.value;
}

#pragma optimize("", off)
void __declspec(naked) SetClientDvarHook()
{
	__asm
	{
		push [esp + 4h]
		push sCG_fov
		call _stricmp
		add esp, 8h

		test eax, eax
		jz doReturn
		jmp setClientDvarHook.pOriginal

doReturn:
		push [esp + 8h]
		push s65
		call _stricmp
		add esp, 8h

		test eax, eax
		jz doReturn2
		jmp CG_SetFOVToNon65

doReturn2:
		retn
	}
}
#pragma optimize("", on)

void PatchMW2_ClientDvar()
{
	setClientDvarHook.initialize(setClientDvarHookLoc, SetClientDvarHook);
	setClientDvarHook.installHook();
}