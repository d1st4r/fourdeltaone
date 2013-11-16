// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Patches for compatibility/important changes in
//          early game versions (IW4M 159, for instance)
//
// Initial author: NTAuthority
// Started: 2011-05-20
// ==========================================================

#include "StdInc.h"

// prefix - IWNet ping-based searching
static int currentMaxPing = 0;
static int pingSearches = 0;
static int failedSearches = 0;

int GetCurrentMaxPing()
{
	return currentMaxPing;
}

// UI_SafeTranslateString
typedef char* (__cdecl * UI_SafeTranslateString_t)(const char* name);
UI_SafeTranslateString_t UI_SafeTranslateString = (UI_SafeTranslateString_t)0x4F1700;

// UI_ReplaceConversions_Int
typedef char* (__cdecl * UI_ReplaceConversions_Int_t)(const char* string, int number);
UI_ReplaceConversions_Int_t UI_ReplaceConversions_Int = (UI_ReplaceConversions_Int_t)0x501090;

// local variables for PingDisplayHookFunc
typedef struct pdh_value_s
{
	int type;
	char* string;
} pdh_value_t;

static pdh_value_t* pdh_value;
static int curPing;
static char* searchString;

StompHook pingDisplayHook;
DWORD pingDisplayHookLoc = 0x62EE01;

void __declspec(naked) PingDisplayHookFunc()
{
	__asm mov pdh_value, esi

	if ( GetCurrentMaxPing() && GetCurrentMaxPing() <= 150 )
	{
		curPing = GetCurrentMaxPing();
		searchString = UI_SafeTranslateString("PATCH_SEARCHINGFORGAMES_NMS");
		pdh_value->type = 2;
		pdh_value->string = UI_ReplaceConversions_Int(searchString, curPing);
	}
	else
	{
		pdh_value->type = 2;
		pdh_value->string = UI_SafeTranslateString("MENU_SEARCHING_FOR_LOBBY");
	}

	__asm pop edi
	__asm retn
}

// values for hpong result
DWORD* gameState = (DWORD*)0xB2C540;

// random fact that used to be here: I called va() 'Com_Sprintf' back then. oh, how wrong I was. :D

typedef int (__cdecl * GetNumPartyPlayers_t)(void* party);
GetNumPartyPlayers_t GetNumPartyPlayers = (GetNumPartyPlayers_t)0x497330;

typedef int (__cdecl * GetMaxPartyPlayers_t)(void* party);
GetMaxPartyPlayers_t GetMaxPartyPlayers = (GetMaxPartyPlayers_t)0x4F5D60;

// local variables for hpong result
static int curPlayers;
static int maxPlayers;
static int** currentParty;

const char* __cdecl FormatHpongResult(const char* string, int num, char* v1, char* v2)
{
	//__asm mov eax, [esp + 40h] // 34 + 4 (call return) + 4 (arg 1) + some other 4
	__asm mov currentParty, esi

	curPlayers = GetNumPartyPlayers(*currentParty);
	maxPlayers = GetMaxPartyPlayers(*currentParty);

	return va("%ihpong %s %s %d %d %d", num, v1, v2, (*gameState > 0), curPlayers, maxPlayers);
}

CallHook hpongResultHook;
DWORD hpongResultHookLoc = 0x5B44D7;

void __declspec(naked) HpongResultHookFunc()
{
	__asm jmp FormatHpongResult
}

dvar_t* ping_default_min;
dvar_t* ping_increment;
dvar_t* ping_searches_per;

void RetryPingSearch()
{
	if (!currentMaxPing)
	{
		currentMaxPing = ping_default_min->current.integer;
		pingSearches = -1;
	}

	pingSearches++;
	failedSearches++;

	Com_Printf(25, "Min Ping %d Search %d Failed Search Count %d\n", currentMaxPing, pingSearches, failedSearches);

	if (pingSearches > ping_searches_per->current.integer)
	{
		currentMaxPing += ping_increment->current.integer;
		pingSearches = 0;
	}
}

CallHook retryGameSearchHook;
DWORD retryGameSearchHookLoc = 0x4352FF;

void __declspec(naked) RetryGameSearchHookFunc()
{
	RetryPingSearch();
	__asm jmp retryGameSearchHook.pOriginal
}

CallHook qosHook;
DWORD qosHookLoc = 0x4C024E;

int qhPingMS;
int qhTime;
int qhBias = 0;

void __declspec(naked) QosHookFunc() {
	__asm mov qhPingMS, ebx
	__asm mov qhTime, ebp

	qhPingMS += qhBias;

	if (qhPingMS < 1)
	{
		qhPingMS = 1;
	}

	if (qhPingMS > GetCurrentMaxPing())
	{
		// this method is a bit lazy, but don't want to translate it to proper C
		// it sets the party slot to inactive
		__asm
		{
			mov eax, [edi + 0F0h]
			mov byte ptr [esi + eax], 0
		}
	}

	__asm mov ebx, qhPingMS
	__asm mov ebp, qhTime

	// and for security, we put ebx to the printf parameter
	__asm mov [esp + 0Ch], ebx // 4 + 8 (arg3)

	__asm jmp qosHook.pOriginal
}

void PatchMW2_PatchPing()
{
	pingDisplayHook.initialize(pingDisplayHookLoc, PingDisplayHookFunc);
	pingDisplayHook.installHook();

	hpongResultHook.initialize(hpongResultHookLoc, HpongResultHookFunc);
	hpongResultHook.installHook();

	retryGameSearchHook.initialize(retryGameSearchHookLoc, RetryGameSearchHookFunc);
	retryGameSearchHook.installHook();

	qosHook.initialize(qosHookLoc, QosHookFunc);
	qosHook.installHook();

	ping_searches_per = Dvar_RegisterInt("ping_searches_per", 2, 1, 30, DVAR_FLAG_USERINFO | DVAR_FLAG_SAVED, "Number of searches at each min ping value");
	ping_increment = Dvar_RegisterInt("ping_increment", 10, 1, 500, DVAR_FLAG_NONE | DVAR_FLAG_SAVED, "Number of milliseconds to increase min ping after each set of searches");
	ping_default_min = Dvar_RegisterInt("ping_default_min", 50, 10, 500, DVAR_FLAG_NONE | DVAR_FLAG_SAVED, "Minimum ping for the initial set of searches");
}

// weapon pool things
#if NON_FUNCTIONING_WEAPON_REALLOCATION
StompHook cgSetupWeaponDefHook;
DWORD cgSetupWeaponDefHookLoc = 0x4BD526;
DWORD cgSetupWeaponDefHookRet = 0x4BD52C;

void __declspec(naked) CG_SetupWeaponDefHookStub()
{
	__asm
	{
		cmp edi, 4B0h
		jb valueBelow

		lea eax, [edi + 0B7Bh]
		jmp returnHere

valueBelow:
		lea eax, [edi + 0AF4h]

returnHere:
		jmp cgSetupWeaponDefHookRet
	}
}

StompHook saveRegisteredWeaponsBreakHook;
DWORD saveRegisteredWeaponsBreakHookLoc = 0x45D190;
DWORD saveRegisteredWeaponsBreakHookRet = 0x45D196;

StompHook saveRegisteredWeaponsContinueHook;
DWORD saveRegisteredWeaponsContinueHookLoc = 0x45D1B2;

void __declspec(naked) SaveRegisteredWeaponsBreakHookStub()
{
	__asm
	{
		cmp esi, 4B0h
		jnb returnOddly

returnAsUsual:
		push esi
		mov eax, 04E6EC0h
		call eax
		jmp saveRegisteredWeaponsBreakHookRet

returnOddly:
		jmp saveRegisteredWeaponsContinueHookLoc
	}
}

void __declspec(naked) SaveRegisteredWeaponsContinueHookStub()
{
	__asm
	{
		mov esi, 4B0h
		mov eax, 4F5CC0h
		call eax
		cmp eax, esi
		jbe usualReturn

doLoop:
		push esi
		mov eax, 4E6EC0h
		call eax
		push eax
		lea ecx, [esi+0B7Bh]
		push ecx
		mov eax, 4982E0h
		call eax
		add esp, 0Ch
		add esi, 1
		mov eax, 4F5CC0h
		call eax
		cmp esi, eax
		jb doLoop

usualReturn:
		pop esi
		retn
	}
}

#define MAX_CONFIG_STRINGS 4139
typedef struct cfgState_s
{
	int configStrings[MAX_CONFIG_STRINGS];
	char buffer[131072];
	int unknown;
} cfgState_t;

static cfgState_t configState;
static short clientConfigStrings[MAX_CONFIG_STRINGS]; // SL_ strings

void ResizeConfigStrings()
{
	for (unsigned char* i = (unsigned char*)CODE_START; (int)i < CODE_END; i++)
	{
		if (i[0] == 0x81 && *(int*)(i + 2) == 4139)
		{
			//*(int*)(i + 2) = MAX_CONFIG_STRINGS;
		}

		if (i[0] == 0x68 && *(int*)(i + 1) == 0x40AC)
		{
			*(int*)(i + 1) = (MAX_CONFIG_STRINGS * 4);
		}

		if (i[0] == 0x68 && *(int*)(i + 1) == 0x240B0)
		{
			*(int*)(i + 1) = sizeof(configState);
		}

		if (*(int*)i == 0xAC077C)
		{
			*(int*)i = (int)&configState;
		}

		if (*(int*)i == 0xAC4828)
		{
			*(int*)i = (int)(&configState.buffer);
		}

		if (*(int*)i == 0xAE4828)
		{
			*(int*)i = (int)(&configState.unknown);
		}

		if (*(int*)i == 0x208A632)
		{
			*(int*)i = (int)clientConfigStrings;
		}

		if (*(int*)i == 0x208C670)
		{
			*(int*)i = ((int)clientConfigStrings) + 0x203E;
		}

		if (*(int*)i == 0x208C672)
		{
			*(int*)i = ((int)clientConfigStrings) + 0x2040;
		}

		if (*(int*)i == 0x208C674)
		{
			*(int*)i = ((int)clientConfigStrings) + 0x2042;
		}

		if (*(int*)i == 0x208C688)
		{
			*(int*)i = ((int)clientConfigStrings) + sizeof(clientConfigStrings);
		}
	}
}

void PatchMW2_WeaponPool()
{
	// configstring patches for weapons >= 1200
	/*cgSetupWeaponDefHook.initialize(6, (PBYTE)cgSetupWeaponDefHookLoc);
	cgSetupWeaponDefHook.installHook(CG_SetupWeaponDefHookStub, true, false);

	saveRegisteredWeaponsBreakHook.initialize(6, (PBYTE)saveRegisteredWeaponsBreakHookLoc);
	saveRegisteredWeaponsBreakHook.installHook(SaveRegisteredWeaponsBreakHookStub, true, false);

	saveRegisteredWeaponsContinueHook.initialize(5, (PBYTE)saveRegisteredWeaponsContinueHookLoc);
	saveRegisteredWeaponsContinueHook.installHook(SaveRegisteredWeaponsContinueHookStub, true, false);

	// a few random numbers in code that seem to depend on weapon asset pool size
	*(DWORD*)0x403783 = 1400;
	*(DWORD*)0x403E8C = 1400;
	*(DWORD*)0x41BC34 = 1400;
	*(DWORD*)0x42EB42 = 1400;
	*(DWORD*)0x44FA7B = 1400;
	*(DWORD*)0x474E0D = 1400;
	*(DWORD*)0x48E8F2 = 1400;
	*(DWORD*)0x492647 = 1400;
	*(DWORD*)0x494585 = 1400;
	*(DWORD*)0x4945DB = 1400;
	*(DWORD*)0x4B1F96 = 1400;
	*(DWORD*)0x4D4A99 = 1400;
	*(DWORD*)0x4DD566 = 1400;
	*(DWORD*)0x4E3683 = 1400;
	*(DWORD*)0x58609F = 1400;
	*(DWORD*)0x586CAE = 1400;
	*(DWORD*)0x58F7BE = 1400;
	*(DWORD*)0x58F7D9 = 1400;
	*(DWORD*)0x58F82D = 1400;
	*(DWORD*)0x5D6C8B = 1400;
	*(DWORD*)0x5D6CF7 = 1400;
	*(DWORD*)0x5E24D5 = 1400;
	*(DWORD*)0x5E2604 = 1400;
	*(DWORD*)0x5E2828 = 1400;
	*(DWORD*)0x5E2B4F = 1400;
	*(DWORD*)0x5F2614 = 1400;
	*(DWORD*)0x5F7187 = 1400;
	*(DWORD*)0x5FECF9 = 1400;
	*(DWORD*)0x5FECF9 = 1400;*/

	// reallocate configstrings
	ResizeConfigStrings();
}
#endif

// javelin glitch stuff, seems to reset a flag on weapnext
StompHook javelinResetHook; // don't exactly know what this is, but it's changed between 169 and 172
DWORD javelinResetHookLoc = 0x578F52;

void __declspec(naked) JavelinResetHookStub()
{
	__asm
	{
		mov eax, 577A10h
		call eax
		pop edi
		mov dword ptr [esi+34h], 0
		pop esi
		pop ebx
		retn
	}
}

void PatchMW2_JavelinGlitch()
{
	javelinResetHook.initialize(javelinResetHookLoc, JavelinResetHookStub);
	javelinResetHook.installHook();
}

void PatchMW2_Prefix()
{
	//PatchMW2_PatchPing();
	PatchMW2_JavelinGlitch();
	//PatchMW2_WeaponPool();
}