// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Structured data handling.
//
// Initial author: NTAuthority
// Started: 2011-06-16
// ==========================================================

#include "StdInc.h"
#include <tinyxml.h>

typedef enum
{
	STRUCTURED_DATA_INT = 0,
	STRUCTURED_DATA_BYTE = 1,
	STRUCTURED_DATA_BOOL = 2,
	STRUCTURED_DATA_STRING = 3,
	STRUCTURED_DATA_ENUM = 4,
	STRUCTURED_DATA_STRUCT = 5,
	STRUCTURED_DATA_INDEXEDARR = 6,
	STRUCTURED_DATA_ENUMARR = 7,
	STRUCTURED_DATA_FLOAT = 8,
	STRUCTURED_DATA_SHORT = 9
} structuredDataType_t;

typedef struct
{
	structuredDataType_t type;
	union
	{
		int index;
	};
	int offset;
} structuredDataItem_t;

typedef struct
{
	const char* name;
	structuredDataItem_t item;
} structuredDataChild_t;

typedef struct
{
	int numChildren;
	structuredDataChild_t* children;
	int unknown1;
	int unknown2;
} structuredDataStruct_t;

typedef struct
{
	int enumIndex;
	structuredDataItem_t item;
} structuredDataEnumArray_t;

typedef struct
{
	const char* key;
	int index;
} structuredDataEnumIndex_t;

typedef struct
{
	int numIndices;
	int unknown;
	structuredDataEnumIndex_t* indices;
} structuredDataEnum_t;

typedef struct
{
	int numItems;
	structuredDataItem_t item;
} structuredDataIndexedArray_t;

typedef struct
{
	int version;
	unsigned int hash;
	int numEnums;
	structuredDataEnum_t* enums;
	int numStructs;
	structuredDataStruct_t* structs;
	int numIndexedArrays;
	structuredDataIndexedArray_t* indexedArrays;
	int numEnumArrays;
	structuredDataEnumArray_t* enumArrays;
	structuredDataItem_t rootItem;
} structuredData_t;

typedef struct
{
	const char* name;
	int unknown;
	structuredData_t* data;
} structuredDataDef_t;

typedef struct
{
	structuredData_t* data;
	structuredDataItem_t* item;
	int offset;
	int error;
} structuredDataFindState_t;

void StructuredData_InitFindState(structuredDataFindState_t* state, structuredData_t* data)
{
	state->data = data;
	state->item = &data->rootItem;
	state->offset = 0;
	state->error = 0;
}

static int _StructuredData_CompareItemName(const void* a, const void* b)
{
	structuredDataChild_t* childA = (structuredDataChild_t*)a;
	structuredDataChild_t* childB = (structuredDataChild_t*)b;

	return (strcmp(childA->name, childB->name));
}

structuredDataItem_t* StructuredData_FindSubItem(structuredDataFindState_t* state, const char* name)
{
	if (state->item->type != STRUCTURED_DATA_STRUCT)
	{
		return 0;
	}

	structuredDataChild_t dummyChild;
	dummyChild.name = name;

	structuredDataStruct_t* structure = &state->data->structs[state->item->index];
	structuredDataChild_t* child = (structuredDataChild_t*)bsearch(&dummyChild, structure->children, structure->numChildren, sizeof(structuredDataChild_t), _StructuredData_CompareItemName);

	if (child)
	{
		structuredDataItem_t* item = &child->item;
		state->item = item;
		state->offset += item->offset;
		return item;
	}

	return 0;
}

static int _StructuredData_CompareIndexName(const void* a, const void* b)
{
	structuredDataEnumIndex_t* indexA = (structuredDataEnumIndex_t*)a;
	structuredDataEnumIndex_t* indexB = (structuredDataEnumIndex_t*)b;

	return (strcmp(indexA->key, indexB->key));
}

structuredDataItem_t* StructuredData_FindEnumIndex(structuredDataFindState_t* state, const char* key)
{
	if (state->item->type != STRUCTURED_DATA_ENUMARR)
	{
		return 0;
	}

	structuredDataEnumIndex_t dummyIndex;
	dummyIndex.key = key;

	structuredDataEnumArray_t* enumeration = &state->data->enumArrays[state->item->index];
	structuredDataEnum_t* indices = &state->data->enums[enumeration->enumIndex];
	structuredDataEnumIndex_t* index = (structuredDataEnumIndex_t*)bsearch(&dummyIndex, indices->indices, indices->numIndices, sizeof(structuredDataEnumIndex_t), _StructuredData_CompareIndexName);

	if (index)
	{
		structuredDataItem_t* item = &enumeration->item;
		state->item = item;
		state->offset += (item->offset * index->index);
		return item;
	}

	return 0;
}

short StructuredData_GetShort(structuredDataFindState_t* state, char* data)
{
	return *(short*)(data + state->offset);
}

int StructuredData_GetInteger(structuredDataFindState_t* state, char* data)
{
	return *(int*)(data + state->offset);
}

static bool isPlayerData = false;

void StructuredData_GetEnumName(char* buffer, size_t size, int enumIndex)
{
	if (!isPlayerData)
	{
		_snprintf(buffer, size, "enum%d", enumIndex);
		return;
	}

	switch (enumIndex)
	{
	case 0:
		_snprintf(buffer, size, "feature");
		break;
	case 1:
		_snprintf(buffer, size, "weapon");
		break;
	case 2:
		_snprintf(buffer, size, "attachment");
		break;
	case 3:
		_snprintf(buffer, size, "challenge");
		break;
	case 4:
		_snprintf(buffer, size, "camo");
		break;
	case 5:
		_snprintf(buffer, size, "specialty");
		break;
	case 6:
		_snprintf(buffer, size, "killstreak");
		break;
	case 7:
		_snprintf(buffer, size, "award");
		break;
	case 8:
		_snprintf(buffer, size, "cardicon");
		break;
	case 9:
		_snprintf(buffer, size, "cardtitle");
		break;
	case 10:
		_snprintf(buffer, size, "cardnameplate");
		break;
	case 11:
		_snprintf(buffer, size, "team");
		break;
	case 12:
		_snprintf(buffer, size, "gametype");
		break;
	default:
		_snprintf(buffer, size, "enum%d", enumIndex);
		break;
	}
}

void StructuredData_GetStructName(char* buffer, size_t size, int structIndex)
{
	if (!isPlayerData)
	{
		_snprintf(buffer, size, "struct%d", structIndex);
		return;
	}

	switch (structIndex)
	{
	case 0:
		_snprintf(buffer, size, "playerdata");
		break;
	case 1:
		_snprintf(buffer, size, "weaponsetup");
		break;
	case 2:
		_snprintf(buffer, size, "customclass");
		break;
	case 3:
		_snprintf(buffer, size, "awarditem");
		break;
	case 4:
		_snprintf(buffer, size, "rounddata");
		break;
	default:
		_snprintf(buffer, size, "struct%d", structIndex);
		break;
	}
}

static char arrayName[64];

const char* StructuredData_GetEnumArrayName(structuredDataEnumArray_t* enumArray, int index)
{
	char enumName[64];
	StructuredData_GetEnumName(enumName, sizeof(enumName), enumArray->enumIndex);
	_snprintf(arrayName, sizeof(arrayName), "%slist%d", enumName, index);
	return arrayName;
}

static char typeName[64];

const char* StructuredData_TypeName(structuredDataItem_t* item, structuredData_t* data)
{
	switch (item->type)
	{
		case STRUCTURED_DATA_BOOL:
			return "bool";
		case STRUCTURED_DATA_BYTE:
			return "byte";
		case STRUCTURED_DATA_ENUM:
			StructuredData_GetEnumName(typeName, sizeof(typeName), item->index);
			return typeName;
			//return "enum";
		case STRUCTURED_DATA_ENUMARR:
			//StructuredData_GetEnumName(typeName, sizeof(typeName), data->enumArrays[item->index].enumIndex);
			return StructuredData_GetEnumArrayName(&data->enumArrays[item->index], item->index);
			//return typeName;
			//return "enumarr";
		case STRUCTURED_DATA_FLOAT:
			return "float";
		case STRUCTURED_DATA_INDEXEDARR:
			return "indexedarr";
		case STRUCTURED_DATA_INT:
			return "int";
		case STRUCTURED_DATA_SHORT:
			return "short";
		case STRUCTURED_DATA_STRING:
			return "string";
		case STRUCTURED_DATA_STRUCT:
			//return "struct";
			StructuredData_GetStructName(typeName, sizeof(typeName), item->index);
			return typeName;
		default:
			return "unknown";
	}
}

TiXmlElement* StructuredData_DumpToXml(structuredData_t* data, structuredDataItem_t* item, const char* name, int offset)
{
	offset += item->offset;

	TiXmlElement* element = new TiXmlElement(StructuredData_TypeName(item, data));

	if (name[0] == '\0')
	{
		element->SetAttribute("size", item->offset);
	}
	else
	{
		element->SetAttribute("name", name);
		element->SetAttribute("offset", offset);
	}

	if (item->type == STRUCTURED_DATA_STRUCT)
	{
		/*structuredDataStruct_t* structure = &data->structs[item->index];

		for (int i = 0; i < structure->numChildren; i++)
		{
			element->LinkEndChild(StructuredData_DumpToXml(data, &structure->children[i].item, structure->children[i].name, offset));
		}*/

		/*char structName[64];
		StructuredData_GetStructName(structName, sizeof(structName), item->index);

		element->SetAttribute("type", structName);*/
	}
	
	if (item->type == STRUCTURED_DATA_ENUM || item->type == STRUCTURED_DATA_ENUMARR)
	{
		/*char enumName[64];
		structuredDataEnum_t* enumeration;

		if (item->type == STRUCTURED_DATA_ENUMARR)
		{
			StructuredData_GetEnumName(enumName, sizeof(enumName), data->enumArrays[item->index].enumIndex);
			enumeration = &data->enums[data->enumArrays[item->index].enumIndex];
		}
		else
		{
			StructuredData_GetEnumName(enumName, sizeof(enumName), item->index);
			enumeration = &data->enums[item->index];
		}*/

		//element->SetAttribute("type", enumName);
		/*for (int i = 0; i < enumeration->numIndices; i++)
		{
			TiXmlElement* enumIndex = new TiXmlElement("index");
			enumIndex->SetAttribute("name", enumeration->indices[i].key);
			enumIndex->SetAttribute("i", enumeration->indices[i].index);
			element->LinkEndChild(enumIndex);
		}*/
	}

	if (/*item->type == STRUCTURED_DATA_ENUMARR || */item->type == STRUCTURED_DATA_INDEXEDARR)
	{
		structuredDataItem_t* subItem;

		if (item->type == STRUCTURED_DATA_ENUMARR)
		{
			subItem = &data->enumArrays[item->index].item;
		}
		else
		{
			subItem = &data->indexedArrays[item->index].item;
		}

		element->LinkEndChild(StructuredData_DumpToXml(data, subItem, "", offset));
	}

	if (item->type == STRUCTURED_DATA_INDEXEDARR)
	{
		element->SetAttribute("length", data->indexedArrays[item->index].numItems);
	}

	if (item->type == STRUCTURED_DATA_STRING)
	{
		element->SetAttribute("length", item->index);
	}

	return element;
}

void StructuredData_DumpDataDef_f()
{
	char rawFileName[255];
	const char* filename = (Cmd_Argc() > 1) ? Cmd_Argv(1) : "mp/playerdata.def";

	isPlayerData = (Cmd_Argc() == 1);

	//_mkdir("raw");
	//_mkdir("raw/mp");

	structuredDataDef_t* data = (structuredDataDef_t*)DB_FindXAssetHeader(ASSET_TYPE_STRUCTUREDDATADEF, filename);
	if (data)
	{
		TiXmlDocument xdoc;
		TiXmlElement* rootElement = new TiXmlElement("structureddatadef");
		rootElement->LinkEndChild(StructuredData_DumpToXml(data->data, &data->data->rootItem, "Root", 0));
		rootElement->SetAttribute("version", data->data->version);

		TiXmlElement* enums = new TiXmlElement("enums");

		// loop through enums
		for (int i = 0; i < data->data->numEnums; i++)
		{
			char enumName[64];
			StructuredData_GetEnumName(enumName, sizeof(enumName), i);

			TiXmlElement* element = new TiXmlElement("enum");
			element->SetAttribute("name", enumName);

			structuredDataEnum_t* enumeration = &data->data->enums[i];
			for (int j = 0; j < enumeration->numIndices; j++)
			{
				TiXmlElement* index = new TiXmlElement("index");
				index->SetAttribute("name", enumeration->indices[j].key);
				index->SetAttribute("index", enumeration->indices[j].index);
				element->LinkEndChild(index);
			}

			enums->LinkEndChild(element);
		}

		TiXmlElement* structs = new TiXmlElement("structs");

		// loop through structs
		for (int i = 0; i < data->data->numStructs; i++)
		{
			char structName[64];
			StructuredData_GetStructName(structName, sizeof(structName), i);

			TiXmlElement* element = new TiXmlElement("struct");
			element->SetAttribute("name", structName);

			structuredDataStruct_t* structure = &data->data->structs[i];
			for (int j = 0; j < structure->numChildren; j++)
			{
				element->LinkEndChild(StructuredData_DumpToXml(data->data, &structure->children[j].item, structure->children[j].name, 0));
			}

			structs->LinkEndChild(element);
		}

		TiXmlElement* enumArrays = new TiXmlElement("enumarrays");

		// loop through enumarrays
		for (int i = 0; i < data->data->numEnumArrays; i++)
		{
			char enumName[64];
			StructuredData_GetEnumName(enumName, sizeof(enumName), data->data->enumArrays[i].enumIndex);

			TiXmlElement* element = new TiXmlElement("enumarray");
			element->SetAttribute("name", StructuredData_GetEnumArrayName(&data->data->enumArrays[i], i));
			element->SetAttribute("enum", enumName);
			element->LinkEndChild(StructuredData_DumpToXml(data->data, &data->data->enumArrays[i].item, "", 0));

			enumArrays->LinkEndChild(element);
		}

		rootElement->LinkEndChild(structs);
		rootElement->LinkEndChild(enumArrays);
		rootElement->LinkEndChild(enums);
		xdoc.LinkEndChild(rootElement);

		sprintf(rawFileName, "raw/%s", filename);
		xdoc.SaveFile(rawFileName);

		return;
	}

	Com_Printf(0, "No such structured data definition.\n");
}

void testCmd()
{
	/*structuredDataFindState_t state;
	structuredDataDef_t* data = (structuredDataDef_t*)DB_FindXAssetHeader(ASSET_TYPE_STRUCTUREDDATADEF, "mp/playerdata.def");
	StructuredData_InitFindState(&state, data->data);
	StructuredData_FindSubItem(&state, "skills");
	StructuredData_FindEnumIndex(&state, "tdm");
	Com_Printf(0, "skills.tdm = %d\n", StructuredData_GetShort(&state, (char*)0x1AD3694));*/
	//printf("meh");
	//int hi = StructuredData_GetInteger(&state, (char*)0x1AD3694);
	//printf("%d", hi);
}

// cmd_function_t
cmd_function_t sddCmd;
cmd_function_t dumpDataDefCmd;

// hook functions
void LiveStorage_DownloadStats(const char* profileName);

// hooks
CallHook downloadStatsHook;
DWORD downloadStatsHookLoc = 0x4B229D;

StompHook writeStatsHook;
DWORD writeStatsHookLoc = 0x682E70;

CallHook loadOldStatsHook;
DWORD loadOldStatsHookLoc = 0x60A846;

void WriteStatsHookStub();
int LoadOldStatsHookFunc(int a1, int a2, void* buffer, int size);

static char mapEntities[1024 * 1024];

StompHook getEntityStringHook;
DWORD getEntityStringHookLoc = 0x432690;

char* GetEntityString()
{
	char* buffer;
	char* ret = NULL;
	char filename[255];
	_snprintf(filename, sizeof(filename), "maps/oilrig.d3dbsp.ents");

	// why the weird casts?
	if (FS_ReadFile((const char*)(&filename), (char**)&buffer) >= 0)
	{
		strcpy(mapEntities, buffer);
		ret = mapEntities;

		FS_FreeFile(buffer);
	}

	return ret;
}

void __declspec(naked) GetEntityStringHookStub()
{
	__asm
	{
		jmp GetEntityString
	}
}

static char svs_statBuffers[MAX_CLIENTS][0x4000];
static char client_statBuffer[0x4000];

char* SV_GetClientStatBuffer(int clientNum)
{
	return svs_statBuffers[clientNum];
}

void __declspec(naked) StructuredDataSizeHookStub()
{
	__asm
	{
		cmp [esp + 4], 1FFCh
		jne doReturn

		mov [esp + 4], 3FFCh

doReturn:
		push esi
		push edi
		mov edi, [esp + 0Ch]

		push 4D5C56h
		retn
	}
}

void PatchMW2_Stats()
{
	/*
	*(DWORD*)0x799990 = (DWORD)"maps/mp/mp_subbase.d3dbsp";
	*(DWORD*)0x799994 = (DWORD)"maps/mp/mp_subbase.d3dbsp";

	getEntityStringHook.initialize(5, (PBYTE)getEntityStringHookLoc);
	getEntityStringHook.installHook(GetEntityStringHookStub, true, false);
	return;
	*/

	Cmd_AddCommand("structuredDataDebug", testCmd, &sddCmd, 0);
	Cmd_AddCommand("dumpDataDef", StructuredData_DumpDataDef_f, &dumpDataDefCmd, 0);

	if (GAME_FLAG(GAME_FLAG_DEDICATED))
	{
		return;
	}

	downloadStatsHook.initialize(downloadStatsHookLoc, LiveStorage_DownloadStats);
	downloadStatsHook.installHook();

	writeStatsHook.initialize(writeStatsHookLoc, WriteStatsHookStub);
	writeStatsHook.installHook();

	loadOldStatsHook.initialize(loadOldStatsHookLoc, LoadOldStatsHookFunc);
	loadOldStatsHook.installHook();

	// don't download stats every frame when not downloaded yet
	memset((void*)0x40F8AD, 0x90, 5);

	*(BYTE*)0x60A830 = 0xB0;
	*(BYTE*)0x60A831 = 0x00;
	*(BYTE*)0x60A832 = 0xC3;

	// EXPERIMENTAL FROM HERE ON
	/*// structured data size hook
	call(0x4D5C50, StructuredDataSizeHookStub, PATCH_JUMP);

	// client stat buffer stuff on server
	call(0x4014C0, SV_GetClientStatBuffer, PATCH_JUMP);*/

}

static char oldStats[8192];
char* statBuffer = (char*)0x1AD3690;

typedef void (__cdecl * LiveStorage_CompleteStatDownload_t)(int, int, int);
LiveStorage_CompleteStatDownload_t LiveStorage_CompleteStatDownload = (LiveStorage_CompleteStatDownload_t)0x4C04C0;

static bool didMigration = false;

bool LiveStorage_MigrateStats()
{
	char* buffer;

	if (!didMigration && FS_ReadFile("md202ef8d_aIW.stat", &buffer) > 0)
	{
		NPID npID;
		NP_GetNPID(&npID);

		NPAsync<NPWriteUserFileResult>* async = NP_WriteUserFile("iw4.stat", npID, (uint8_t*)buffer, 8192);
		async->SetCallback((void (*)(NPAsync<NPWriteUserFileResult>*))LiveStorage_DownloadStats, NULL);
		FS_FreeFile(buffer);

		didMigration = true;
		return true;
	}

	return false;
}

extern int* clientState;

void LiveStorage_DownloadStatsCB(NPAsync<NPGetUserFileResult>* async)
{
	NPGetUserFileResult* result = async->GetResult();

	if (result->result == GetFileResultOK)
	{
		// backup the stats in-memory for regression testing
		memcpy(oldStats, result->buffer, sizeof(oldStats));

		if (*clientState < 3)
		{
			LiveStorage_CompleteStatDownload(0, 0, 0);
		}
	}
	else
	{
		if (result->result == GetFileResultNotFound)
		{
			if (LiveStorage_MigrateStats())
			{
				return;
			}
		}

		// reset stats
		__asm
		{
			push esi
			mov eax, 60A5A0h
			mov esi, 0
			call eax
			pop esi
		}
	}
}

void LiveStorage_DownloadStats(const char* profileName)
{
	NPID npID;
	if (NP_GetNPID(&npID))
	{
		NPAsync<NPGetUserFileResult>* async = NP_GetUserFile("iw4.stat", npID, (uint8_t*)statBuffer, 8192);
		async->SetCallback(LiveStorage_DownloadStatsCB, NULL);
	}
}


static const char* fileID;
static bool useSteamCloud;
static unsigned int bufferSize;

bool WriteStatsFunc(int remote, void* buffer)
{
	// filename buffer
	char filename[256];

	// check buffer size to be valid, though it doesn't matter if we don't do TEA
	if (bufferSize > 8192 || bufferSize & 3) return false; // & 3 is for 4-byte alignment

	// don't do writing for steam cloud
	if (remote) return true;

	// get NPID
	NPID npID;
	NP_GetNPID(&npID);

	// upload to NP if this is the main stat file
	if (strcmp(fileID, "md202ef8d"))
	{
		NP_WriteUserFile("iw4.stat", npID, (uint8_t*)buffer, bufferSize);
	}

	// sprintf a filename
	_snprintf(filename, sizeof(filename), "iw4_%s_%llx.stat", fileID, npID);

	// and write the file
	return FS_WriteFile(filename, "players", buffer, bufferSize);
}

void __declspec(naked) WriteStatsHookStub()
{
	__asm
	{
		// prepare weird optimized parameters
		mov fileID, ecx
		mov useSteamCloud, dl
		mov bufferSize, eax

		// and jump to our normal cdecl function
		jmp WriteStatsFunc
	}
}

int LoadOldStatsHookFunc(int a1, int a2, void* buffer, int size)
{
	if (oldStats[0] != 0)
	{
		memcpy(buffer, oldStats, size);

		return 0;
	}

	return 1;
}