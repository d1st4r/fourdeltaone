// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: apple pie
//
// Initial author: NTAuthority
// Started: 2013-05-26
// ==========================================================

#include "StdInc.h"

static int sv_fps = 20;
static int sv_fpsDiv = 50;
static double sv_fpsFloat = 20.f;

int _divideStuff(int frameCount)
{
	return frameCount / sv_fpsDiv;
}

void __declspec(naked) G_AntiLagGetSnapStub()
{
	__asm
	{
		test ebx, ebx
		jnz divide 

		xor eax, eax
		jmp dostuff

divide:
		//xor edx, edx
		//mov eax, sv_fpsDiv
		//idiv ebx
// 		push ebx
// 		xor edx, edx
// 		mov eax, ecx
// 		mov ebx, sv_fpsDiv
// 		idiv ebx
// 		pop ebx
		push ebx
		call _divideStuff
		add esp, 4h

dostuff:
		mov edx, eax
		mov eax, edx

		push ebp
		push 455986h
		retn
	}
}

void __declspec(naked) _getCachedSnapStub()
{
	__asm
	{
		test ecx, ecx
		jnz divide 

		xor eax, eax
		jmp dostuff

divide:
		//xor edx, edx
		//mov eax, sv_fpsDiv
		//idiv ecx
		push ecx
		call _divideStuff
		add esp, 4h

dostuff:
		mov edx, eax
		mov eax, edx

		push 4DC240h
		retn
	}
}

int VM_ExecuteWaitIntFunc(int waitTime)
{
	return (waitTime * sv_fps);
}

void __declspec(naked) VM_ExecuteWaitIntStub()
{
	__asm
	{
		push eax
		call VM_ExecuteWaitIntFunc
		add esp, 4h

		mov esi, eax

		push 6205E6h
		retn
	}
}

// a char for now, as some places it's used are byte-sized
void PatchFrameRate(char targetFPS)
{
	// SV_ArchiveSnapshot
	*(char*)0x43E35B = -targetFPS;

	// SV_Frame, sort-of
	*(BYTE*)0x62725D = 1000 / targetFPS;

	// antilag snap
	*(BYTE*)0x45598E = 1000 / targetFPS;
	*(BYTE*)0x455991 = 1000 / targetFPS;

	// antilag snapshot getter
	call(0x455974, G_AntiLagGetSnapStub, PATCH_JUMP);

	// cached snap
	*(BYTE*)0x4DC25C = 1000 / targetFPS;
	*(BYTE*)0x4DC26C = 1000 / targetFPS;
	*(BYTE*)0x4DC296 = 1000 / targetFPS;

	// cached snapshot getter
	call(0x4DC22F, _getCachedSnapStub, PATCH_JUMP);

	// VM_Execute int wait statements
	call(0x6205DF, VM_ExecuteWaitIntStub, PATCH_JUMP);

	sv_fps = targetFPS;
	sv_fpsFloat = targetFPS;
	sv_fpsDiv = (1000 / targetFPS);

	// VM_Execute float wait
	*(double**)0x62058F = &sv_fpsFloat;
}

void PatchMW2_FrameRate()
{
	PatchFrameRate(30);
}