// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Modern Warfare 2 patches: asset restrictions and
//          pre-load modifications.
//
// Initial author: NTAuthority
// Started: 2011-05-20
// ==========================================================

#include "StdInc.h"
#include <unordered_map>

static std::unordered_map<DWORD, bool> deadAssets;

// TODO: provide generic hooking for these calls
bool AssetRestrict_RestrictFromMaps(assetType_t type, const char* name, const char* zone);
void AssetRestrict_PreLoadFromMaps(assetType_t type, void* entry, const char* zone);
void AssetRestrict_PreLoadFromExperimental(assetType_t type, void* entry, const char* zone);

StompHook dbAddXAssetHook;
DWORD dbAddXAssetHookLoc = 0x5BB650;
DWORD dbAddXAssetHookRet = 0x5BB657;

typedef const char* (__cdecl * DB_GetXAssetNameHandler_t)(void* asset);
DB_GetXAssetNameHandler_t* DB_GetXAssetNameHandlers = (DB_GetXAssetNameHandler_t*)0x799328;

char CanWeLoadAsset(assetType_t type, void* entry)
{
	const char* name = DB_GetXAssetNameHandlers[type](entry);

	if (!name)
	{
		return 2;
	}

	if (type == ASSET_TYPE_WEAPON)
	{
		// somewhat-workaround for issue 'could not load weapon "destructible_car"' and cars not doing any damage
		if (strcmp(CURRENT_ZONE_NAME, "code_post_gfx_mp") && (!strcmp(name, "none") || !strcmp(name, "destructible_car"))) // common_tc_mp also has these
		{
			//return 1;
		}

		if (!strcmp(CURRENT_ZONE_NAME, "patch_mp"))
		{
			return 0;
		}
	}

	if (AssetRestrict_RestrictFromMaps(type, name, CURRENT_ZONE_NAME))
	{
		OutputDebugString(va("restricting %s\n", name));

		//deadAssets[*(DWORD*)entry] = true;
		return 2;
	}

	return 1;
}

void DoBeforeLoadAsset(assetType_t type, void** entry)
{
	if (entry)
	{
		AssetRestrict_PreLoadFromMaps(type, *entry, CURRENT_ZONE_NAME);
		AssetRestrict_PreLoadFromExperimental(type, *entry, CURRENT_ZONE_NAME);
	}
}

void* LoadDefaultAsset(assetType_t atype)
{
	void* defaultAsset;

	__asm
	{
		push edi
		mov edi, atype
		mov eax, 5BB210h
		call eax
		pop edi

		mov defaultAsset, eax
	}

	static void* retStuff[2];
	retStuff[0] = 0;
	retStuff[1] = defaultAsset;

	return retStuff;
}

#pragma optimize("", off)
void __declspec(naked) DB_AddXAssetHookStub()
{
	__asm
	{
		mov eax, [esp + 4]
		mov ecx, [esp + 8]

		push ecx
		push eax
		call CanWeLoadAsset
		add esp, 08h

		cmp al, 2h
		je doDefault

		test al, al
		jz doNotLoad

		mov eax, [esp + 4]
		mov ecx, [esp + 8]
		push ecx
		push eax
		call DoBeforeLoadAsset
		add esp, 08h

		mov eax, [esp + 8]
		sub esp, 14h
		jmp dbAddXAssetHookRet

doNotLoad:
		mov eax, [esp + 8]
		retn

doDefault:
		mov eax, [esp + 4]

		push eax
		call LoadDefaultAsset
		add esp, 4h

		retn
	}
}
#pragma optimize("", on)

#if 0
StompHook markAssetHook;
DWORD markAssetHookLoc = 0x5BBA30;
DWORD markAssetHookRet = 0x5BBA38;

bool MarkAssetHookFunc(DWORD ptr)
{
	return (deadAssets.find(ptr) != deadAssets.end());
}

void __declspec(naked) MarkAssetHookStub()
{
	__asm
	{
		mov eax, [esp + 4h]

		push eax
		call MarkAssetHookFunc
		add esp, 4

		cmp al, 1
		je returnFalse

		sub esp, 8
		push edi
		mov edi, [esp + 0Ch + 4]
		jmp markAssetHookRet

returnFalse:
		xor eax, eax
		retn
	}
}
#endif

void PatchMW2_AssetRestrict()
{
	dbAddXAssetHook.initialize(dbAddXAssetHookLoc, DB_AddXAssetHookStub, 7);
	dbAddXAssetHook.installHook();

	//markAssetHook.initialize(markAssetHookLoc, MarkAssetHookStub);
	//markAssetHook.installHook();
}