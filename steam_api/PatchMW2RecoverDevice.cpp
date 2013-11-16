// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Handlers for recovering lost D3D9 (non-Ex)
//          devices.
//
// Initial author: NTAuthority
// Started: 2013-02-07
// ==========================================================

#include "StdInc.h"

void ReleaseCModels();
void ReloadCModels();

void ForAllCustomImages(void (*handler)(void*));

CallHook beginRecoverDeviceHook;
DWORD beginRecoverDeviceHookLoc = 0x5082BD;

typedef void (__cdecl * DB_EnumXAssets_t)(assetType_t type, void (*handler)(void*), int maybe0);
static DB_EnumXAssets_t DB_EnumXAssets = (DB_EnumXAssets_t)0x42A770;

void BeginRecoverDeviceHookFunc(assetType_t imageType, void (*handler)(void*), int stuff)
{
	ReleaseCModels();
	ForAllCustomImages(handler);

	if (imageType == ASSET_TYPE_IMAGE)
	{
		DB_EnumXAssets(imageType, handler, stuff);
	}
}

CallHook endRecoverDeviceHook;
DWORD endRecoverDeviceHookLoc = 0x508343;

void EndRecoverDeviceHookFunc(assetType_t imageType, void (*handler)(void*), int stuff)
{
	ReloadCModels();
	ForAllCustomImages(handler);

	if (imageType == ASSET_TYPE_IMAGE)
	{
		DB_EnumXAssets(imageType, handler, stuff);
	}
}

void ClearAllCustomMaterials();

void PreVidRestartHookFunc()
{
	BeginRecoverDeviceHookFunc(ASSET_TYPE_COL_MAP_MP, (void(*)(void*))0x51F790, 0);
	ClearAllCustomMaterials();
}

void PostVidRestartHookFunc()
{
	EndRecoverDeviceHookFunc(ASSET_TYPE_COL_MAP_MP, (void(*)(void*))0x51F7B0, 0);
}

CallHook postVidRestartHook;
DWORD postVidRestartHookLoc = 0x4CA3A7;

void __declspec(naked) PostVidRestartHookStub()
{
	__asm
	{
		call PostVidRestartHookFunc
		jmp postVidRestartHook.pOriginal
	}
}

void PatchMW2_RecoverDevice()
{
	beginRecoverDeviceHook.initialize(beginRecoverDeviceHookLoc, BeginRecoverDeviceHookFunc);
	beginRecoverDeviceHook.installHook();

	endRecoverDeviceHook.initialize(endRecoverDeviceHookLoc, EndRecoverDeviceHookFunc);
	endRecoverDeviceHook.installHook();

	call(0x4CA2FD, PreVidRestartHookFunc, PATCH_CALL);

	postVidRestartHook.initialize(postVidRestartHookLoc, PostVidRestartHookStub);
	postVidRestartHook.installHook();
}