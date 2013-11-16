#include "StdInc.h"

#define IW4M_OLD_CODE

CallHook uiLoadHook1;
DWORD uiLoadHook1Loc = 0x60B4AC;

CallHook ffLoadHook1;
DWORD ffLoadHook1Loc = 0x506BC7;

CallHook ffLoadHook2;
DWORD ffLoadHook2Loc = 0x506B25;

void __cdecl UILoadHook1(XZoneInfo* data, int count, int unknown) {
	XZoneInfo newData[5];
	memcpy(&newData[0], data, sizeof(XZoneInfo) * 2);
	newData[0].name = "dlc1_ui_mp";
	newData[0].type1 = 3;
	newData[0].type2 = 0;
	newData[1].name = "dlc2_ui_mp";
	newData[1].type1 = 3;
	newData[1].type2 = 0;

	int count2 = 2;

	#ifdef WE_DO_WANT_NUI
	if ((*(DWORD*)0x11D2104 & 4) == 0 && !GAME_FLAG(GAME_FLAG_DEDICATED))
	{
		newData[2].name = "ui_viewer_mp";
		//newData[2].name = "mp_bog_sh";
		newData[2].type1 = 2;
		newData[2].type2 = 0;

		count2++;

		newData[3].name = "team_us_army";
		newData[3].type1 = 2;
		newData[3].type2 = 0;

		count2++;
	}
	#endif

	/*newData[2].name = "ui_viewer_mp";
	newData[2].type1 = 4;
	newData[2].type2 = 0;*/

	return DB_LoadXAssets(newData, count2, unknown);
}

void __declspec(naked) UILoadHook1Stub() {
	__asm jmp UILoadHook1
}

void __cdecl FFLoadHook1(XZoneInfo* data, int count, int unknown) {
	int newCount = count + 2;

	#ifdef WE_DO_WANT_NUI
	newCount += 2;
	#endif

	static XZoneInfo newData[20];
	memcpy(&newData[0], data, sizeof(XZoneInfo) * count);

	newData[0].type1 = 1;
	newData[1].type1 = 1;
	#ifndef IW4M_OLD_CODE
	newData[0].name = newData[2].name;
	newData[1].name = newData[3].name;

	newData[0].type1 = newData[2].type1;
	newData[0].type2 = newData[2].type2;

	newData[1].type1 = newData[3].type1;
	newData[1].type2 = newData[3].type2;

	newData[2].name = "common_gametype_mp";
	newData[3].name = "localized_common_gametype_mp";
	#endif

	newData[count].name = "dlc1_ui_mp";
	newData[count].type1 = 2;
	newData[count].type2 = 0;
	newData[count + 1].name = "dlc2_ui_mp";
	newData[count + 1].type1 = 2;
	newData[count + 1].type2 = 0;

	/*newData[count + 2].name = "weap_trey";
	newData[count + 2].type1 = newData[2].type1;
	newData[count + 2].type2 = newData[2].type2;*/

	int i = count + 2;

	#ifdef WE_DO_WANT_NUI
	if (!GAME_FLAG(GAME_FLAG_DEDICATED))
	{
		newData[i].name = "ui_viewer_mp";
		//newData[i].name = "mp_bog_sh";
		newData[i].type1 = 2;
		newData[i].type2 = 0;

		i++;

		newData[i].name = "team_us_army";
		newData[i].type1 = 2;
		newData[i].type2 = 0;

		i++;
	}
	#endif

	/*newData[count + 3].name = newData[3].name;
	newData[count + 3].type1 = newData[3].type1;
	newData[count + 3].type2 = newData[3].type2;
	newData[2].name = "iw4_wc_shaders_min";
	newData[2].type1 = newData[2].type1;
	newData[2].type2 = 0;
	newData[3].name = "ui_viewer_mp";
	newData[3].type1 = 4;
	newData[3].type2 = 0;*/
	
	return DB_LoadXAssets(newData, newCount, unknown);
}

void __declspec(naked) FFLoadHook1Stub() {
	__asm {
		jmp FFLoadHook1
	}
}

void __cdecl FFLoadHook2(XZoneInfo* data, int count, int unknown) {
	int newCount = count + 2;

	XZoneInfo newData[20];
	memcpy(&newData[0], data, sizeof(XZoneInfo) * count);

	//newData[2].name = newData[1].name;
	//newData[1].name = newData[0].name;
	//newData[0].name = "patch_code_post_gfx_mp";
	//newData[1].name = "l_code_post_gfx_mp";
	//newData[2].name = "w2_shaders";

	newData[2].name = "patch_code_post_gfx_mp";

	//newData[count].name = "w2_shaders";
	//newData[count].type1 = 0;
	//newData[count].type2 = 0;

	count++;

	newData[count].name = "patch_alter_mp";
	newData[count].type1 = 0;
	newData[count].type2 = 0;

	return DB_LoadXAssets(newData, newCount, unknown);
}

void __declspec(naked) FFLoadHook2Stub() {
	__asm {
		jmp FFLoadHook2
	}
}

// zone\dlc patches
CallHook zoneLoadHook1;
DWORD zoneLoadHook1Loc = 0x5BC82C;

CallHook zoneLoadHook2;
DWORD zoneLoadHook2Loc = 0x4CCE08;

char zone_language[MAX_PATH];
char* loadedPath = "";
char* zonePath = "";

#ifdef IW4M_OLD_CODE
char* zone_dlc = "zone\\dlc\\";
char* zone_alter = "zone\\alter\\";
#endif

dvar_t** fs_basepath = (dvar_t**)0x63D0CD4;
dvar_t** fs_cdpath = (dvar_t**)0x63D0BB0;

char* GetZoneLocation(const char* name) 
{
	static char path[MAX_PATH];

	#ifdef IW4M_OLD_CODE
	_snprintf(path, MAX_PATH, "%s\\zone\\%s\\%s.ff", (*fs_basepath)->current.string, "alter\\", name);
	
	if (FileExists(path)) {
		return path;
	}

	if ((*fs_cdpath)->current.string[0])
	{
		_snprintf(path, MAX_PATH, "%s\\zone\\%s\\%s.ff", (*fs_cdpath)->current.string, "alter\\", name);

		if (FileExists(path)) {
			return path;
		}
	}

	_snprintf(path, MAX_PATH, "%s\\zone\\%s\\%s.ff", (*fs_basepath)->current.string, "dlc\\", name);

	if (FileExists(path)) {
		return path;
	}

	if ((*fs_cdpath)->current.string[0])
	{
		_snprintf(path, MAX_PATH, "%s\\zone\\%s\\%s.ff", (*fs_cdpath)->current.string, "dlc\\", name);

		if (FileExists(path)) {
			return path;
		}
	}

	DWORD getLang = 0x45CBA0;
	char* language;

	__asm {
		call getLang
		mov language, eax
	}

	_snprintf(path, MAX_PATH, "%s\\zone\\%s\\%s.ff", (*fs_basepath)->current.string, language, name);

	if (FileExists(path)) {
		sprintf(path, "zone\\%s\\", language);
		return path;
	}

	if ((*fs_cdpath)->current.string[0])
	{
		_snprintf(path, MAX_PATH, "%s\\zone\\%s\\%s.ff", (*fs_cdpath)->current.string, language, name);

		if (FileExists(path)) {
			return path;
		}
	}

	return NULL;

	#else
	
	_snprintf(path, MAX_PATH, "%s\\GameData\\XAssets\\%s\\%s.ff", (*fs_basepath)->current.string, "w2\\", name);
	
	if (FileExists(path)) {
		sprintf(path, "%s\\GameData\\XAssets\\w2\\", (*fs_basepath)->current.string);
		return path;
	}

	if ((*fs_cdpath)->current.string[0])
	{
		_snprintf(path, MAX_PATH, "%s\\GameData\\XAssets\\%s\\%s.ff", (*fs_cdpath)->current.string, "w2\\", name);

		if (FileExists(path)) {
			sprintf(path, "%s\\GameData\\XAssets\\w2\\", (*fs_cdpath)->current.string);
			return path;
		}
	}

	_snprintf(path, MAX_PATH, "%s\\GameData\\XAssets\\%s\\%s.ff", (*fs_basepath)->current.string, "dlc\\", name);

	if (FileExists(path)) {
		sprintf(path, "%s\\GameData\\XAssets\\dlc\\", (*fs_basepath)->current.string);
		return path;
	}

	if ((*fs_cdpath)->current.string[0])
	{
		_snprintf(path, MAX_PATH, "%s\\GameData\\XAssets\\%s\\%s.ff", (*fs_cdpath)->current.string, "dlc\\", name);

		if (FileExists(path)) {
			sprintf(path, "%s\\GameData\\XAssets\\dlc\\", (*fs_cdpath)->current.string);
			return path;
		}
	}

	DWORD getLang = 0x45CBA0;
	char* language;

	__asm {
		call getLang
		mov language, eax
	}

	_snprintf(path, MAX_PATH, "%s\\GameData\\XAssets\\%s\\%s.ff", (*fs_basepath)->current.string, language, name);

	if (FileExists(path)) {
		sprintf(path, "%s\\GameData\\XAssets\\%s\\", (*fs_basepath)->current.string, language);
		return path;
	}

	if ((*fs_cdpath)->current.string[0])
	{
		_snprintf(path, MAX_PATH, "%s\\GameData\\XAssets\\%s\\%s.ff", (*fs_cdpath)->current.string, language, name);

		if (FileExists(path)) {
			sprintf(path, "%s\\GameData\\XAssets\\%s\\", (*fs_cdpath)->current.string, language);
			return path;
		}
	}

	return NULL;
	#endif
}

char* GetZonePath(const char* fileName)
{
	// we do it a lot simpler than IW did.
	return GetZoneLocation(fileName);
}

void __declspec(naked) ZoneLoadHook1Stub() {
	__asm {
		mov loadedPath, esi // MAKE SURE TO EDIT THIS REGISTER FOR OTHER EXE VERSIONS
	}

	zonePath = GetZonePath(loadedPath);

	__asm {
		mov eax, zonePath
		retn
	}
}

void __declspec(naked) ZoneLoadHook2Stub() {
	__asm {
		mov loadedPath, eax
	}

	zonePath = GetZonePath(loadedPath);

	__asm {
		mov eax, zonePath
		retn
	}
}

StompHook getZoneRootHook;
DWORD getZoneRootHookLoc = 0x4326E0;

const char* GetZoneRootHookFunc(const char* zoneName)
{
	char filename[260];

	if ((*fs_cdpath)->current.string[0])
	{
		_snprintf(filename, sizeof(filename) - 1, "%s\\%s", (*fs_cdpath)->current.string, zoneName);

		if (GetFileAttributes(filename) != INVALID_FILE_ATTRIBUTES)
		{
			return (*fs_cdpath)->current.string;
		}
	}

	return (*fs_basepath)->current.string;
}

// RAWFILE SUPPORT
// more functions
typedef void* (__cdecl * LoadModdableRawfile_t)(int a1, const char* filename);
LoadModdableRawfile_t LoadModdableRawfile = (LoadModdableRawfile_t)0x61ABC0;

// wrapper function for LoadModdableRawfile
void* __cdecl LoadModdableRawfileFunc(const char* filename, void* buffer, int bufSize) {
	// call LoadModdableRawfile
	// we should actually return buffer instead, but as we can't make LoadModdableRawfile load to a custom buffer
	// and code shouldn't use the 'buffer' parameter directly, this should work in most cases
	// even then, we can hardly properly memcpy to the buffer as we don't know the actual file buffer size
	return LoadModdableRawfile(0, filename);
}

void __declspec(naked) LoadModdableRawfileStub() {
	__asm jmp LoadModdableRawfileFunc
}

// wrapper function for LoadModdableRawfile
void* __cdecl LoadModdableRawfileFunc2(const char* filename) {
	// call LoadModdableRawfile
	// we should actually return buffer instead, but as we can't make LoadModdableRawfile load to a custom buffer
	// and code shouldn't use the 'buffer' parameter directly, this should work in most cases
	// even then, we can hardly properly memcpy to the buffer as we don't know the actual file buffer size
	return LoadModdableRawfile(0, filename);
}

void __declspec(naked) LoadModdableRawfileStub2() {
	__asm jmp LoadModdableRawfileFunc2
}

CallHook rawFileHook1;
DWORD rawFileHook1Loc = 0x632155;

CallHook rawFileHook2;
DWORD rawFileHook2Loc = 0x5FA46C;

CallHook rawFileHook3;
DWORD rawFileHook3Loc = 0x5FA4D6;

CallHook rawFileHook4;
DWORD rawFileHook4Loc = 0x6321EF;

CallHook rawFileHook5;
DWORD rawFileHook5Loc = 0x630A88;

CallHook rawFileHook6;
DWORD rawFileHook6Loc = 0x59A6F8;

CallHook rawFileHook7;
DWORD rawFileHook7Loc = 0x57F1E6;

CallHook rawFileHook8;
DWORD rawFileHook8Loc = 0x57ED36;

CallHook rawFileHook9;
DWORD rawFileHook9Loc = 0x4C77A2;

CallHook rawFileHook10;
DWORD rawFileHook10Loc = 0x4E46D1;

CallHook zoneTestHook;
DWORD zoneTestHookLoc = 0x4CCE23;

void ZoneTestHookFunc(char* buffer, int length, const char* format, const char* absPath, const char* zonePath, const char* zoneName)
{
	_snprintf(buffer, length, "%s%s.ff", zonePath, zoneName);
}

CallHook zoneSprintfHook;
DWORD zoneSprintfHookLoc = 0x4B2F15;

void ZoneSprintfHookFunc(char* buffer, int length, const char* format, const char* absPath, const char* zonePath, const char* zoneName)
{
	_snprintf(buffer, length, "%s%s", zonePath, zoneName);
}

bool IgnoreEntityHookFunc(const char* entity);

void ReallocEntries();

typedef int (__cdecl * DB_GetXAssetSizeHandler_t)();

void** DB_XAssetPool = (void**)0x7998A8;
unsigned int* g_poolSize = (unsigned int*)0x7995E8;

DB_GetXAssetSizeHandler_t* DB_GetXAssetSizeHandlers = (DB_GetXAssetSizeHandler_t*)0x799488;

void* ReallocateAssetPool(assetType_t type, unsigned int newSize)
{
	int elSize = DB_GetXAssetSizeHandlers[type]();
	void* poolEntry = malloc(newSize * elSize);
	DB_XAssetPool[type] = poolEntry;
	g_poolSize[type] = newSize;
	return poolEntry;
}

void PatchMW2_Load()
{
	// rawfile moddability
	rawFileHook1.initialize(rawFileHook1Loc, LoadModdableRawfileStub);
	rawFileHook1.installHook();

	rawFileHook2.initialize(rawFileHook2Loc, LoadModdableRawfileStub);
	rawFileHook2.installHook();

	rawFileHook3.initialize(rawFileHook3Loc, LoadModdableRawfileStub);
	rawFileHook3.installHook();

	rawFileHook4.initialize(rawFileHook4Loc, LoadModdableRawfileStub2);
	rawFileHook4.installHook();

	rawFileHook5.initialize(rawFileHook5Loc, LoadModdableRawfileStub);
	rawFileHook5.installHook();

	rawFileHook6.initialize(rawFileHook6Loc, LoadModdableRawfileStub);
	rawFileHook6.installHook();

	rawFileHook7.initialize(rawFileHook7Loc, LoadModdableRawfileStub);
	rawFileHook7.installHook();

	rawFileHook8.initialize(rawFileHook8Loc, LoadModdableRawfileStub);
	rawFileHook8.installHook();

	rawFileHook9.initialize(rawFileHook9Loc, LoadModdableRawfileStub);
	rawFileHook9.installHook();

	//rawFileHook10.initialize(rawFileHook10Loc, LoadModdableRawfileStub);
	//rawFileHook10.installHook();

	call(0x444948, LoadModdableRawfileStub, PATCH_CALL); // shock loading
	call(0x444972, LoadModdableRawfileStub, PATCH_CALL); // shock loading
	call(0x61C7FC, LoadModdableRawfileFunc2, PATCH_CALL); // radiant/keys.txt

	// fastfile loading hooks
	uiLoadHook1.initialize(uiLoadHook1Loc, UILoadHook1Stub);
	uiLoadHook1.installHook();

	ffLoadHook1.initialize(ffLoadHook1Loc, FFLoadHook1Stub);
	ffLoadHook1.installHook();

	ffLoadHook2.initialize(ffLoadHook2Loc, FFLoadHook2Stub);
	ffLoadHook2.installHook();

	zoneLoadHook1.initialize(zoneLoadHook1Loc, ZoneLoadHook1Stub);
	zoneLoadHook1.installHook();

	zoneLoadHook2.initialize(zoneLoadHook2Loc, ZoneLoadHook2Stub);
	zoneLoadHook2.installHook();

	getZoneRootHook.initialize(getZoneRootHookLoc, GetZoneRootHookFunc);
	getZoneRootHook.installHook();

	// disable 'ignoring asset' notices
	memset((void*)0x5BB902, 0x90, 5);

	// ignore 'no iwd files found in main'
	memset((void*)0x642A4B, 0x90, 5);

	if (strstr(GetCommandLine(), "fs_cdpath") != NULL)
	{
		// fs_basepath for 'players' -> fs_cdpath
		*(DWORD*)0x482408 = 0x63D0BB0;
	}

	// zone folders -> remove absolute stuff
	zoneTestHook.initialize(zoneTestHookLoc, ZoneTestHookFunc);
	zoneTestHook.installHook();

	zoneSprintfHook.initialize(zoneSprintfHookLoc, ZoneSprintfHookFunc);
	zoneSprintfHook.installHook();

	// move miles to bin/
	*(const char**)0x4F50C9 = "mss";

	// fastfile load weapon hook thing
	*(BYTE*)0x45C670 = 0xC3;
}

bool IgnoreEntityHookFunc(const char* entity)
{
	return (!strncmp(entity, "dyn_", 4) || !strncmp(entity, "node_", 5) || !strncmp(entity, "actor_", 6)/* || !strncmp(entity, "weapon_", 7)*/);
}