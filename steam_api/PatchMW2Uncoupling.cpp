#include "StdInc.h"
#include <unordered_map>

struct XZoneMemoryEntry
{
	void* memory;
	size_t size;
};

struct XZoneMemory
{
	XZoneMemoryEntry blocks[8];
};

size_t* g_streamPos = (size_t*)0x16E5554;
int* g_streamPosIndex = (int*)0x16E5578;

typedef void (__cdecl * DB_InitStreams_t)(XZoneMemory* memory);
DB_InitStreams_t DB_InitStreams = (DB_InitStreams_t)0x4D0810;

StompHook loadXFileDataHook;
DWORD loadXFileDataHookLoc = 0x445460;

//void* baseMemory[8];
XZoneMemory newMemory;

void DB_LoadXFileData(void* pointer, size_t size)
{
	
}

void UncoupleCommand()
{
	// this is an experiment.
	// it may or may not end up working.

	// patch stream allocation/loading functions
	

	// load the requested zone
	time_t startTime = Com_Milliseconds();

	XZoneInfo asset;
	asset.name = "oilrig";
	asset.type1 = 4;
	asset.type2 = 0;

	DB_LoadXAssets(&asset, 1, 1);

	unsigned int duration = (unsigned int)(Com_Milliseconds() - startTime);

	printf("duration: %d\n", duration);
}

void PatchMW2_Uncoupling()
{
	return;
	static cmd_function_t uncoupleCmd;
	Cmd_AddCommand("uncouple", UncoupleCommand, &uncoupleCmd, 0);
}


/*
struct StreamPositions
{
	void* position[8];
};

static StreamPositions positionStack[32];
static int curPosition = 0;

void Uncouple_PushStreamPosition(StreamPositions* positions, int overrideIndex, size_t override)
{
	memcpy(&positionStack[curPosition], positions, sizeof(StreamPositions));

	if (overrideIndex != -1)
	{
		positionStack[curPosition].position[overrideIndex] = (void*)override;
	}

	curPosition++;
}

StreamPositions Uncouple_PopStreamPosition()
{
	curPosition--;
	return positionStack[curPosition];
}

std::tr1::unordered_map<std::string, StreamPositions> _assetMap;

void Uncouple_InsertCurrentPosition(const char* name)
{
	if (curPosition > 0)
	{
		StreamPositions position = Uncouple_PopStreamPosition();

		if (!strcmp(name, "~navy_seal_dive_gear_spc-rgb&~21343668"))
		{
			return;
		}

		_assetMap[name] = position;
	}
}

void __declspec(naked) Uncouple_PushStreamPositions()
{
	//Uncouple_InsertStreamPosition((StreamPositions*)0x16E5558, *g_streamPosIndex, *g_streamPos);
	Uncouple_PushStreamPosition((StreamPositions*)0x16E5558, *g_streamPosIndex, *g_streamPos);

	__asm
	{
		cmp esi, 0FFFFFFFEh
		jne zeroPointer

		mov eax, 43B290h
		call eax
		mov esi, eax
		retn

zeroPointer:
		xor esi, esi
		retn
	}
}

void Uncouple_ReplaceInsert(void* address)
{
	char* charAddress = (char*)address;
	HookInstall((DWORD)address, (DWORD)Uncouple_PushStreamPositions, 13);
}

void UncoupleCommand()
{
	// this is an experiment.
	// it may or may not end up working.

	// patch 'insert pointer' calls so we have an index table of stream positions
	signature_t signature;
	memset(&signature, 0, sizeof(signature));

	signature.signature = (unsigned char*)"\x83\xFE\xFE\x89\x0D\x00\x00\x00\x00\x75\x09\xE8\x00\x00\x00\x00\x8B\xF0\xEB\x02\x33\xF6\x6A\x01";
	signature.mask = "xxxxx????xxx????xxxxxxxx";
	signature.size = 24;
	signature.logOffset = 0;
	signature.replaceCB = Uncouple_ReplaceInsert;
	signature.replaceOffset = 9;

	ProcessSignature(&signature);

	// load the base zone
	XZoneInfo asset;
	asset.name = "oilrig";
	asset.type1 = 4;
	asset.type2 = 0;

	DB_LoadXAssets(&asset, 1, 1);

	// save this zone's stream stuff
	//memcpy(&baseMemory, (void*)0x16E5558, sizeof(XZoneMemory));

	// allocate new memory
	for (int i = 0; i < 8; i++)
	{
		newMemory.blocks[i].memory = malloc(1024 * 1024);
		newMemory.blocks[i].size = (1024 * 1024);
	}

	DB_InitStreams(&newMemory);

	// patch a few functions
	loadXFileDataHook.initialize(5, (PBYTE)loadXFileDataHookLoc);
	loadXFileDataHook.installHook((void(*)())DB_LoadXFileData, true, false);

	// get a material pointer to mess with
	Material* origPointer = (Material*)DB_FindXAssetHeader(ASSET_TYPE_MATERIAL, "compass_map_oilrig_lvl_3");
	StreamPositions assetPositions = _assetMap["compass_map_oilrig_lvl_3"];

	// done?
	loadXFileDataHook.releaseHook(false);

	printf("0\n");
}
*/