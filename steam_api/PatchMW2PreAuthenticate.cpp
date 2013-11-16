// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Patches to perform NP authentication at challenge
//          time.
//
// Initial author: NTAuthority
// Started: 2012-12-26
// ==========================================================

#include "StdInc.h"
#include "Hooking.h"

//#define DISABLE_PREAUTH

typedef struct
{
	netadr_t adr;
	int challenge;
	int time1;
	int pingTime;
	int time2;
	int pad;
	int pad2;
	char guid[20];
} challengeState_t;

challengeState_t* challengeStates = (challengeState_t*)0x62B68D4;
extern int* svs_time;

typedef void (__cdecl * SV_SendSteamAuthorize_t)(netadr_t adr, NPID id);
SV_SendSteamAuthorize_t SV_SendSteamAuthorize = (SV_SendSteamAuthorize_t)0x437CE0;

void SV_SendAuthorize(const char* guidString, challengeState_t* state)
{
	strcpy_s(state->guid, sizeof(state->guid), guidString);

	NPID serverID;
	NP_GetNPID(&serverID);

	SV_SendSteamAuthorize(state->adr, serverID);
	//NET_OutOfBandPrint(NS_SERVER, state->adr, "steamauthReq");	
}

void HandleSteamAuthResult(NPAsync<NPValidateUserTicketResult>* async)
{
	NPValidateUserTicketResult* result = async->GetResult();
	challengeState_t* state = (challengeState_t*)async->GetUserData();

	NPID guid = _strtoi64(state->guid, NULL, 16);

	if (result->id != guid)
	{
		NET_OutOfBandPrint(NS_SERVER, state->adr, "error\n%s", "Auth ticket result NPID does not match challenge NPID.");

		memset(state, 0, sizeof(*state));
		return;
	}

	if (result->result == ValidateUserTicketResultOK)
	{
		state->pingTime = *svs_time;
		NET_OutOfBandPrint(NS_SERVER, state->adr, "challengeResponse %i", state->challenge);
	}
	else
	{
		NET_OutOfBandPrint(NS_SERVER, state->adr, "error\n%s", "Ticket verification failed. Redownload the client from http://fourdeltaone.net/ and try again.");

		memset(state, 0, sizeof(*state));
	}
}

typedef bool (__cdecl * NET_IsLanAddress_t)(netadr_t adr);
NET_IsLanAddress_t NET_IsLanAddress = (NET_IsLanAddress_t)0x43D6C0;

CallHook processSteamAuthHook;
DWORD processSteamAuthHookLoc = 0x6266B5;

void ProcessSteamAuthHookFunc(netadr_t adr, msg_t* msg)
{
	char ticketData[2048];
	int i;
	challengeState_t* state;

	for (i = 0; i < 1024; i++)
	{
		if (NET_CompareAdr(challengeStates[i].adr, adr))
		{
			state = &challengeStates[i];
			break;
		}
	}

	if (i == 1024)
	{
		NET_OutOfBandPrint(NS_SERVER, adr, "error\n%s", "Unknown IP for challenge.");
		return;
	}

	NPID guid = _strtoi64(state->guid, NULL, 16);
	NPID npID = MSG_ReadInt64(msg);

	if (npID != guid)
	{
		memset(state, 0, sizeof(*state));

		NET_OutOfBandPrint(NS_SERVER, adr, "error\n%s", "Auth ticket NPID does not match challenge NPID.");
		return;
	}

	short ticketLength = MSG_ReadShort(msg);

	if (ticketLength > sizeof(ticketData))
	{
		memset(state, 0, sizeof(*state));

		NET_OutOfBandPrint(NS_SERVER, adr, "error\n%s", "All I have now is dickheads! Dickheads everywhere, Tommy! I make you real rich.");
		return;
	}

	MSG_ReadData(msg, ticketData, ticketLength); // muahaha

	DWORD clientIP = (adr.ip[0] << 24) | (adr.ip[1] << 16) | (adr.ip[2] << 8) | adr.ip[3];

	if (NET_IsLanAddress(adr) || adr.type == NA_IP6)
	{
		clientIP = 0;
	}
	
#ifndef DISABLE_PREAUTH
	NPAsync<NPValidateUserTicketResult>* async = NP_ValidateUserTicket(ticketData, ticketLength, clientIP, npID);
	async->SetCallback(HandleSteamAuthResult, state);
#else
	state->pingTime = *svs_time;
	NET_OutOfBandPrint(NS_SERVER, state->adr, "challengeResponse %i", state->challenge);
#endif
}

StompHook getChallengeHook;
DWORD getChallengeHookLoc = 0x4B95B1;

void __declspec(naked) GetChallengeHookStub()
{
	__asm
	{
		push esi
		push edx

		call SV_SendAuthorize

		add esp, 8h

		pop edi
		pop esi
		pop ebp
		pop ebx
		add esp, 0Ch
		retn
	}
}

void PatchMW2_PreAuthenticate()
{
	processSteamAuthHook.initialize(processSteamAuthHookLoc, ProcessSteamAuthHookFunc);
	processSteamAuthHook.installHook();

	getChallengeHook.initialize(getChallengeHookLoc, GetChallengeHookStub);
	getChallengeHook.installHook();

	// disable Steam auth
	*(BYTE*)0x4663A8 = 0xEB;
}