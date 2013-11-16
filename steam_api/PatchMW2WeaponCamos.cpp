// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Weapon camouflage hacking.
//
// Initial author: NTAuthority
// Started: 2011-12-16
// ==========================================================

#include "StdInc.h"

#include <google/dense_hash_map>
#include <string>
#include <xhash>

using google::dense_hash_map;
static dense_hash_map<std::string, XModel*, stdext::hash_compare<std::string>> xModelTable;
static dense_hash_map<std::string, Material*, stdext::hash_compare<std::string>> xMaterialTable;

static std::vector<Material*> _3dMaterials;

void Material_Copy(Material* source, Material* destination)
{
	memcpy(destination, source, sizeof(Material));

	destination->maps = (MaterialTextureDef*)malloc(sizeof(MaterialTextureDef) * destination->numMaps);
	memcpy(destination->maps, source->maps, sizeof(MaterialTextureDef) * destination->numMaps);
}

void XModelCopy(XModel* source, XModel* destination)
{
	memcpy(destination, source, sizeof(XModel));

	destination->materials = (Material**)malloc(sizeof(Material*) * destination->numMaterials);
	memcpy(destination->materials, source->materials, sizeof(Material*) * destination->numMaterials);

	destination->unknowns = malloc(28 * destination->numBones);
	memcpy(destination->unknowns, source->unknowns, 28 * destination->numBones);
}

XModel* WeaponCamos_HandleGold(XModel* origModel, const char* weaponPart)
{
	std::string index(va("%s_gold", origModel->name));

	if (xModelTable[index])
	{
		return xModelTable[index];
	}

	XModel* newModel = (XModel*)malloc(sizeof(XModel));
	XModelCopy(origModel, newModel);

	char* modelName = (char*)malloc(strlen(origModel->name) + 6);
	sprintf(modelName, "%s_gold", origModel->name);
	newModel->name = modelName;

	for (int i = 0; i < newModel->numMaterials; i++)
	{
		Material* material = newModel->materials[i];
		if (strstr(material->name, weaponPart) == NULL)
		{
			continue;
		}

		char* newMaterialName = (char*)malloc(strlen(material->name) + 6);
		sprintf(newMaterialName, "%s_gold", material->name);

		if (xMaterialTable[newMaterialName])
		{
			newModel->materials[i] = xMaterialTable[newMaterialName];
			//free(newMaterialName);
			continue;
		}

		Material* newMaterial = (Material*)malloc(sizeof(Material));
		Material_Copy(material, newMaterial);
		newMaterial->name = newMaterialName;

		for (int j = 0; j < newMaterial->numMaps; j++)
		{
			if (!GAME_FLAG(GAME_FLAG_DEDICATED))
			{
				if (strstr(newMaterial->maps[j].image->name, "spc") != NULL)
				{
					newMaterial->maps[j].image = Image_Load(va("gold/%s", newMaterial->maps[j].image->name));
				}
					
				if (strstr(newMaterial->maps[j].image->name, "col") != NULL)
				{
					newMaterial->maps[j].image = Image_Load(va("gold/%s", newMaterial->maps[j].image->name));
				}
			}
		}

		_3dMaterials.push_back(newMaterial);

		newModel->materials[i] = newMaterial;
		xMaterialTable[newMaterialName] = newMaterial;
	}

	xModelTable[index] = newModel;
	xModelTable[newModel->name] = newModel;

	return newModel;
}

XModel* WeaponCamos_HandleCamo(XModel* origModel, const char* camoName)
{
	//XModel* famasWoodland = (XModel*)DB_FindXAssetHeader(ASSET_TYPE_XMODEL, "viewmodel_famas_woodland");
	std::string index(va("%s_%s", origModel->name, camoName));
	if (xModelTable[index])
	{
		return xModelTable[index];
	}

	XModel* newModel = (XModel*)malloc(sizeof(XModel));
	XModelCopy(origModel, newModel);

	newModel->name = str_replace((char*)origModel->name, "woodland", (char*)camoName);

	if (!newModel->name)
	{
		newModel->name = origModel->name;
	}

	for (int i = 0; i < newModel->numMaterials; i++)
	{
		Material* material = newModel->materials[i];
		if (strstr(material->name, "woodland") == NULL)
		{
			continue;
		}

		char* newMaterialName = str_replace((char*)material->name, "woodland", (char*)camoName);

		if (xMaterialTable[newMaterialName])
		{
			newModel->materials[i] = xMaterialTable[newMaterialName];
			free(newMaterialName);
			continue;
		}

		Material* newMaterial = (Material*)malloc(sizeof(Material));
		Material_Copy(material, newMaterial);
		newMaterial->name = newMaterialName;

		for (int j = 0; j < newMaterial->numMaps; j++)
		{
			if (strstr(newMaterial->maps[j].image->name, "woodland") == NULL)
			{
				continue;
			}

			if (!GAME_FLAG(GAME_FLAG_DEDICATED))
			{
				newMaterial->maps[j].image = Image_Load(va("weapon_camo_%s", camoName));
			}
		}

		_3dMaterials.push_back(newMaterial);

		newModel->materials[i] = newMaterial;
		xMaterialTable[newMaterialName] = newMaterial;
	}

	//xModelTable["viewmodel_fama"] = famasWinter;
	xModelTable[index] = newModel;
	xModelTable[newModel->name] = newModel;
	//xModelTable["viewmodel_famas_woodland"] = famasWinter;
	//xModelTable["weapon_famas_f1_woodland"] = famasWinter;

	return newModel;
}

// took me ages to figure this one out; CoD4 disassembly to the rescue though.
void Material_ForEach(void (*callback)(Material*))
{
	std::vector<Material*>::iterator iter;
	for (iter = _3dMaterials.begin(); iter < _3dMaterials.end(); iter++)
	{
		callback(*iter);
	}
}

typedef struct  
{
	void* unknown;
	XModel** viewModels;
	char pad[464];
	XModel** worldModels;
} weaponModels_t;

typedef struct  
{
	const char* name;
	weaponModels_t* models;
} weaponDef_t;

static const char* goldenWeapons[] = 
{
	"cheytac",
	"uzi",
	"wa2000",
	"ump45",
	"tavor",
	"aug",
	"scar",
	"spas",
	"sa80",
	"rpg",
	"rpd",
	"p90",
	"mp5",
	"1887",
	"mg4",
	"masada",
	"m240",
	"barrett",
	"m16",
	"m21",
	"m4",
	"kriss",
	"glock",
	"fn2000",
	"fal",
	"famas",
	"ak47",
	NULL
};

void WeaponCamos_HandleWeapon(weaponDef_t* weapon)
{
	//if (strstr(weapon->name, "uzi"))
	if (!weapon->models->viewModels[9] && !weapon->models->worldModels[9])
	{
		for (const char** weaponName = goldenWeapons; *weaponName != NULL; weaponName++)
		{
			if (strstr(weapon->name, *weaponName))
			{
				weapon->models->viewModels[9] = WeaponCamos_HandleGold(weapon->models->viewModels[0], *weaponName);
				weapon->models->worldModels[9] = WeaponCamos_HandleGold(weapon->models->worldModels[0], *weaponName);

				break;
			}
		}
	}

	if (weapon->models->viewModels[1])
	{
		//weapon->models->viewModels[9] = WeaponCamos_HandleCamo(weapon->models->viewModels[1], "gold");
	}

	if (weapon->models->worldModels[1])
	{
		//weapon->models->worldModels[9] = WeaponCamos_HandleCamo(weapon->models->worldModels[1], "gold");
	}
}

CallHook postWeaponLoadHook;
DWORD postWeaponLoadHookLoc = 0x408243;

void __declspec(naked) PostWeaponLoadHookStub()
{
	__asm
	{
		push eax
		call WeaponCamos_HandleWeapon
		pop eax

		jmp postWeaponLoadHook.pOriginal
	}
}

/*
CallHook xModelLoadHook;
DWORD xModelLoadHookLoc = 0x659457;

CallHook xModelLoadHook2;
DWORD xModelLoadHook2Loc = 0x49BD84;

CallHook xModelLoadHook3;
DWORD xModelLoadHook3Loc = 0x49BDA0;

void* XModelLoadHookFunc(assetType_t type, const char* filename)
{
	XModel* material = xModelTable[filename];

	OutputDebugStringA(va("%s %d\n", filename, (material) ? 1 : 0));

	if (material)
	{
		return material;
	}

	return (XModel*)DB_FindXAssetHeader(ASSET_TYPE_XMODEL, filename);
}
*/

StompHook forEachXAssetHook;
DWORD forEachXAssetHookLoc = 0x4B7720;

void __declspec(naked) ForEachXAssetHookStub()
{
	__asm
	{
		cmp [esp + 4h], 5
		jne returnToRightfulOwner

		mov eax, [esp + 8h]
		push eax
		call Material_ForEach
		add esp, 4h

returnToRightfulOwner:
		mov eax, 4B76D0h
		jmp eax
	}
}

void PatchMW2_WeaponCamos()
{
#if PRE_RELEASE_DEMO
	// before enabling: g_modelindex: overflow
	return;
#endif

	xModelTable.set_empty_key("");
	xMaterialTable.set_empty_key("");

	/*
	xModelLoadHook.initialize(xModelLoadHookLoc, XModelLoadHookFunc);
	xModelLoadHook.installHook();

	xModelLoadHook2.initialize(xModelLoadHook2Loc, XModelLoadHookFunc);
	xModelLoadHook2.installHook();

	xModelLoadHook3.initialize(xModelLoadHook3Loc, XModelLoadHookFunc);
	xModelLoadHook3.installHook();
	*/

	forEachXAssetHook.initialize(forEachXAssetHookLoc, ForEachXAssetHookStub);
	forEachXAssetHook.installHook();

	postWeaponLoadHook.initialize(postWeaponLoadHookLoc, PostWeaponLoadHookStub);
	postWeaponLoadHook.installHook();
}