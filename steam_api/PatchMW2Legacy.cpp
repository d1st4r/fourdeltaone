// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: compatibility with legacy versions of IW4M
//
// Initial author: NTAuthority
// Started: 2011-10-15
// ==========================================================

#include "StdInc.h"

StompHook getProtocolHook;
DWORD getProtocolHookLoc = 0x4FB500;

bool Legacy_IsClientLegacy(int clientNum)
{
	int protocol = atoi(Info_ValueForKey(svs_clients[clientNum].connectInfoString, "protocol"));

	return (protocol == 144);
}

static int protocolOverride;
static bool legacyMode;

bool Legacy_IsLegacyMode()
{
	if (!protocolOverride)
	{
		return false;
	}

	if (protocolOverride == PROTOCOL)
	{
		return false;
	}

	return true;
}

int Legacy_GetProtocol()
{
	if (protocolOverride)
	{
		return protocolOverride;
	}

	return PROTOCOL;
}

void Legacy_ResetToNonLegacy();

void XStartPrivateMatch_f()
{
	Legacy_ResetToNonLegacy();

	__asm
	{
		mov eax, 44AEA0h
		call eax
	}
}

CallHook entityWriteFieldsHook;
DWORD entityWriteFieldsHookLoc = 0x6111AC;

void EntityWriteFieldsHookStub();

CallHook sendScoreboardLineHook;
DWORD sendScoreboardLineHookLoc = 0x45969A;

static gentity_t* ent;

void SendScoreboardLineHookFunc(char* buf, int size, const char* format, ...)
{
	const char* newFormat = format;

	if (Legacy_IsClientLegacy(ent - g_entities))
	{
		newFormat = " %i %i %i %i %i %i %i 0";
	}

	va_list ap;
	va_start(ap, format);
	_vsnprintf(buf, size, newFormat, ap);
	va_end(ap);
}

void __declspec(naked) SendScoreboardLineHookStub()
{
	__asm
	{
		mov eax, [esp + 0B50h]
		mov ent, eax
		jmp SendScoreboardLineHookFunc
	}
}

CallHook loadConfigStringHook;
DWORD loadConfigStringHookLoc = 0x4A75BC;

void RemoveConfigStringCache()
{
	*(void**)0x63D0B9C = DB_FindXAssetHeader(ASSET_TYPE_STRINGTABLE, "mp/defaultStringTable.csv");
}

void __declspec(naked) LoadConfigStringHookStub()
{
	__asm
	{
		/*mov eax, [esp + 8h]
		push eax
		mov eax, [esp + 8h]
		push eax
		call loadConfigStringHook.pOriginal
		add esp, 8h*/

		call RemoveConfigStringCache

		retn
	}
}

StompHook cgObituaryIconHook;
DWORD cgObituaryIconHookLoc = 0x586C9A;
DWORD cgObituaryIconHookRet = 0x586CA0;

void __declspec(naked) CG_ObituaryIconHookStub()
{
	__asm
	{
		cmp ecx, 578h // 1400
		jl returnStuff

		sub ecx, 0C8h // 200

returnStuff:
		mov ebx, [eax + 80h]
		jmp cgObituaryIconHookRet
	}
}

CallHook bgGetWeaponDefCgItemHook;
DWORD bgGetWeaponDefCgItemHookLoc = 0x5860AC;

void __declspec(naked) CG_ItemWeaponHookStub()
{
	__asm
	{
		mov eax, [esp + 4]
		push eax
		call bgGetWeaponDefCgItemHook.pOriginal
		add esp, 4h

		test eax, eax
		jnz returnStuff

		push 0
		call bgGetWeaponDefCgItemHook.pOriginal
		add esp, 4h

returnStuff:
		retn
	}
}

void PatchMW2_Legacy()
{
	getProtocolHook.initialize(getProtocolHookLoc, Legacy_GetProtocol);
	getProtocolHook.installHook();

	entityWriteFieldsHook.initialize(entityWriteFieldsHookLoc, EntityWriteFieldsHookStub);
	entityWriteFieldsHook.installHook();

	sendScoreboardLineHook.initialize(sendScoreboardLineHookLoc, SendScoreboardLineHookStub);
	sendScoreboardLineHook.installHook();

	loadConfigStringHook.initialize(loadConfigStringHookLoc, LoadConfigStringHookStub);
	loadConfigStringHook.installHook();

	cgObituaryIconHook.initialize(cgObituaryIconHookLoc, CG_ObituaryIconHookStub);

	bgGetWeaponDefCgItemHook.initialize(bgGetWeaponDefCgItemHookLoc, CG_ItemWeaponHookStub);

	// no xmodel loaded for item index ... error
	//memset((void*)0x5860DC, 0x90, 5);

	// xstartprivatematch pre-hook
	*(DWORD*)0x40554E = (DWORD)XStartPrivateMatch_f;

	// allow connections from any party netcode
	*(BYTE*)0x5B669E = 0xEB;

	// allow connections from any protocol
	*(BYTE*)0x460582 = 0xEB;

	// scoreboard length; 'skills' var got added post-159
	//*(BYTE*)0x592293 = 8;
	//*(BYTE*)0x592296 = (8 * 4);

	// SendScoreboard formatting string; last field is new
	//*(const char**)0x459689 = " %i %i %i %i %i %i %i 0";
}

void Legacy_ResetToNonLegacy()
{
	if (!GAME_FLAG(GAME_FLAG_DEDICATED))
	{
		*(DWORD*)0x47440B = (DWORD)"mp/defaultStringTable.csv";
	}

	*(BYTE*)0x5AC2C3 = 0xEB;

	*(BYTE*)0x592293 = 7;
	*(BYTE*)0x59228D = (7 * 4);

	*(DWORD*)0x58609F = 1200;

	cgObituaryIconHook.releaseHook();
	bgGetWeaponDefCgItemHook.releaseHook();
}

void Legacy_SetProtocol(int protocol)
{
	protocolOverride = protocol;

	if (protocol != PROTOCOL)
	{
		// playlist version
		*(DWORD*)0x6354688 = 504;

		// configstring table
		*(DWORD*)0x47440B = (DWORD)"mp/configStrings/configStrings_pc_%s_%s.csv";

		// configstring reading
		*(BYTE*)0x5AC2C3 = 0x75;

		// scoreboard command length due to skills var
		*(BYTE*)0x592293 = 8;
		*(BYTE*)0x59228D = (8 * 4);

		// weapon index multiplier stuff
		*(DWORD*)0x58609F = 1400;

		cgObituaryIconHook.installHook();
		bgGetWeaponDefCgItemHook.installHook();
	}
	else
	{
		Legacy_ResetToNonLegacy();
	}
}

// - EV_OBITUARY special weapon compatibility
static int* a1;

void __declspec(naked) EntityWriteFields(int* snapshotInfo, int a3, entityState_t* oldState, entityState_t* newState, int a6, int a7, int a8, int a9)
{
	__asm
	{
		mov eax, a1
		jmp entityWriteFieldsHook.pOriginal
	}
}

void EntityWriteFieldsHookFunc(int* snapshotInfo, int a3, entityState_t* oldState, entityState_t* newState, int a6, int a7, int a8, int a9)
{
	int clientNum = *snapshotInfo;
	int entityNum = newState->entityNum;

	entityState_t oldStateBackup;
	entityState_t newStateBackup;

	memcpy(&oldStateBackup, oldState, sizeof(oldStateBackup));
	memcpy(&newStateBackup, newState, sizeof(newStateBackup));

	if (newState->eType == (18 + 98)) // EV_OBITUARY
	{
		if (Legacy_IsClientLegacy(clientNum))
		{
			int weaponIndex = (newState->eventParm | (newState->un1 << 8));

			if (weaponIndex >= 1200) // max weapons in 159
			{
				weaponIndex += 200; // max weapons after weapon patches
			}

			newState->eventParm = weaponIndex & 0xFF;
			newState->un1 = weaponIndex >> 8;
		}
	}
	else if (newState->eType == 3) // ET_ITEM
	{
		if (Legacy_IsClientLegacy(clientNum))
		{
			return;
			/*int weaponIndex = newState->itemIndex % 1200;
			int weaponModel = newState->itemIndex / 1200;

			newState->itemIndex = weaponIndex;*/
		}
	}

	EntityWriteFields(snapshotInfo, a3, oldState, newState, a6, a7, a8, a9);

	memcpy(oldState, &oldStateBackup, sizeof(oldStateBackup));
	memcpy(newState, &newStateBackup, sizeof(newStateBackup));
}

void __declspec(naked) EntityWriteFieldsHookStub()
{
	__asm
	{
		mov a1, eax
		jmp EntityWriteFieldsHookFunc
	}
}

static int lastAd;

void Legacy_Advertise()
{
	if ((Com_Milliseconds() - lastAd) > 120000)
	{
		for (int i = 0; i < *svs_numclients; i++)
		{
			client_t* client = &svs_clients[i];

			if (client->state < 3)
			{
				continue;
			}

			if (!Legacy_IsClientLegacy(i))
			{
				continue;
			}

			SV_GameSendServerCommand(i, 0, va("%c \"This server is running ^24D1 IW4M^7 - the next generation MW2 mod.\"", 104));
			SV_GameSendServerCommand(i, 0, va("%c \"Get the client at http://fourdeltaone.net/\"", 104));
		}

		lastAd = Com_Milliseconds();
	}
}