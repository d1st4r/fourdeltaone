// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Modern Warfare 2 patches: dedicated servers
//
// Initial author: NTAuthority
// Started: 2010-09-27
// ==========================================================

#include "StdInc.h"
#include "hwbrk.h"
#include "AdminPlugin.h"
#include "ExtScript.h"
#include "PatchMW2Voting.h"

#include <winsock2.h>
#include <CrashRpt.h>

#pragma unmanaged

void PatchMW2_OneThread();

// annoyingly we don't have a name for this function as it's inlined in IW3's R_BeginRegistration
typedef void (*LoadInitialFF_t)(void);
LoadInitialFF_t LoadInitialFF = (LoadInitialFF_t)0x506AC0;

DWORD dedicatedInitHookLoc = 0x60BE98;
CallHook dedicatedInitHook;

struct InitialFastFiles_t {
	const char* code_post_gfx_mp;
	const char* localized_code_post_gfx_mp;
	const char* ui_mp;
	const char* localized_ui_mp;
	const char* common_mp;
	const char* localized_common_mp;
	const char* patch_mp;
};

void InitDedicatedFastFiles() {
	InitialFastFiles_t fastFiles;
	memset(&fastFiles, 0, sizeof(fastFiles));
	fastFiles.code_post_gfx_mp = "code_post_gfx_mp";
	fastFiles.localized_code_post_gfx_mp = "localized_code_post_gfx_mp";
	fastFiles.ui_mp = "ui_mp";
	fastFiles.localized_ui_mp = "localized_ui_mp";
	fastFiles.common_mp = "common_mp";
	fastFiles.localized_common_mp = "localized_common_mp";
	fastFiles.patch_mp = "patch_mp";

	memcpy((void*)0x66E1CB0, &fastFiles, sizeof(fastFiles));

	LoadInitialFF();
}

cmd_function_t sv_tell;
cmd_function_t sv_tell2;

cmd_function_t sv_say;
cmd_function_t sv_say2;

cmd_function_t sv_tellraw;
cmd_function_t sv_tellraw2;

cmd_function_t sv_sayraw;
cmd_function_t sv_sayraw2;

cmd_function_t sv_maprotate;
cmd_function_t sv_maprotate2;

cmd_function_t cv_sets;

cmd_function_t sv_exclamation;
cmd_function_t sv_exclamation2;

void SV_ConTell_f();
void SV_ConSay_f();
void SV_ConTellRaw_f();
void SV_ConSayRaw_f();
void SV_MapRotate_f();
void Dvar_SetS_f();
void SV_Exclamation_f();

dvar_t* sv_sayName;
dvar_t* iw4m_username;
dvar_t* iw4m_password;

dvar_t* sv_mapRotation;
dvar_t* sv_mapRotationCurrent;
dvar_t* iw4m_version;

dvar_t* iw4m_remoteKick;
dvar_t* iw4m_secure;

dvar_t* com_logFilter;

dvar_t* sv_online;

dvar_t* sv_master[8];

void InitDedicatedVars() {
	*(dvar_t**)0x62E2828 = Dvar_RegisterString("ui_gametype", "", DVAR_FLAG_NONE, "Current game type");
	*(dvar_t**)0x62E279C = Dvar_RegisterString("ui_mapname", "", DVAR_FLAG_NONE, "Current map name");
	sv_rconPassword = (dvar_t*)Dvar_RegisterString("rcon_password", "", DVAR_FLAG_NONE, "[IW4] the password for rcon");
	sv_sayName = (dvar_t*)Dvar_RegisterString("sv_sayName", "^7Console", DVAR_FLAG_NONE, "[IW4] the name to pose as for 'say' commands");
	sv_mapRotation = (dvar_t*)Dvar_RegisterString("sv_mapRotation", "map_restart", DVAR_FLAG_NONE, "List of maps for the server to play");
	sv_mapRotationCurrent = (dvar_t*)Dvar_RegisterString("sv_mapRotationCurrent", "", DVAR_FLAG_NONE, "Current map in the map rotation");
	iw4m_version = (dvar_t*)Dvar_RegisterString("iw4m_version", VERSION, DVAR_FLAG_READONLY, "");
	com_logFilter = (dvar_t*)Dvar_RegisterBool("com_logFilter", 1, DVAR_FLAG_LATCHED, "Removes ~95% of unneeded lines from the log");
	sv_online = (dvar_t*)Dvar_RegisterBool("sv_online", 1, DVAR_FLAG_SERVERINFO, "Controls whether the server is online/offline");

	iw4m_secure = (dvar_t*)Dvar_RegisterInt("iw4m_secure", 1, 0, 1, DVAR_FLAG_SERVERINFO, "Enable checking of 'clean' client status");
	iw4m_remoteKick = (dvar_t*)Dvar_RegisterInt("iw4m_remoteKick", 1, 0, 1, DVAR_FLAG_SERVERINFO, "Allow the master server to kick unclean clients automatically");

	sv_master[0] = Dvar_RegisterString("sv_master1", "iw4.prod.fourdeltaone.net", DVAR_FLAG_NONE, "");
	sv_master[1] = Dvar_RegisterString("sv_master2", "master.alterrev.net", DVAR_FLAG_NONE, "");
	sv_master[2] = Dvar_RegisterString("sv_master3", "server.iw4play.de", DVAR_FLAG_NONE, "");
	sv_master[3] = Dvar_RegisterString("sv_master4", "", DVAR_FLAG_NONE, "");
	sv_master[4] = Dvar_RegisterString("sv_master5", "", DVAR_FLAG_NONE, "");
	sv_master[5] = Dvar_RegisterString("sv_master6", "", DVAR_FLAG_NONE, "");

	Cmd_AddCommand("say", Cbuf_AddServerText_f, &sv_say, 0);
	Cmd_AddServerCommand("say", SV_ConSay_f, &sv_say2);

	Cmd_AddCommand("tell", Cbuf_AddServerText_f, &sv_tell, 0);
	Cmd_AddServerCommand("tell", SV_ConTell_f, &sv_tell2);

	Cmd_AddCommand("sayraw", Cbuf_AddServerText_f, &sv_sayraw, 0);
	Cmd_AddServerCommand("sayraw", SV_ConSayRaw_f, &sv_sayraw2);

	Cmd_AddCommand("tellraw", Cbuf_AddServerText_f, &sv_tellraw, 0);
	Cmd_AddServerCommand("tellraw", SV_ConTellRaw_f, &sv_tellraw2);

	Cmd_AddCommand("sets", Dvar_SetS_f, &cv_sets, 0);

	Cmd_AddCommand("!", Cbuf_AddServerText_f, &sv_exclamation, 0);
	Cmd_AddServerCommand("!", SV_Exclamation_f, &sv_exclamation2);

	Vote_Init();

	//Cmd_AddCommand("map_rotate", Cbuf_AddServerText_f, &sv_maprotate, 0);
	//Cmd_AddServerCommand("map_rotate", SV_MapRotate_f, &sv_maprotate2);
}

void InitLogFilter()
{
	if (com_logFilter && com_logFilter->current.boolean)
	{
		// both of these combined take up around 97% space
		// I had 213k lines, taking these lines out gave me 5k lines. (BIG DIFFERENCE)
		memset((void*)0x647466, 0x90, 5); // 'dvar set' lines
		memset((void*)0x5DF4F2, 0x90, 5); // 'sending splash open' lines
	}
}

#pragma optimize("", off)
void __declspec(naked) DedicatedInitHookStub() {
	InitDedicatedFastFiles();
	InitDedicatedVars();
	InitLogFilter();

	__asm {
		jmp dedicatedInitHook.pOriginal
	}
}
#pragma optimize("", on)

void G_MeSay(gentity_t* entity, const char* name, const char* message)
{
	int clientNum = (g_entities - entity);

	const char* color = "";
	
	for (int i = 0; i < *svs_numclients; i++)
	{
		if (svs_clients[i].state < 4)
		{
			continue;
		}

		gentity_t* thisEntity = &g_entities[i];

		if (thisEntity->client->team < 2)
		{
			if (thisEntity->client->team == entity->client->team)
			{
				color = "^9";
			}
			else
			{
				color = "^8";
			}
		}

		SV_GameSendServerCommand(i, 0, va("%c \"%s%s ^7%s\"", 104, color, name, message));
	}
}

bool Scriptability_OnSay(int client, char* name, char** textptr, int team)
{
	return g_scriptability->cbOnSay(client, name, textptr, team);
}

DWORD gSayPreHookLoc = 0x4D000B;
CallHook gSayPreHook;

DWORD gSayPostHook1Loc = 0x4D00D4;
CallHook gSayPostHook1;

DWORD gSayPostHook2Loc = 0x4D0110;
CallHook gSayPostHook2;

bool gsShouldSend = true;
char* gspText;
char* gspName;
void* gspEnt;
char* gspTextPtr;
int gspTeam;

#pragma optimize("", off)
void __declspec(naked) GSayPreHookFunc()
{
	__asm mov eax, [esp + 100h + 10h]
	__asm mov gspText, eax

	__asm mov eax, [esp + 4h] // as name is arg to this function, that should work too
	__asm mov gspName, eax

	__asm mov eax, [esp + 100h + 4h]
	__asm mov gspEnt, eax

	__asm mov gspTeam, ebx

	gsShouldSend = true;

	gspTextPtr = &gspText[1];

	if (!Scriptability_OnSay((gentity_t*)gspEnt - g_entities, gspName, &gspTextPtr, gspTeam))
	{
		gsShouldSend = false;
	}

	if (gsShouldSend)
	{
		if (Vote_OnSay((gentity_t*)gspEnt - g_entities, &gspText[1]))
		{
			gsShouldSend = false;
		}
	}

#if USE_MANAGED_CODE
	if (APC_TriggerSay(gspEnt, gspName, &gspText[1]))
	{
		gsShouldSend = false;
	}
	else if (gspText[1] == '/')
#else
	if (gspText[1] == '/')
#endif
	{
		gsShouldSend = false;

		if (!_strnicmp(&gspText[1], "/me ", 4) && gspText[5] != '\0')
		{
			if (GAME_FLAG(GAME_FLAG_DEDICATED))
			{
				Com_Printf(15, "* %s %s\n", gspName, &gspText[5]);
			}

			G_MeSay((gentity_t*)gspEnt, gspName, &gspText[5]);
		}

		gspText[1] = gspText[0];
		gspText += 1;
		__asm mov eax, gspText
		__asm mov [esp + 100h + 10h], eax
	}

	if (GAME_FLAG(GAME_FLAG_DEDICATED) && gsShouldSend)
	{
		Com_Printf(15, "%s: %s\n", gspName, &gspText[1]);
	}

	__asm jmp gSayPreHook.pOriginal
}

// these two need to pushad/popad as otherwise some registers the function uses as param are screwed up
void __declspec(naked) GSayPostHook1Func()
{
	__asm pushad

	if (!gsShouldSend)
	{
		__asm popad
		__asm retn
	}

	__asm popad

	__asm jmp gSayPostHook1.pOriginal
}

void __declspec(naked) GSayPostHook2Func()
{
	__asm pushad

	if (!gsShouldSend)
	{
		__asm popad
		__asm retn
	}

	__asm popad

	__asm jmp gSayPostHook2.pOriginal
}
#pragma optimize("", on)

StompHook onClientDisconnectHook;
DWORD onClientDisconnectHookLoc = 0x4D1600;
DWORD onClientDisconnectHookRet = 0x4D1605;

void OnClientDisconnectHookFunc(client_t* client)
{
	Vote_OnDisconnect(client - svs_clients);
}

#pragma optimize("", off)
void __declspec(naked) OnClientDisconnectHookStub()
{
	__asm
	{
		mov eax, [esp + 4h]
		push eax
		call OnClientDisconnectHookFunc
		add esp, 4h

		sub esp, 10h
		push ebx
		push ebp
		jmp onClientDisconnectHookRet
	}
}
#pragma optimize("", on)

CallHook gRunFrameHook;
DWORD gRunFrameHookLoc = 0x62726D;

void ReproCase_ClientDrop();
void Legacy_Advertise();

void G_RunFrameHookFunc()
{
	//ReproCase_ClientDrop();
	Legacy_Advertise();
	Vote_OnFrame();
#if USE_MANAGED_CODE
	APC_TriggerFrame();
#endif
}

#pragma optimize("", off)
void __declspec(naked) G_RunFrameHookStub()
{
	G_RunFrameHookFunc();

	__asm
	{
		jmp gRunFrameHook.pOriginal
	}
}
#pragma optimize("", on)

StompHook doPostInitHook1;
CallHook steamCheckHook;
DWORD doPostInitHook1Loc = 0x60BFBF;
DWORD cliCommandHook1Loc = 0x4D9661;
DWORD cliCommandHook2Loc = 0x4D9684;
DWORD steamCheckHookLoc = 0x4663AA;

void HTTPService_Init();

void PushAutoCommands()
{
	Ext_Initialize();
	HTTPService_Init();

	Cmd_ExecuteSingleCommand(0, 0, "exec autoexec.cfg");
	Cmd_ExecuteSingleCommand(0, 0, "onlinegame 1");
	Cmd_ExecuteSingleCommand(0, 0, "exec default_xboxlive.cfg");
	Cmd_ExecuteSingleCommand(0, 0, "xblive_rankedmatch 0");
	Cmd_ExecuteSingleCommand(0, 0, "xblive_privatematch 1");
	Cmd_ExecuteSingleCommand(0, 0, "xstartprivatematch");
	Cmd_ExecuteSingleCommand(0, 0, "sv_network_fps 1000");
	Cmd_ExecuteSingleCommand(0, 0, "com_maxfps 125");
}

DWORD processCLI = 0x60C3D0;
DWORD doPostInit = 0x43D140;

void __declspec(naked) DoPostInitStub()
{
	__asm
	{
		call PushAutoCommands
		call processCLI
		jmp doPostInit
	}
}

void __declspec(naked) CLICommandHook2Stub()
{
	__asm
	{
		call PushAutoCommands
		jmp processCLI
	}
}

static bool validated = false;
static bool isValid = false;

void DoSteamCheckWrap()
{
	//DoSteamCheck();
}

void __declspec(naked) DoSteamCheckStub()
{
	__asm push ecx
	DoSteamCheckWrap();
	__asm pop ecx
	__asm jmp steamCheckHook.pOriginal
	//__asm retn
}

CallHook heartbeatHookFrame;
DWORD heartbeatHookFrameLoc = 0x6276A1;

StompHook svheartbeatHook;
DWORD svheartbeatHookLoc = 0x4E6640;

CallHook killServerHook;
DWORD killServerHookLoc = 0x480708;

CallHook stopServerHook;
DWORD stopServerHookLoc = 0x4F5C35;

void SV_Heartbeat_f();

void __declspec(naked) Hook_SV_Heartbeat_f()
{
	__asm jmp SV_Heartbeat_f
}

void SV_MasterHeartbeat();

void __declspec(naked) HeartbeatHookFrameStub()
{
	SV_MasterHeartbeat();
	__asm jmp heartbeatHookFrame.pOriginal
}

void SV_MasterHeartbeat_OnKill();

void __declspec(naked) KillServerHookStub()
{
	SV_MasterHeartbeat_OnKill();
	__asm jmp killServerHook.pOriginal
}

void __declspec(naked) StopServerHookStub()
{
	SV_MasterHeartbeat_OnKill();
	__asm jmp stopServerHook.pOriginal
}

CallHook checkPrivateSlotHook;
DWORD checkPrivateSlotHookLoc = 0x5B717B;

int CheckPrivateSlotHookFunc(void* unknown, __int64 guid)
{
	int retval = -1;
	FILE* slots = fopen("reservedslots.txt", "r");

	if (!slots)
	{
		return retval;
	}

	while (!feof(slots))
	{
		unsigned int testGuid;
		if (fscanf(slots, "%lx\n", &testGuid) > 0)
		{
			if (testGuid == (guid & 0xFFFFFFFF))
			{
				retval = 1;
				break;
			}
		}
		else
		{
			break;
		}
	}

	fclose(slots);
	return retval;
}

void __declspec(naked) CheckPrivateSlotHookStub()
{
	__asm jmp CheckPrivateSlotHookFunc
}

#define NEW_MAXCLIENTS 24

void PatchMW2_SayCommands()
{
	gSayPreHook.initialize(gSayPreHookLoc, GSayPreHookFunc);
	gSayPreHook.installHook();

	gSayPostHook1.initialize(gSayPostHook1Loc, GSayPostHook1Func);
	gSayPostHook1.installHook();

	gSayPostHook2.initialize(gSayPostHook2Loc, GSayPostHook2Func);
	gSayPostHook2.installHook();
}

void PatchMW2_DediSlowmotion();

void PatchMW2_Dedicated()
{
	//steam sometimes doesn't like the server
	*(BYTE*)0x683370 = 0xC3;

	*(BYTE*)0x5B4FF0 = 0xC3; // self-registration on party
	*(BYTE*)0x426130 = 0xC3; // other party stuff?

	//*(BYTE*)0x412300 = 0xC3; // upnp devices
	*(BYTE*)0x4D7030 = 0xC3; // upnp stuff

	*(BYTE*)0x4B0FC3 = 0x04; // make CL_Frame do client packets, even for game state 9
	*(BYTE*)0x4F5090 = 0xC3; // init sound system (1)
	*(BYTE*)0x507B80 = 0xC3; // start render thread
	*(BYTE*)0x4F84C0 = 0xC3; // R_Init caller
	*(BYTE*)0x46A630 = 0xC3; // init sound system (2)
	*(BYTE*)0x41FDE0 = 0xC3; // Com_Frame audio processor?
	*(BYTE*)0x41B9F0 = 0xC3; // called from Com_Frame, seems to do renderer stuff
	*(BYTE*)0x41D010 = 0xC3; // CL_CheckForResend, which tries to connect to the local server constantly
	*(BYTE*)0x62B6C0 = 0xC3; // UI expression 'DebugPrint', mainly to prevent some console spam
	*(BYTE*)0x6258D0 = 0xC3; // handles the 'userinfo' reliable command, should make it so that only connect's name/rate/shit get used

	*(BYTE*)0x468960 = 0xC3; // some mixer-related function called on shutdown

	*(BYTE*)0x60AD90 = 0;    // masterServerName flags

	*(WORD*)0x4DCEC9 = 0x9090; // some check preventing proper game functioning

	memset((void*)0x507C79, 0x90, 6); // another similar bsp check

	memset((void*)0x414E4D, 0x90, 6); // unknown check in SV_ExecuteClientMessage (0x20F0890 == 0, related to client->f_40)

	memset((void*)0x4E532D, 0x90, 5); // function checking party heartbeat timeouts, causes random issues

	memset((void*)0x4DCEE9, 0x90, 5); // some deinit renderer function

	memset((void*)0x59A896, 0x90, 5); // warning message on a removed subsystem
	memset((void*)0x4B4EEF, 0x90, 5); // same as above

	memset((void*)0x64CF77, 0x90, 5); // function detecting video card, causes Direct3DCreate9 to be called

	memset((void*)0x60BC52, 0x90, 0x15); // recommended settings check

	// isHost script call return 0
	*(DWORD*)0x5DEC04 = 0;

	// map_rotate func
	*(DWORD*)0x4152E8 = (DWORD)SV_MapRotate_f;

	// sv_network_fps max 1000, and uncheat
	*(BYTE*)0x4D3C67 = 0; // ?
	*(DWORD*)0x4D3C69 = 1000;

	// AnonymousAddRequest
	*(BYTE*)0x5B5E18 = 0xEB;
	*(WORD*)0x5B5E5C = 0x9090;
	*(BYTE*)0x5B5E64 = 0xEB;

	// HandleClientHandshake
	*(BYTE*)0x5B6EA5 = 0xEB;
	*(WORD*)0x5B6EEB = 0x9090;
	*(BYTE*)0x5B6EF3 = 0xEB;

	*(BYTE*)0x519DDF = 0; // r_loadForRenderer default to 0
	
	// disable cheat protection on onlinegame
	*(BYTE*)0x404CF7 = 0x80; 

	*(BYTE*)0x508470 = 0xC3; // some d3d9 call on error

	// stop saving a config_mp.cfg
	*(BYTE*)0x60B240 = 0xC3;

	// load fastfiles independtly from renderer
	dedicatedInitHook.initialize(dedicatedInitHookLoc, DedicatedInitHookStub);
	dedicatedInitHook.installHook();

	// slash command stuff
	gRunFrameHook.initialize(gRunFrameHookLoc, G_RunFrameHookStub);
	gRunFrameHook.installHook();

	doPostInitHook1.initialize(doPostInitHook1Loc, DoPostInitStub);
	doPostInitHook1.installHook();

	//steamCheckHook.initialize(steamCheckHookLoc, DoSteamCheckStub);
	//steamCheckHook.installHook();

	heartbeatHookFrame.initialize(heartbeatHookFrameLoc, HeartbeatHookFrameStub);
	heartbeatHookFrame.installHook();

	svheartbeatHook.initialize(svheartbeatHookLoc, Hook_SV_Heartbeat_f);
	svheartbeatHook.installHook();

	killServerHook.initialize(killServerHookLoc, KillServerHookStub);
	killServerHook.installHook();

	stopServerHook.initialize(stopServerHookLoc, StopServerHookStub);
	stopServerHook.installHook();

	checkPrivateSlotHook.initialize(checkPrivateSlotHookLoc, CheckPrivateSlotHookStub);
	checkPrivateSlotHook.installHook();

	//cliCommandHook2.initialize((PBYTE)cliCommandHook2Loc);
	//cliCommandHook2.installHook(CLICommandHook2Stub, false);
	memset((void*)cliCommandHook1Loc, 0x90, 5);
	memset((void*)cliCommandHook2Loc, 0x90, 5);

	// cause Steam auth to always be requested (duplicate of above!)
	//memset((void*)0x46FB8C, 0x90, 6);

	// do not reset sv_privateClients on starting a private match
	memset((void*)0x44AF2F, 0x90, 5);

	// don't whine about missing soundaliases, causes main thread lag in onethread due to WndProc
	memset((void*)0x644207, 0x90, 5);

	// attempt at single-threaded server, might work better for dedicated
	PatchMW2_OneThread();

	// stuff
	PatchMW2_DediSlowmotion();
}

// DEDICATED SAY CODE
int* maxclients = (int*)0x1A8354C;

void G_SayTo(int mode, void* targetEntity, void* sourceEntity, int unknown, DWORD color, const char* name, const char* text) {
	DWORD func = 0x5DF620;

	__asm {
		push text
		push name
		push color
		push unknown
		push sourceEntity
		mov ecx, targetEntity
		mov eax, mode
		call func
		add esp, 14h
	}
}

void G_SayToClient(int client, DWORD color, const char* name, const char* text) {
	gentity_t* sourceEntity = &g_entities[0];
	int unknown = 55;
	int mode = 0;

	gentity_t* other = &g_entities[client];

	G_SayTo(mode, other, sourceEntity, unknown, color, name, text);
}

void G_SayToAll(DWORD color, const char* name, const char* text) {
	gentity_t* sourceEntity = &g_entities[0];
	int unknown = 55;
	int mode = 0;

	for (int i = 0; i < *maxclients; i++) {
		gentity_t* other = &g_entities[i];

		G_SayTo(mode, other, sourceEntity, unknown, color, name, text);
	}
}

void Cmd_GetStringSV(int start, char* buffer, unsigned int length)
{
	*buffer = 0;

	for (int i = start; i < Cmd_ArgcSV(); i++) {
		if (strlen(Cmd_ArgvSV(i)) > 64)
		{
			break;
		}

		if (strlen(buffer) > (length - 64))
		{
			break;
		}

		strncat( buffer, Cmd_ArgvSV(i), length );

		if (i != (Cmd_ArgcSV() - 1))
		{
			strcat(buffer, " ");
		}
	}	
}

void SV_ConSay_f()
{
	if (Cmd_ArgcSV() < 2)
	{
		return;
	}

	char message[1024];
	Cmd_GetStringSV(1, message, sizeof(message));

	if (sv_sayName->current.string[0])
	{
		SV_GameSendServerCommand(-1, 0, va("%c \"%s: %s\"", 104, sv_sayName->current.string, message));
		Com_Printf(15, "%s: %s\n", sv_sayName->current.string, message);
	}
	else
	{
		SV_GameSendServerCommand(-1, 0, va("%c \"Console: %s\"", 104, message));
		Com_Printf(15, "Console: %s\n", message);
	}
}

void SV_ConSayRaw_f()
{
	if (Cmd_ArgcSV() < 2)
	{
		return;
	}

	char message[1024];
	Cmd_GetStringSV(1, message, sizeof(message));
	SV_GameSendServerCommand(-1, 0, va("%c \"%s\"", 104, message));
	Com_Printf(15, "Raw: %s\n", message);
}
void SV_ConTell_f()
{
	if (Cmd_ArgcSV() < 3)
	{
		return;
	}

	int client = atoi(Cmd_ArgvSV(1));
	char message[1024];
	Cmd_GetStringSV(2, message, sizeof(message));

	if (sv_sayName->current.string[0])
	{
		SV_GameSendServerCommand(client, 0, va("%c \"%s: %s\"", 104, sv_sayName->current.string, message));
		Com_Printf(15, "%s -> %i: %s\n", sv_sayName->current.string, client, message);
	}
	else
	{
		SV_GameSendServerCommand(client, 0, va("%c \"Console: %s\"", 104, message));
		Com_Printf(15, "Console -> %i: %s\n", client, message);
	}
}

void SV_ConTellRaw_f()
{
	if (Cmd_ArgcSV() < 3)
	{
		return;
	}

	int client = atoi(Cmd_ArgvSV(1));
	char message[1024];
	Cmd_GetStringSV(2, message, sizeof(message));
	SV_GameSendServerCommand(client, 0, va("%c \"%s\"", 104, sv_sayName->current.string, message));
	Com_Printf(15, "Raw -> %i: %s\n", client, message);
}
// END DEDI SAY

// DEDI MAP ROTATION
typedef void (__cdecl * SetConsoleString_t)(dvar_t* cvar, char* value);
SetConsoleString_t SetConsoleString = (SetConsoleString_t)0x4A9580;

typedef void (__cdecl * SetConsole_t)(const char* cvar, char* value);
SetConsole_t SetConsole = (SetConsole_t)0x44F060;

char cbuf[512];

char* GetStringConvar(char* key);

void SV_ExecuteLastMap()
{
	char* mapname = GetStringConvar("mapname");

	if (!strlen(mapname))
	{
		mapname = "mp_afghan";
	}

	Cmd_ExecuteSingleCommand(0, 0, va("map %s", mapname));
}

extern dvar_t* playlist_enabled;

void Party_ChooseNextPlaylistEntry(int controller)
{
	__asm
	{
		push controller
		mov eax, 04573FFh // past the xblive_privatematch check
		call eax
		add esp, 4h
	}
}

void SV_MapRotateUsingPlaylists()
{
	Party_ChooseNextPlaylistEntry(0);
	
	dvar_t* ui_gametype = Dvar_FindVar("ui_gametype");
	dvar_t* ui_mapname = Dvar_FindVar("ui_mapname");

	SetConsole("g_gametype", ui_gametype->current.string);
	Cmd_ExecuteSingleCommand(0, 0, va("map %s", ui_mapname->current.string));
}

void SV_MapRotate_f()
{
	Com_Printf(0, "map_rotate...\n\n");

	if (playlist_enabled->current.boolean)
	{
		Com_Printf(0, "rotating using playlists as playlist_enabled = 1\n");
		//SV_MapRotateUsingPlaylists();
		g_scriptability->cbRotateMap();
		return;
	}

	Com_Printf(0, "\"sv_mapRotation\" is: \"%s\"\n\n", sv_mapRotation->current.string);
	Com_Printf(0, "\"sv_mapRotationCurrent\" is: \"%s\"\n\n", sv_mapRotationCurrent->current.string);

	// if nothing, just restart
	if (strlen(sv_mapRotation->current.string) == 0)
	{
		//Cmd_ExecuteSingleCommand(0, 0, "map mp_afghan\n");
		SV_ExecuteLastMap();
		return;
	}

	// first, check if the string contains nothing
	if (!sv_mapRotationCurrent->current.string[0])
	{
		Com_Printf(0, "setting new current rotation...\n");

		SetConsoleString(sv_mapRotationCurrent, sv_mapRotation->current.string);
	}

	// now check the 'current' string for tokens -- ending or 'map' means quits
	char rotation[8192];
	memset(rotation, 0, sizeof(rotation));
	strncpy(rotation, sv_mapRotationCurrent->current.string, sizeof(rotation));

	char* token = strtok(rotation, " ");
	bool isType = true;
	char* type = "";
	char* value = "";
	bool changedMap = false;

	while (token)
	{
		if (isType)
		{
			type = token;
		}
		else
		{
			value = token;

			if (!strcmp(type, "gametype"))
			{
				Com_Printf(0, "new gametype: %s\n", value);
				SetConsole("g_gametype", value);
			}
			else if (!strcmp(type, "fs_game"))
			{
				Com_Printf(0, "new fs_game: %s\n", value);

				if (!strcmp(value, "\"\"") || !strcmp(value, "''"))
				{
					SetConsole("fs_game", "");
				}
				else
				{
					SetConsole("fs_game", value);
				}
			}
			else if (!strcmp(type, "map"))
			{
				Com_Printf(0, "new map: %s\n", value);
				_snprintf(cbuf, sizeof(cbuf), "map %s", value);
				Cmd_ExecuteSingleCommand(0, 0, cbuf);

				//if ((*sv_running)->current.integer)
				if (!strcmp(GetStringConvar("mapname"), value))
				{
					changedMap = true;
				}

				break;
			}
		}

		token = strtok(NULL, " ");
		isType = !isType;
	}

	token += (strlen(token) + 1);

	SetConsoleString(sv_mapRotationCurrent, token);

	if (!changedMap)
	{
		Com_Printf(0, "no map was found, restarting current map");
		//Cbuf_AddText(0, "map mp_afghan\n");
		SV_ExecuteLastMap();
	}
}
// END DEDI MAP ROTATION

// SETS COMMAND
typedef void (__cdecl * Dvar_Set_t)(void);
Dvar_Set_t Dvar_Set_f = (Dvar_Set_t)0x44F790;

void Dvar_SetS_f()
{
	dvar_t	*v;

	if ( Cmd_Argc() != 3 ) {
		Com_Printf (0, "USAGE: sets <variable> <value>\n");
		return;
	}

	if (!strcmp(Cmd_Argv(1), "_mayCrash"))
	{
		//DWORD oldProtect;
		//VirtualProtect((void*)0x1B272A4, 4, PAGE_READONLY, &oldProtect);
		//VirtualProtect((void*)0x631F70C, 4, PAGE_READONLY, &oldProtect);
		*(BYTE*)0x0 = 0;
	}

	Dvar_Set_f();
	v = Dvar_FindVar( Cmd_Argv( 1 ) );
	if ( !v ) {
		return;
	}
	v->flags |= DVAR_FLAG_SERVERINFO;
}
// END SETS COMMAND
extern bool validInt(char* str);
void SV_Exclamation_f()
{
	char* usage = "USAGE: ! <client # / all>\n";
	if ( Cmd_Argc() != 2 ) {
		Com_Printf (0, usage);
		return;
	}
	int client = 0;
	if(!strcmp(Cmd_Argv(1), "all")){
		client = -1;
	} else {
		if(!validInt(Cmd_Argv(1))){
			Com_Printf (0, "Invalid input.\n");
			return;
		}
		client = atoi(Cmd_Argv(1));
		if(client >= *(int*)0x31D938C){
			Com_Printf (0, "Invalid player.\n");
			return;
		}
	}
	SV_GameSendServerCommand(client, 0, va("%c \"\"", 106));
}

// MASTER SERVER
#define MAX_MASTER_SERVERS 4

int* svs_time = (int*)0x31D9384;
int svs_nextHeartbeatTime;
int svs_nextGUIDbeatTime;

extern dvar_t** masterServerName;

/*
================
SV_MasterHeartbeat

Send a message to the masters every few minutes to
let it know we are alive, and log information.
We will also have a heartbeat sent when a server
changes from empty to non-empty, and full to non-full,
but not on every player enter or exit.
================
*/
#define	HEARTBEAT_MSEC	120*1000
#define	HEARTBEAT_GAME	"IW4"

#define	GUIDBEAT_MSEC	30*1000

void SetWindowTitle();
void UserAvatars_SendGUIDsToAllClients();

#define PORT_MASTER 20810

void SV_MasterHeartbeat() {
	static netadr_t	adr[16];

	if ( *svs_time >= svs_nextGUIDbeatTime )
	{
		svs_nextGUIDbeatTime = *svs_time + GUIDBEAT_MSEC;
		UserAvatars_SendGUIDsToAllClients();
	}


	// if not time yet, don't send anything
	if ( *svs_time < svs_nextHeartbeatTime ) {
		return;
	}

	if ( !sv_online || !sv_online->current.boolean ) {
		return;
	}

	SetWindowTitle();

	svs_nextHeartbeatTime = *svs_time + HEARTBEAT_MSEC;

	for (int i = 0; i < 6; i++)
	{
		const char* masterName = sv_master[i]->current.string;

		// send to group masters
		if ( !masterName[0] ) {
			return;
		}

		// see if we haven't already resolved the name
		// resolving usually causes hitches on win95, so only
		// do it when needed
		if ( adr[i].type != NA_IP ) {
			Com_Printf( 0, "Resolving %s\n", masterName );
			if ( !NET_StringToAdr( masterName, &adr[i] ) ) {
				// if the address failed to resolve, clear it
				// so we don't take repeated dns hits
				Com_Printf( 0, "Couldn't resolve address: %s\n", masterName );
				//SetConsole( (*masterServerName)->name, "" );
				return;
			}
			if ( !strstr( ":", masterName ) ) {
				adr[i].port = htons(PORT_MASTER);
			}
			Com_Printf( 0, "%s resolved to %i.%i.%i.%i:%i\n", masterName,
				adr[i].ip[0], adr[i].ip[1], adr[i].ip[2], adr[i].ip[3],
				htons(adr[i].port) );
		}

		// this command should be changed if the server info / status format
		// ever incompatibly changes
		NET_OutOfBandPrint( NS_SERVER, adr[i], "heartbeat %s\n", HEARTBEAT_GAME );

		Com_Printf (0, "Sent heartbeat to %s\n", masterName );
	}
}

void SV_Heartbeat_f()
{
	svs_nextHeartbeatTime = -9999;
	svs_nextGUIDbeatTime = -9999;
}

void SV_MasterHeartbeat_OnKill()
{
	// send twice due to possibility of dropping
	svs_nextHeartbeatTime = -9999;
	SV_MasterHeartbeat();

	svs_nextHeartbeatTime = -9999;
	SV_MasterHeartbeat();
}
// END MASTER SERVER

// SINGLE-THREADED SERVER
StompHook frameEpilogueHook;
DWORD frameEpilogueHookLoc = 0x6272E3;

CallHook frameTriggerHook;
DWORD frameTriggerHookLoc = 0x627695;

StompHook packetEventHook;
DWORD packetEventHookLoc = 0x43D1C7;

#pragma optimize("", off)
void __declspec(naked) FrameEpilogueFunc()
{
	__asm
	{
		pop edi
		pop esi
		pop ebx
		mov esp, ebp
		pop ebp
		retn
	}
}

void __declspec(naked) PacketEventHookFunc()
{
	__asm
	{
		mov eax, 049F0B0h
		call eax
		mov eax, 0458160h
		jmp eax
	}
}
#pragma optimize("", on)

void PatchMW2_OneThread()
{
	// remove starting of server thread from Com_Init_Try_Block_Function
	memset((void*)0x60BEC0, 0x90, 5);

	// make server thread function jump to per-frame stuff
	*(BYTE*)0x627049 = 0xE9;
	*(DWORD*)0x62704A = (0x6271CE - 0x62704E);

	// make SV_WaitServer insta-return
	*(BYTE*)0x4256F0 = 0xC3;

	// dvar setting function, unknown stuff related to server thread sync
	*(BYTE*)0x647781 = 0xEB;

	frameEpilogueHook.initialize(frameEpilogueHookLoc, FrameEpilogueFunc);
	frameEpilogueHook.installHook();

	frameTriggerHook.initialize(frameTriggerHookLoc, (void*)0x627040);
	frameTriggerHook.installHook();

	packetEventHook.initialize(packetEventHookLoc, PacketEventHookFunc);
	packetEventHook.installHook();
}