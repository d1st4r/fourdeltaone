// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Extending .arena map name size to 32, as needed
//          for mp_estate_tropical.
//
// Initial author: NTAuthority
// Started: 2013-07-18
// ==========================================================

#include "StdInc.h"
#include "Hooking.h"

static newMapArena_t newArenas[128];

void __declspec(naked) _arenaMapOffsetHook1()
{
	__asm
	{
		lea eax, [esi + newMapArena_t::mapName]
		push eax
		push ebx
		
		push 420725h
		retn
	}
}

void __declspec(naked) _arenaMapOffsetHook2()
{
	__asm
	{
		lea eax, [edi + newMapArena_t::mapName]
		push eax
		push edx
		
		push 49BD3Eh
		retn
	}
}

void __declspec(naked) _arenaMapOffsetHook3()
{
	__asm
	{
		lea eax, [esi + newMapArena_t::mapName]
		push eax
		push edx

		push 63279Eh
		retn
	}
}

// 3edgy5me
auto PatchMW2_ArenaLength() -> void
{
	int patchArray[] = { 0x62E6964, 0x62E6984, 0x62E73A4, 0x62E73AC, 0x62E7430 };
	SearchAndPatch(patchArray, 5, 0x62E6964, (DWORD)newArenas[0].other);

	patchArray[0] = 0x62E6934;
	SearchAndPatch(patchArray, 1, 0x62E6934, (DWORD)newArenas);

	auto PatchArenaOffset = [] (int offset)
	{
		char* ptr = (char*)offset;

		while (*(int*)ptr != 0xB00)
		{
			ptr++;
		}

		*(int*)ptr = sizeof(newMapArena_t);
	};

	PatchArenaOffset(0x4064DC);
	PatchArenaOffset(0x417800);
	PatchArenaOffset(0x420734);
	PatchArenaOffset(0x42F26F);
	PatchArenaOffset(0x49BD4D);
	PatchArenaOffset(0x4A960A);
	PatchArenaOffset(0x4A963D);
	PatchArenaOffset(0x4A9673);
	PatchArenaOffset(0x4A96A1);
	PatchArenaOffset(0x4A96E5);
	PatchArenaOffset(0x4A96FB);
	PatchArenaOffset(0x4A9758);
	PatchArenaOffset(0x4A979D);
	PatchArenaOffset(0x4A97BA);
	PatchArenaOffset(0x4D0777);
	PatchArenaOffset(0x630B27);
	PatchArenaOffset(0x630B7F);
	PatchArenaOffset(0x631EBA);
	PatchArenaOffset(0x6327AD);

	*(DWORD*)0x4A9616 = (DWORD)newArenas[0].mapName;
	*(DWORD*)0x4A9703 = (DWORD)newArenas[0].mapName;

	call(0x420720, _arenaMapOffsetHook1, PATCH_JUMP);
	call(0x49BD39, _arenaMapOffsetHook2, PATCH_JUMP);
	call(0x632799, _arenaMapOffsetHook3, PATCH_JUMP);

	*(BYTE*)0x4A95F8 = 32;
}