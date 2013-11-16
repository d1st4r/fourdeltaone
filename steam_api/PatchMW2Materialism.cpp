// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Material/image modification code
//
// Initial author: NTAuthority
// Started: 2011-06-09
// ==========================================================

#include "StdInc.h"
#include <google/dense_hash_map>
#include <string>
#include <xhash>

using google::dense_hash_map;

//static dense_hash_map<const char*, Material*, asset_hash<const char*>> materialTable;
static dense_hash_map<std::string, Material*, stdext::hash_compare<std::string>> materialTable;

unsigned int R_HashString(const char* string)
{
	unsigned int hash = 0;

	while (*string)
	{
		hash = (*string | 0x20) ^ (33 * hash);
		string++;
	}

	return hash;
}

typedef int (__cdecl * _Reader_t)(int, int); // defaulting to int for now
typedef bool (__cdecl * Image_LoadFromFileWithReader_t)(GfxImage* image, _Reader_t reader);
Image_LoadFromFileWithReader_t Image_LoadFromFileWithReader = (Image_LoadFromFileWithReader_t)0x53ABF0;

typedef void (__cdecl * Image_Release_t)(GfxImage* image);
Image_Release_t Image_Release = (Image_Release_t)0x51F010;

#pragma optimize("", off)
void Image_Setup_(GfxImage* image, short width, short height, short depth, unsigned int flags, int format)
{
	DWORD func = 0x54AF50;

	__asm
	{
		push edi
		mov eax, image
		mov di, width
		push format
		push flags
		push 0
		push depth
		push height
		call func
		add esp, 14h
		pop edi
	}
}

void Image_Setup(GfxImage* image, short width, short height, short depth, unsigned int flags, int format)
{
	return Image_Setup_(image, width, height, depth, flags, format);
}
#pragma optimize("", on)

// a3 is 'disable data needed for smoothing' or so, maybe 'do not auto-generate mipmaps'?
GfxImage* Image_Alloc(const char* name, char a2, char a3, char a4)
{
	GfxImage* memory = (GfxImage*)malloc(sizeof(GfxImage) + strlen(name) + 1);
	memset(memory, 0, sizeof(GfxImage));

	memory->a2 = a2;
	memory->a3 = a3;
	memory->a4 = a4;
	memory->name = ((char*)memory) + sizeof(GfxImage);
	strcpy(memory->name, name);

	return memory;
}

GfxImage* Image_Load(const char* name)
{
	GfxImage* image = Image_Alloc(name, 3, 0, 0);
	Image_LoadFromFileWithReader(image, (_Reader_t)/*0x42ECA0*/0x46CBF0);
	return image;
}

static unsigned char stateMapCopy[8];
static bool stateMapCopied = false;

static void Material_InitializeStateMap()
{
	if (stateMapCopied) return;

	Material* base = (Material*)DB_FindXAssetHeader(ASSET_TYPE_MATERIAL, "ui_cursor");
	memcpy(stateMapCopy, base->stateMap, 8);
	stateMapCopied = true;
}

#pragma optimize("", off)
Material* Material_CreateWithImage(const char* name, GfxImage* image)
{
	Material* material = new Material;
	memset(material, 0, sizeof(Material));

	material->maps = new MaterialTextureDef;
	memset(material->maps, 0, sizeof(MaterialTextureDef));

	Material_InitializeStateMap();

	material->name = name;
	//material->flags = 0x2F00;
	material->flags = 0x2200;
	material->animationX = 1;
	material->animationY = 1;
	material->unknown2 = 0xFFFFFFFF;
	material->unknown3 = 0xFFFFFF00;
	memset(material->unknown4, 0xFF, sizeof(material->unknown4));
	material->numMaps = 1;
	material->unknownCount2 = 1;
	material->unknown6 = 3;
	material->unknown7 = 4;
	material->techniqueSet = (MaterialTechniqueSet*)DB_FindXAssetHeader(ASSET_TYPE_TECHSET, "2d");

	material->maps->typeHash = R_HashString("colorMap");
	material->maps->firstCharacter = 'c';
	material->maps->secondLastCharacter = 'p';
	material->maps->unknown = 0xE2; // causes some nice filtering
	material->maps->image = image;

	material->stateMap = stateMapCopy;

	return material;
}
#pragma optimize("", on)

Material* Material_Create(const char* name, const char* imageFile)
{
	return Material_CreateWithImage(name, Image_Load(imageFile));
}

Material* Material_Obtain(const char* name, const char* imageFile)
{
	if (!materialTable[name])
	{
		materialTable[name] = Material_Create(name, imageFile);
	}

	return materialTable[name];
}

Material* Material_ObtainImage(const char* name, GfxImage* image)
{
	if (!materialTable[name])
	{
		materialTable[name] = Material_CreateWithImage(name, image);
	}

	return materialTable[name];
}

void Material_Delete(const char* name, const char* imageCondition)
{
	if (materialTable[name])
	{
		if (!imageCondition)
		{
			materialTable[name] = NULL;
		}
		else
		{
			if (!_stricmp(materialTable[name]->maps[0].image->name, imageCondition))
			{
				Image_Release(materialTable[name]->maps[0].image);
				free(materialTable[name]->maps[0].image);
				materialTable[name] = NULL;
			}
		}
	}
}

const char* LoadScreens_Filename();

GfxImage* g_nuiImage;
Material* UserAvatars_GetMaterial(const char* filename);

Material* Material_Register(const char* filename)
{
	// this really needs a better way of doing them.
	// maybe a fastfile building tool? I think the X360 people already have code to write the 2d techset out...
	if (!_stricmp(filename, "preview_oilrig"))
	{
		return Material_Obtain(filename, filename);
	}

	if (!_stricmp(filename, "preview_gulag"))
	{
		return Material_Obtain(filename, filename);
	}

	if (!_stricmp(filename, "preview_invasion"))
	{
		return Material_Obtain(filename, filename);
	}

	if (!_stricmp(filename, "preview_iw4_credits"))
	{
		return Material_Obtain(filename, filename);
	}

	if (!_stricmp(filename, "preview_estate"))
	{
		return Material_Obtain(filename, filename);
	}

	if (!_stricmp(filename, "preview_co_hunted"))
	{
		return Material_Obtain(filename, filename);
	}

	if (!_stricmp(filename, "preview_so_hunted"))
	{
		return Material_Obtain(filename, filename);
	}

	if (!_stricmp(filename, "preview_arcadia"))
	{
		return Material_Obtain(filename, filename);
	}

	if (!_stricmp(filename, "preview_contingency"))
	{
		return Material_Obtain(filename, filename);
	}

	if (!_stricmp(filename, "preview_airport"))
	{
		return Material_Obtain(filename, filename);
	}

	if (!_stricmp(filename, "preview_roadkill"))
	{
		return Material_Obtain(filename, filename);
	}

	if (!_stricmp(filename, "preview_airport"))
	{
		return Material_Obtain(filename, filename);
	}

	if (!_stricmp(filename, "preview_so_bridge"))
	{
		return Material_Obtain(filename, filename);
	}

	if (!_stricmp(filename, "preview_trainer"))
	{
		return Material_Obtain(filename, filename);
	}

	if (!_stricmp(filename, "preview_ending"))
	{
		return Material_Obtain(filename, filename);
	}

	if (!_stricmp(filename, "preview_favela"))
	{
		return Material_Obtain(filename, filename);
	}

	if (!_stricmp(filename, "preview_cliffhanger"))
	{
		return Material_Obtain(filename, filename);
	}

	if (!_stricmp(filename, "preview_dcemp"))
	{
		return Material_Obtain(filename, filename);
	}

	if (!_stricmp(filename, "preview_dcburning"))
	{
		return Material_Obtain(filename, filename);
	}

	if (!_stricmp(filename, "preview_dc_whitehouse"))
	{
		return Material_Obtain(filename, filename);
	}

	if (!_stricmp(filename, "preview_af_chase"))
	{
		return Material_Obtain(filename, filename);
	}

	if (!_stricmp(filename, "preview_af_caves"))
	{
		return Material_Obtain(filename, filename);
	}

	if (!_stricmp(filename, "preview_boneyard"))
	{
		return Material_Obtain(filename, filename);
	}

	if (!_stricmp(filename, "preview_favela_escape"))
	{
		return Material_Obtain(filename, filename);
	}

	if (!_stricmp(filename, "cardicon_t5_prestige_1"))
	{
		return Material_Obtain(filename, filename);
	}

	if (!_stricmp(filename, "nui"))
	{
		return Material_ObtainImage(filename, g_nuiDraw->nuiImage);
	}

	if (!_stricmp(filename, "hud_icon_wager"))
	{
		return Material_Obtain(filename, filename);
	}

	if (!_stricmp(filename, "peace"))
	{
		return Material_Obtain("peace", "weap_peacekeeper_c");
	}

	if (!_stricmp(filename, "$levelbriefingX"))
	{
		if (LoadScreens_Filename())
		{
			return Material_Obtain("$levelbriefingX", LoadScreens_Filename());
		}
	}

	if (!_stricmp(filename, "codfont"))
	{
		return Material_Obtain(filename, filename);
	}

	Material* avatar = UserAvatars_GetMaterial(filename);
	if (avatar)
	{
		return avatar;
	}


	return NULL;
}

StompHook materialLoadHook;
DWORD materialLoadHookLoc = 0x505E50;

void* MaterialLoadHookFunc(assetType_t type, const char* filename)
{
	Material* material = Material_Register(filename);

	if (material)
	{
		return material;
	}

	return (Material*)DB_FindXAssetHeader(ASSET_TYPE_MATERIAL, filename);
}

void Material_TableDebug_f()
{
	if (Cmd_Argc() < 2)
	{
		Com_Printf(0, "Usage: %s [material name]\n", Cmd_Argv(0));
		return;
	}

	Material* material = materialTable[Cmd_Argv(1)];
	Com_Printf(0, "%s is actually %s\n", Cmd_Argv(1), material->name);
}

cmd_function_t tDebugCmd;

void PatchMW2_Materialism()
{
	materialTable.set_empty_key("");

	materialLoadHook.initialize(materialLoadHookLoc, MaterialLoadHookFunc);
	materialLoadHook.installHook();

	Cmd_AddCommand("materialTableDebug", Material_TableDebug_f, &tDebugCmd, 0);
}

GfxImage* UserAvatars_GetImages(int num);

void ForAllCustomImages(void (*handler)(void*))
{
	for (google::dense_hash_map<std::string, Material*>::const_iterator iter = materialTable.begin(); iter != materialTable.end(); iter++)
	{
		Material* material = (*iter).second;

		for (int i = 0; i < material->numMaps; i++)
		{
			handler(material->maps[i].image);
		}
	}

	for (int i = 0; i < 18; i++)
	{
		handler(UserAvatars_GetImages(i));
	}
}

void ClearAllCustomMaterials()
{
	materialTable.clear();
}