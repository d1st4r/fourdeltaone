// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Vehicle definition loading.
//
// Initial author: NTAuthority
// Started: 2013-01-01
// ==========================================================

#include "StdInc.h"

#define INFOSTRING_SIZE 8192

void Com_LoadInfoString(const char* filename, const char* type, const char* identifier, char* buffer, size_t bufferSize)
{
	int handle;
	int fileLength = FS_FOpenFileRead(const_cast<char*>(filename), &handle, 0);

	if (fileLength < 0)
	{
		Com_Error(1, "Could not load %s [%s]", type, filename);
	}

	int idLength = strlen(identifier) + 1;
	FS_Read(buffer, idLength, handle);

	if (strncmp(identifier, buffer, idLength - 1) != NULL)
	{
		Com_Error(1, "File [%s] is not a %s", filename, type);
	}

	int size = (fileLength - idLength - 1);

	if (size >= bufferSize)
	{
		Com_Error(1, "File [%s] is too long of a %s to parse", filename, type);
	}

	FS_Read(buffer, size, handle);
	FS_FCloseFile(handle);

	buffer[size] = '\0';

	if (strchr(buffer, '"') || strchr(buffer, ';'))
	{
		Com_Error(1, "File [%s] is not a valid %s", filename, type);
	}
}

struct VehicleDef
{
	const char* name;
	char pad[716];
};

typedef bool (__cdecl * ParseConfigStringToStructCustomSize_t)(void* struc, void* fields, int structSize, const char* string, int maxCustomField, bool (*customFieldParser)(char* struc, const char* value, int type), void* strCpyFunc);
ParseConfigStringToStructCustomSize_t ParseConfigStringToStructCustomSize = (ParseConfigStringToStructCustomSize_t)0x48B200;

static std::unordered_map<std::string, VehicleDef*> _vehicleDefs;

bool VEH_ParseSpecificField(char* struc, const char* value, int type)
{
	switch (type)
	{
		case 16:
			{
				const char** typeNames = (const char**)0x79F2C0;

				for (int i = 0; i <= 6; i++)
				{
					if (!_stricmp(typeNames[i], value))
					{
						*(int*)(struc + 4) = i;
						return true;
					}
				}

				Com_Error(1, "Unknown vehicle type [%s]", value); // cod4 lacked the % in %s here

				break;
			}
		case 17:
		case 18:
		case 19:
			{
				int off = 0xB8;

				if (type == 18)
				{
					off = 0xBC;
				}
				else if (type == 19)
				{
					off = 0xC0;
				}

				const char** typeNames = (const char**)0x79F2DC;

				for (int i = 0; i <= 3; i++)
				{
					if (!_stricmp(typeNames[i], value))
					{
						*(int*)(struc + off) = i;
						return true;
					}
				}

				Com_Error(1, "Unknown vehicle axle [%s]", value); // cod4 lacked the % in %s here

				break;
			}
		case 20:
			break;
		case 21:
			{
				*(const char**)(struc + 0xAC) = value;
				*(void**)(struc + 0xB0) = DB_FindXAssetHeader(ASSET_TYPE_PHYSPRESET, value);

				break;
			}
		default:
			Com_Error(1, "Bad vehicle field type %i", type);
	}

	return true;
}

void* G_LoadVehicleDef(const char* filename)
{
	if (_vehicleDefs.find(filename) != _vehicleDefs.end())
	{
		return _vehicleDefs[filename];
	}

	// physpreset custom type
	*(DWORD*)0x79F4E8 = 21;

	char vehicleBuffer[INFOSTRING_SIZE];
	Com_LoadInfoString(va("vehicles/mp/%s", filename), "vehicle file", "VEHICLEFILE", vehicleBuffer, sizeof(vehicleBuffer));

	VehicleDef* vehicleDef = new VehicleDef;
	memset(vehicleDef, 0, sizeof(VehicleDef));

	vehicleDef->name = filename;

	if (ParseConfigStringToStructCustomSize(vehicleDef, (void*)0x79F2E8, 143, vehicleBuffer, 22, VEH_ParseSpecificField, (void*)0x4D7CD0))
	{
		_vehicleDefs[filename] = vehicleDef;

		return vehicleDef;
	}

	return nullptr;
}