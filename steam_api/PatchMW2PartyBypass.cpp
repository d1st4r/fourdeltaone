// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: A simple replacement for the party-based server
//          handshake.
//
// Initial author: NTAuthority
// Started: 2013-04-02
// ==========================================================

#include "StdInc.h"

static netadr_t currentLobbyTarget;
static uint32_t joinChallenge;
static uint32_t joinStarted;

typedef void (__cdecl * SetConsole_t)(const char* cvar, char* value);
extern SetConsole_t SetConsole;

typedef void (__cdecl * CL_ConnectFromParty_t)(int controller, void*, netadr_t adr, int, int, const char*, const char*);
CL_ConnectFromParty_t CL_ConnectFromParty = (CL_ConnectFromParty_t)0x433D30;

typedef void (__cdecl* Menus_CloseAll_t)(DWORD);
Menus_CloseAll_t Menus_CloseAll = (Menus_CloseAll_t)0x4BA5B0;

void CL_JoinResponse(netadr_t from, const char* infostring)
{
	if (!joinChallenge)
	{
		return;
	}

	const char* challenge = Info_ValueForKey(infostring, "challenge");

	if (!challenge)
	{
		Com_Printf(0, "Invalid join response received: no challenge\n");
		return;
	}

	int challengeNum = atoi(&challenge[5]);

	if (challengeNum != joinChallenge)
	{
		Com_Printf(0, "Invalid join response received: challenge mismatch\n");
		return;
	}

	const char* numClients = Info_ValueForKey(infostring, "clients");
	const char* maxClients = Info_ValueForKey(infostring, "sv_maxclients");

	if (!numClients || !maxClients)
	{
		return;
	}

	Cmd_ExecuteSingleCommand(0, 0, "closemenu popup_reconnectingtoparty");
	joinStarted = 0;

	if (atoi(numClients) >= atoi(maxClients))
	{
		SetConsole("partyend_reason", "@EXE_SERVERISFULL");
		Cmd_ExecuteSingleCommand(0, 0, "openmenu menu_xboxlive_partyended");
		return;
	}

	// well, let's try connecting
	Dvar_SetCommand("xblive_privatematch", "1");

	Menus_CloseAll(0x62E2858);

	char xnaddr[32];
	CL_ConnectFromParty(0, xnaddr, from, 0, 0, Info_ValueForKey(infostring, "mapname"), Info_ValueForKey(infostring, "gametype"));
}

void CL_ConnectFrame()
{
	if (!joinStarted)
	{
		return;
	}

	if ((Com_Milliseconds() - joinStarted) > 5000) // should be ample time
	{
		joinStarted = 0;
		joinChallenge = 0;

		Cmd_ExecuteSingleCommand(0, 0, "closemenu popup_reconnectingtoparty");
		SetConsole("partyend_reason", "Server connection timed out.");
		Cmd_ExecuteSingleCommand(0, 0, "openmenu menu_xboxlive_partyended");
	}
}

static void ConnectToAddress(netadr_t adr)
{
	Cmd_ExecuteSingleCommand(0, 0, "openmenu popup_reconnectingtoparty");

	joinStarted = Com_Milliseconds();
	joinChallenge = Com_Milliseconds();

	NET_OutOfBandPrint(NS_CLIENT, adr, "getinfo _join%i", joinChallenge);
}

typedef bool (__cdecl * NET_StringToAdr_t)(const char* string, netadr_t* adr);
extern NET_StringToAdr_t NET_StringToAdr;

void Legacy_SetProtocol(int protocol);

void CL_Connect_f()
{
	if (Cmd_Argc() < 2)
	{
		return;
	}

	const char* str = Cmd_Argv(1);
	netadr_t adr;

	int protocol = PROTOCOL;

	if (Cmd_Argc() == 3)
	{
		protocol = atoi(Cmd_Argv(2));
	}

	Legacy_SetProtocol(protocol);

	if (NET_StringToAdr(str, &adr))
	{
		ConnectToAddress(adr);
	}
}

void CL_ConnectNoParty_f()
{
	if (Cmd_Argc() < 4)
	{
		Com_Printf(0, "usage: connectNoParty [ip] [mapname] [gametype]\n");
		return;
	}

	const char* str = Cmd_Argv(1);
	netadr_t adr;
	char xnaddr[32];

	if (NET_StringToAdr(str, &adr))
	{
		Dvar_SetCommand("xblive_privatematch", "1");

		CL_ConnectFromParty(0, &xnaddr, adr, 0, 0, Cmd_Argv(2), Cmd_Argv(3));
	}
}

void CL_Reconnect_f()
{
	Cmd_ExecuteSingleCommand(0, 0, "disconnect");

	netadr_t adr = currentLobbyTarget;
	ConnectToAddress(adr);
}

void PatchMW2_PartyBypass()
{
	static cmd_function_t connectCmd;
	static cmd_function_t connectNoPartyCmd;
	static cmd_function_t reconnectCmd;

	Cmd_AddCommand("connect", CL_Connect_f, &connectCmd, 0);
	Cmd_AddCommand("connectNoParty", CL_Connect_f, &connectNoPartyCmd, 0);
	Cmd_AddCommand("reconnect", CL_Reconnect_f, &reconnectCmd, 0);

	// various changes to SV_DirectConnect-y stuff to allow non-party joinees
	*(WORD*)0x460D96 = 0x90E9;
	*(BYTE*)0x460F0A = 0xEB;
	*(BYTE*)0x401CA4 = 0xEB;
	*(BYTE*)0x401C15 = 0xEB;
}