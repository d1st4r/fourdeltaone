// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Dvar modifications, from changing flags to 
//          restricting changes
//
// Initial author: Pigophone / NTAuthority (copied from 
//                 PatchMW2.cpp)
// Started: 2012-04-19
// ==========================================================

#include "StdInc.h"
float cgFov90 = 90.0f;

StompHook dvarSetVariantHook;
DWORD dvarSetVariantHookLoc = 0x647840;
DWORD dvarSetVariantHookEnd = 0x647847;
DWORD dvarSetVariantHookRetn = 0x64798F;

dvar_t* dvar;
dvar_t** sv_running = (dvar_t**)0x1AD7934; //host
int* demoPlaying = (int*)0xA5EA0C;

int DvarSetVariantHookFunc()
{
	if (dvar && !strcmp(dvar->name, "g_hardcore"))
	{
		//disallow changes if not host, ingame and is not playing demo
		if (*sv_running && !(*sv_running)->current.boolean && CL_IsCgameInitialized() && *demoPlaying != 1)
		{
			Com_Printf(0, "g_hardcore cannot be changed right now.\n");
			return 1;
		}
	}
	return 0;
}

void __declspec(naked) DvarSetVariantHookStub()
{
	__asm {
		mov dvar, edi
		call DvarSetVariantHookFunc
		test al, al
		jnz end
		movzx   eax, byte ptr [edi+12]
		add     eax, 0FFFFFFFEh
		jmp dvarSetVariantHookEnd
end:
		jmp dvarSetVariantHookRetn
	}
}

StompHook dvarSetVariantHook2;
DWORD dvarSetVariantHook2Loc = 0x647684;
DWORD dvarSetVariantHook2Continue = 0x64768A;
DWORD dvarSetVariantHook2Allow = 0x647761;

int DvarSetVariantHook2Func()
{
	if (*demoPlaying == 0) return 0;
	return 1;
}
void __declspec(naked) DvarSetVariantHook2Stub()
{
	__asm 
	{
		call DvarSetVariantHook2Func
		test al, al
		jnz good
		mov ecx, [edi+10h]
		sub esp, 10h
		jmp dvarSetVariantHook2Continue

good:
		jmp dvarSetVariantHook2Allow
	}
}

void ServerCommand_SetClientDvarHook(char* name, char* value)
{
	dvar_t* dvar = Dvar_FindVar(name);
	if(dvar)
	{
		if((dvar->flags & DVAR_FLAG_SAVED) == DVAR_FLAG_SAVED)
		{
			Com_Printf(0, "Not allowing server to set %s to %s (dvar is saved)\n", name, value);
			return;
		}
	}

	Dvar_SetStringByName(name, value);
}

void PatchMW2_Dvars()
{
	// logfile default to 2 as in IW4M1
	*(BYTE*)0x60AE4A = 2;

	// remove write protection from fs_game
	*(DWORD*)0x6431EA ^= DVAR_FLAG_WRITEPROTECTED;

	// un-cheat cg_fov and add archive flags
	*(BYTE*)0x4F8E35 ^= DVAR_FLAG_CHEAT | DVAR_FLAG_SAVED; // 65

	// un-cheat cg_fovscale and add archive flags
	*(BYTE*)0x4F8E68 ^= DVAR_FLAG_CHEAT | DVAR_FLAG_SAVED;

	// set cg_fov max to 90.0
	*(DWORD*)0x4F8E28 = (DWORD)&cgFov90;

	// set flags of upnp_maxAttempts to archive
	*(BYTE*)0x4FD420 |= DVAR_FLAG_SAVED;

	// set flags of cg_drawFPS to archive
	*(BYTE*)0x4F8F69 |= DVAR_FLAG_SAVED;

	// un-cheat cg_debugInfoCornerOffset and add archive flags
	*(BYTE*)0x4F8FC2 ^= DVAR_FLAG_CHEAT | DVAR_FLAG_SAVED;

	// remove archive flags for cg_hudchatposition
	*(BYTE*)0x4F9992 ^= DVAR_FLAG_SAVED;

	// set ui_browserMod's default to -1 (All)
	*(BYTE*)0x630FD1 = 0xFF;

	// playlist minimum -1
	*(BYTE*)0x404CD5 = 0xFF;

	if (!GAME_FLAG(GAME_FLAG_DEDICATED))
	{
		dvarSetVariantHook.initialize(dvarSetVariantHookLoc, DvarSetVariantHookStub);
		dvarSetVariantHook.installHook(); 

		// allow cheat dvars to be modified in demos.
		dvarSetVariantHook2.initialize(dvarSetVariantHook2Loc, DvarSetVariantHook2Stub);
		dvarSetVariantHook2.installHook();

		// disallow servers to set dvars on the client that are saved
		call(0x59386A, ServerCommand_SetClientDvarHook, PATCH_CALL);
	}
}