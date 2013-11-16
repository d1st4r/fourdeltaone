// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Support for 'partial weaponry' - attachments.
//
// Initial author: NTAuthority
// Started: 2013-07-08
// ==========================================================

#include "StdInc.h"
#include "PartialWeaponry.h"

static AttachmentDef* bg_attachmentDefs[256];
static int bg_numAttachments = 0;

AttachmentDef* BG_LoadAttachmentDef_LoadObj(const char* name)
{
	char* fileBuffer;

	if (FS_ReadFile(va("weapons/attachments/%s", name), &fileBuffer) < 0)
	{
		if (!strcmp(name, "nop"))
		{
			Com_Error(0, "Could not load weapon attachment file 'nop'");
		}

		Com_Printf(0, "Could not load weapon attachment file '%s'.\n", name);
		return BG_LoadAttachmentDef_LoadObj("nop");
	}

	char* fileData = fileBuffer;

	std::list<AttachmentPatch> patches;

	Com_BeginParseSession("attachment");

	char* token = Com_ParseExt(&fileData);

	while (fileData)
	{
		if (token[0])
		{
			AttachmentPatch patch;

			if (!_stricmp(token, "multiply"))
			{
				patch.operation = APO_MULTIPLY;
			}
			else if (!_stricmp(token, "add"))
			{
				patch.operation = APO_ADD;
			}
			else if (!_stricmp(token, "set"))
			{
				patch.operation = APO_REPLACE;
			}
			else
			{
				Com_Printf(0, "Attachment error in %s: unknown operation %s\n", name, token);
				continue;
			}

			token = Com_ParseExt(&fileData);

			if (!token)
			{
				Com_Printf(0, "Attachment error in %s: premature end of file\n", name);
				break;
			}

			patch.fieldName = strdup(token);
			
			token = Com_ParseExt(&fileData);

			if (!token)
			{
				Com_Printf(0, "Attachment error in %s: premature end of file\n", name);
				break;
			}

			if ((token[0] == '-' || isdigit(token[0])) && (token[1] == '\0' || isdigit(token[1]) || token[1] == '.'))
			{
				if (strchr(token, '.'))
				{
					patch.value = atof(token);
					patch.type = APT_FLOAT;
				}
				else
				{
					patch.integer = atoi(token);
					patch.type = APT_INTEGER;
				}
			}
			else
			{
				patch.string = strdup(token);
				patch.type = APT_STRING;
			}

			patches.push_back(patch);
		}

		token = Com_ParseExt(&fileData);
	}

	Com_EndParseSession();

	FS_FreeFile(fileBuffer);

	// format the file
	AttachmentDef* adef = new AttachmentDef;
	adef->name = strdup(name);
	adef->numPatches = patches.size();

	AttachmentPatch* apatches = new AttachmentPatch[adef->numPatches];
	int n = 0;

	for (auto i = patches.begin(); i != patches.end(); i++)
	{
		apatches[n] = *i;

		n++;
	}

	adef->patches = apatches;

	return adef;
}

AttachmentDef* BG_LoadAttachmentDef(const char* name)
{
	return BG_LoadAttachmentDef_LoadObj(name);
}

int BG_FindAttachmentDef(const char* name)
{
	for (int i = 0; i < bg_numAttachments; i++)
	{
		if (!_stricmp(bg_attachmentDefs[i]->name, name))
		{
			return i;
		}
	}

	int numAtts = bg_numAttachments;
	bg_attachmentDefs[bg_numAttachments++] = BG_LoadAttachmentDef(name);

	return numAtts;
}

typedef struct weaponEntry_s
{
	const char* name;
	int offset;
	int type;
} weaponEntry_t;

#define NUM_ENTRIES 672
static weaponEntry_t* weaponEntries = (weaponEntry_t*)0x795F00;

static weaponEntry_t* FindWeaponEntry(const char* name)
{
	for (int i = 0; i < NUM_ENTRIES; i++)
	{
		if (!_stricmp(weaponEntries[i].name, name))
		{
			return &weaponEntries[i];
		}
	}

	return nullptr;
}

char* DoStringParse(const char* oldName, const AttachmentPatch* patch)
{
	// move first 2 underscore segments to oldStr
	char oldStr[128];
	strcpy(oldStr, oldName);

	char* old = oldStr;
	old = strchr(oldStr, '_');

	if (old)
	{
		old++;
		old = strchr(old, '_');

		if (old)
		{
			old[0] = '\0';
		}
	}

	// copy first and second parts to from and to respectively
	char fromStr[128];
	char toStr[128];

	memset(fromStr, 0, sizeof(fromStr));
	memset(toStr, 0, sizeof(toStr));

	char* from = fromStr;
	char* to = toStr;

	const char* str = patch->string;

	while (*str != ' ')
	{
		// <w> parts replaced with oldStr
		if (*str == '<')
		{
			str++;

			if (*str == 'w')
			{
				strcat(from, oldStr);
				from += strlen(oldStr);

				str++;
			}

			str++;
		}

		if (*str == ' ')
		{
			break;
		}

		*(from++) = *str;
		str++;
	}

	*from = '\0';
	str++;

	while (*str != '\0')
	{
		// <w> parts replaced with oldStr
		if (*str == '<')
		{
			str++;

			if (*str == 'w')
			{
				strcat(to, oldStr);
				to += strlen(oldStr);

				str++;
			}

			str++;
		}

		if (*str == '\0')
		{
			break;
		}

		*(to++) = *str;
		str++;
	}

	*to = '\0';

	// do a string replace on here
	char* newStr = str_replace((char*)oldName, fromStr, toStr);

	return newStr;
}

void BG_PatchWeaponDef(char* weapon, AttachmentDef* attachment)
{
	Com_Printf(0, "Patching weapon %s using %s\n", *(char**)weapon, attachment->name);

	for (int i = 0; i < attachment->numPatches; i++)
	{
		AttachmentPatch* patch = &attachment->patches[i];
		weaponEntry_t* entry = FindWeaponEntry(patch->fieldName);

		if (!entry)
		{
			Com_Error(1, "Unknown weapon field %s in %s.", patch->fieldName, attachment->name);
		}

		assetType_t assetType;

		switch (entry->type)
		{
			case 0: // string
				{
					char** string = (char**)(weapon + entry->offset);

					if (patch->operation == APO_ADD)
					{
						*string = strdup(va("%s%s", *string, patch->string));
					}
					else
					{
						if (strchr(patch->string, ' '))
						{
							char* oldName = *(char**)(weapon + entry->offset);

							// TODO: needs to be changed to SL?
							*string = DoStringParse(oldName, patch);
							//char* newStr = DoStringParse(oldName, patch);
														
							//free(newStr);
						}
						else
						{
							*string = (char*)patch->string; // ?!
						}
					}

					break;
				}
			case 1: // char[1024]
			case 2: // char[64]
			case 3: // char[256]
				{
					if (strchr(patch->string, ' '))
					{
						char* oldName = (char*)(weapon + entry->offset);

						// TODO: needs to be changed to SL?
						char* str = DoStringParse(oldName, patch);
						sprintf((char*)(weapon + entry->offset), "%s", str);
						free(str);
					}
					else
					{
						sprintf((char*)(weapon + entry->offset), "%s", patch->string);
					}
					break;
				}

#define DO_NUM_PATCH(variable, source) if (patch->operation == APO_ADD) \
				{ \
					variable += source; \
				} else if (patch->operation == APO_MULTIPLY) { \
					variable *= source; \
				} else { \
					variable = source; \
				}

#define GET_NUM(patch) ((patch->type == APT_FLOAT) ? patch->value : ((patch->type == APT_INTEGER) ? patch->integer : atof(patch->string)))

			case 4:
			case 5: // bool-as-int
				{
					//*(int*)(weapon + entry->offset) = patch->integer;

					int oldNum = *(int*)(weapon + entry->offset);
					DO_NUM_PATCH(oldNum, GET_NUM(patch));
					*(int*)(weapon + entry->offset) = oldNum;

					break;
				}
			case 6:
				{
					*(bool*)(weapon + entry->offset) = patch->integer;
					break;
				}

			case 7:
				{
					float oldNum = *(float*)(weapon + entry->offset);

					DO_NUM_PATCH(oldNum, GET_NUM(patch));

					*(float*)(weapon + entry->offset) = oldNum;
					break;
				}
			case 8:
				{
					float oldNum = *(float*)(weapon + entry->offset) / 17.6f;

					DO_NUM_PATCH(oldNum, GET_NUM(patch));

					*(float*)(weapon + entry->offset) = oldNum * 17.6f; // miles/hour - inch/sec
					break;
				}
			case 9:
				{
					float oldNum = *(float*)(weapon + entry->offset) / 1000.f;

					DO_NUM_PATCH(oldNum, GET_NUM(patch));

					*(float*)(weapon + entry->offset) = oldNum * 1000.f;
					break;
				}
#define WEAPON_DO_ARRAY(arr, count) \
	{ \
		for (int i = 0; i < (count); i++) { \
			if (patch->type == APT_STRING && !_stricmp(*(char**)(arr + (i * 4)), patch->string)) { \
				*(int*)(weapon + entry->offset) = i; \
			} \
		} \
	}
			case 16:
				WEAPON_DO_ARRAY(0x795DA0, 4)
				break;
			case 17:
				WEAPON_DO_ARRAY(0x795DB0, 12)
				break;
			case 18:
				WEAPON_DO_ARRAY(0x795E68, 2)
				break;
			case 19:
				WEAPON_DO_ARRAY(0x795E10, 4)
				break;
			case 20:
				WEAPON_DO_ARRAY(0x795E20, 11)
					break;
			case 21:
				WEAPON_DO_ARRAY(0x795E70, 3)
					break;
			case 22:
				WEAPON_DO_ARRAY(0x795E4C, 7)
					break;
			case 23:
				WEAPON_DO_ARRAY(0x795E7C, 6)
					break;
			case 24:
				WEAPON_DO_ARRAY(0x7BE928, *(int*)0x7BDDDC)
					break;
			case 25:
				WEAPON_DO_ARRAY(0x795E94, 3)
					break;
			case 26:
				WEAPON_DO_ARRAY(0x795EA0, 4)
					break;
			case 28:
				WEAPON_DO_ARRAY(0x795EB0, 6)
					break;
			case 29:
				WEAPON_DO_ARRAY(0x795EC8, 3)
					break;
			case 30:
				WEAPON_DO_ARRAY(0x795DE0, 6)
					break;
			case 31:
				WEAPON_DO_ARRAY(0x795DF8, 6)
					break;
			case 32:
				WEAPON_DO_ARRAY(0x795ED4, 7)
					break;
			case 33:
				WEAPON_DO_ARRAY(0x795EF0, 3)
					break;
			case 34:
				WEAPON_DO_ARRAY(0x795EF0, 3) // same as 33
					break;
			case 35:
				WEAPON_DO_ARRAY(0x795EF0, 3) // same as 33
					break;
			case 36:
				WEAPON_DO_ARRAY(0x795EF0, 3) // same as 33
					break;
			case 37:
				WEAPON_DO_ARRAY(0x795EF0, 3) // same as 33
					break;
			/*case 13: // phys collmap
				break;*/
			case 10: // fx
				assetType = ASSET_TYPE_FX;
				goto loadThingAsset;
			case 11: // xmodel
				assetType = ASSET_TYPE_XMODEL;
				goto loadThingAsset;
			case 12: // material
				assetType = ASSET_TYPE_MATERIAL;
				goto loadThingAsset;
			case 14: // soundalias
				assetType = ASSET_TYPE_SOUND;
				goto loadThingAsset;
			case 15: // tracer
				assetType = ASSET_TYPE_TRACER;

loadThingAsset:
				{
					if (strchr(patch->string, ' '))
					{
						char* oldName = **(char***)(weapon + entry->offset);

						char* newStr = DoStringParse(oldName, patch);

						// and get the asset/free the string
						*(void**)(weapon + entry->offset) = DB_FindXAssetHeader(assetType, newStr);

						free(newStr);
					}
					else
					{
						*(void**)(weapon + entry->offset) = DB_FindXAssetHeader(assetType, patch->string);
					}
					break;
				}
			/*case 38: // hideTags
				{
					// hopefully we'll have useful shit here, as if we don't, we're screwed
					// for some reason this code made me go deja vu - seems like I've done SL_ConvertToString before.
					short* tags = (short*)data;
					for (int i = 0; i < 32; i++)
					{
						short tag = tags[i];

						if (tag)
						{
							fprintf(file, "%s%s", SL_ConvertToString(tag), (i == 31 || tags[i + 1] == 0) ? "" : "\n"); // it seems like a newline is needed for all but the last tag
						}
					}
					break;
				}
			case 27: // bounceSound; surface sound
				{
					char*** dat = *(char****)data;
					if (dat != 0)
					{
						char bounceName[128];
						strcpy(bounceName, **dat);
						strrchr(bounceName, '_')[0] = '\0';

						fprintf(file, "%s", bounceName);
					}
					break;
				}*/
			default:
				Com_Error(1, "Unhandled weapon field type %i in %s\n", entry->type, entry->name);
				break;
		}
	}
}

void* WeaponFileHookFunc(const char* filename);

void* BG_LoadWeaponDef_LoadObj_hook(const char* weaponName)
{
	if (weaponName[0] == '*')
	{
		// how fun, one of our lovely attachment-y weapons! :)
		char weaponNameCopy[256];
		strcpy_s(weaponNameCopy, sizeof(weaponNameCopy), &weaponName[1]);

		char* baseNameEnd = strchr(weaponNameCopy, '+');

		if (!baseNameEnd)
		{
			return WeaponFileHookFunc(weaponName);
		}

		baseNameEnd[0] = '\0';

		char* baseWeapon = (char*)WeaponFileHookFunc(weaponNameCopy);
		char* weaponClone = (char*)malloc(2876);
		memcpy(weaponClone, baseWeapon, 2876);

		*(DWORD *)(weaponClone + 4) = (DWORD)(weaponClone + 116);
		*(DWORD *)(weaponClone + 12) = (DWORD)(weaponClone + 1784);
		*(DWORD *)(weaponClone + 16) = (DWORD)(weaponClone + 1848);
		*(DWORD *)(weaponClone + 120) = (DWORD)(weaponClone + 1996);
		*(DWORD *)(weaponClone + 128) = (DWORD)(weaponClone + 2060);
		*(DWORD *)(weaponClone + 132) = (DWORD)(weaponClone + 2208);
		*(DWORD *)(weaponClone + 140) = (DWORD)(weaponClone + 2356);
		*(DWORD *)(weaponClone + 144) = (DWORD)(weaponClone + 2388);
		*(DWORD *)(weaponClone + 148) = (DWORD)(weaponClone + 2420);
		*(DWORD *)(weaponClone + 152) = (DWORD)(weaponClone + 2452);
		*(DWORD *)(weaponClone + 588) = (DWORD)(weaponClone + 2484);
		*(DWORD *)(weaponClone + 1208) = (DWORD)(weaponClone + 2548);
		*(DWORD *)(weaponClone + 1212) = (DWORD)(weaponClone + 2672);
		*(DWORD *)(weaponClone + 1576) = (DWORD)(weaponClone + 2796);

		do
		{
			baseNameEnd++;

			char* attName = baseNameEnd;

			baseNameEnd = strchr(baseNameEnd, '+');

			if (baseNameEnd)
			{
				baseNameEnd[0] = '\0';	
			}

			int attDef = BG_FindAttachmentDef(attName);

			if (bg_attachmentDefs[attDef])
			{
				BG_PatchWeaponDef(weaponClone, bg_attachmentDefs[attDef]);
			}
		} while (baseNameEnd);

		*(const char**)weaponClone = weaponName;

		return weaponClone;
	}
	else
	{
		return WeaponFileHookFunc(weaponName);
	}
}

void WeaponDefInitHook(void* buf, int c0, int size)
{
	memset(buf, c0, size);

	memset(bg_attachmentDefs, 0, sizeof(bg_attachmentDefs));
	bg_numAttachments = 0;

	// fix for weapon bounce sound crashes (reallocation due to hunk reset)
	*(DWORD*)0x7BDDE4 = 0;

	BG_FindAttachmentDef("nop");
}

void PatchMW2_PartialWeaponry()
{
	// TODO: reset attachment defs when weapondefs gets reset

	// allow precacheItem to be called even if the game has started
	*(BYTE*)0x5F2AE7 = 0xEB;

	// same as above for G_GetWeaponIndexForName
	nop(0x49E547, 2);

	// weapon def init
	call(0x4B35EC, WeaponDefInitHook, PATCH_CALL);

	*(DWORD*)0x4081F9 = (DWORD)BG_LoadWeaponDef_LoadObj_hook;
}