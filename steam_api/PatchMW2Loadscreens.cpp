// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Support non-_load.ff load screens for M2 maps.
//
// Initial author: NTAuthority
// Started: 2012-04-28
// ==========================================================

#include "StdInc.h"

// actually a call to DB_LoadXAssets
CallHook loadMapLoadscreenHook;
DWORD loadMapLoadscreenHookLoc = 0x4D807A;

CallHook loadMapLoadscreenHook2;
DWORD loadMapLoadscreenHookLoc2 = 0x4A71A5;

char* GetZonePath(const char* fileName);

static struct  
{
	char filename[64];

	// to return in the hook in UI_DrawMapLevelshot
	menuDef_t* connectMenu;

	// for quick access to replacing
	itemDef_t* connectBackgroundItem;
} g_loadscreens;

menuDef_t* LoadScreens_MessWithConnectMenu(menuDef_t* menu)
{
	menuDef_t* newMenu = (menuDef_t*)malloc(2048); // has to be 2048 due to some stuff being overwritten beyond the end of the supposed struct
	memcpy(newMenu, menu, sizeof(menuDef_t));

	newMenu->items = (itemDef_t**)malloc(sizeof(itemDef_t*) * newMenu->numItems);
	memcpy(newMenu->items, menu->items, sizeof(itemDef_t*) * newMenu->numItems);
	
	itemDef_t* newItem = new itemDef_t;
	memcpy(newItem, newMenu->items[0], sizeof(itemDef_t));

	newMenu->items[0] = newItem;

#ifdef WE_DO_WANT_NUI
	newMenu->numItems = 1; // NUI stuff
#endif
	g_loadscreens.connectMenu = newMenu;
	g_loadscreens.connectBackgroundItem = newItem;

	return newMenu;
}

void Material_Delete(const char* name, const char* imageCondition);
Material* Material_Register(const char* filename);

const char* LoadScreens_Filename()
{
	return (g_loadscreens.filename[0]) ? g_loadscreens.filename : NULL;
}

const char* GetZoneRootHookFunc(const char* zoneName);

void LoadMapLoadscreenHookFunc(XZoneInfo* info, int a2, int a3)
{
	char zonePath[MAX_PATH];
	const char* zoneDir = GetZonePath(info[0].name);
	_snprintf(zonePath, sizeof(zonePath) - 1, "%s\\%s.ff", zoneDir, info[0].name);

	if (FileExists(zonePath))
	{
		g_loadscreens.filename[0] = '\0';
		DB_LoadXAssets(info, a2, a3);

		if (g_loadscreens.connectBackgroundItem)
		{
			g_loadscreens.connectBackgroundItem->window.background = (Material*)DB_FindXAssetHeader(ASSET_TYPE_MATERIAL, "$levelbriefing");
		}
	}
	else
	{
		// cut off the last _load
		strrchr((char*)info[0].name, '_')[0] = '\0';

		// only allow one custom levelbriefing to be loaded at a time
		Material_Delete("$levelbriefingX", g_loadscreens.filename);

		sprintf_s(g_loadscreens.filename, sizeof(g_loadscreens.filename), "loadscreen_%s", info[0].name);

		if (g_loadscreens.connectBackgroundItem)
		{
			// replace the connect menu's thing with 'our' levelbriefing
			g_loadscreens.connectBackgroundItem->window.background = Material_Register("$levelbriefingX");
		}
	}
}

typedef menuDef_t* (__cdecl * Menus_FindByName_t)(void* uiInfoArray, const char* name);
Menus_FindByName_t Menus_FindByName = (Menus_FindByName_t)0x487240;

CallHook getConnectMenuHook;
DWORD getConnectMenuHookLoc = 0x428DE1;

// type is ASSET_TYPE_MENU, connect is "connect". I'm psychic.
menuDef_t* GetConnectMenuHookFunc(assetType_t type, const char* connect)
{
	//return g_loadscreens.connectMenu;
	return Menus_FindByName((void*)0x62E2858, "connect");
}

void PatchMW2_Loadscreens()
{
	loadMapLoadscreenHook.initialize(loadMapLoadscreenHookLoc, LoadMapLoadscreenHookFunc);
	loadMapLoadscreenHook.installHook();

	loadMapLoadscreenHook2.initialize(loadMapLoadscreenHookLoc2, LoadMapLoadscreenHookFunc);
	loadMapLoadscreenHook2.installHook();

	getConnectMenuHook.initialize(getConnectMenuHookLoc, GetConnectMenuHookFunc);
	getConnectMenuHook.installHook();
}