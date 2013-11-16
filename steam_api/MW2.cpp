// ==========================================================
// project 'secretSchemes'
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Game-specific code implementations.
//
// Initial author: NTAuthority
// Started: 2011-05-04
// ==========================================================

#include "StdInc.h"

// function definitions
extern "C"
{
Cbuf_AddText_t Cbuf_AddText = (Cbuf_AddText_t)0x404B20;

CL_AddReliableCommand_t CL_AddReliableCommand = (CL_AddReliableCommand_t)0x454F40;
CL_IsCgameInitialized_t CL_IsCgameInitialized = (CL_IsCgameInitialized_t)0x43EB20;

Cmd_AddCommand_t Cmd_AddCommand = (Cmd_AddCommand_t)0x470090;
Cmd_AddServerCommand_t Cmd_AddServerCommand = (Cmd_AddServerCommand_t)0x4DCE00;
Cmd_ExecuteSingleCommand_t Cmd_ExecuteSingleCommand = (Cmd_ExecuteSingleCommand_t)0x609540;
Cmd_SetAutoComplete_t Cmd_SetAutoComplete = (Cmd_SetAutoComplete_t)0x40EDC0;

Com_Error_t Com_Error = (Com_Error_t)0x4B22D0;
//Com_ParseExt_t Com_ParseExt = (Com_ParseExt_t)0x4C0350;
Com_ParseExt_t Com_ParseExt = (Com_ParseExt_t)0x474D60;
Com_BeginParseSession_t Com_BeginParseSession = (Com_BeginParseSession_t)0x4AAB80;
Com_EndParseSession_t Com_EndParseSession = (Com_EndParseSession_t)0x4B80B0;
Com_Printf_t Com_Printf = (Com_Printf_t)0x402500;
Com_Milliseconds_t Com_Milliseconds = (Com_Milliseconds_t)0x42A660;
Com_PrintError_t Com_PrintError = (Com_PrintError_t)0x4F8C70;

DB_FindXAssetHeader_t DB_FindXAssetHeader = (DB_FindXAssetHeader_t)0x407930;
DB_LoadXAssets_t DB_LoadXAssets = (DB_LoadXAssets_t)0x4E5930;

Dvar_RegisterBool_t Dvar_RegisterBool = (Dvar_RegisterBool_t)0x4CE1A0;
Dvar_RegisterFloat_t Dvar_RegisterFloat = (Dvar_RegisterFloat_t)0x648440;
Dvar_RegisterFloat2_t Dvar_RegisterFloat2 = (Dvar_RegisterFloat2_t)0x4F6070;
Dvar_RegisterFloat3_t Dvar_RegisterFloat3 = (Dvar_RegisterFloat3_t)0x4EF8E0;
Dvar_RegisterFloat4_t Dvar_RegisterFloat4 = (Dvar_RegisterFloat4_t)0x4F28E0;
Dvar_RegisterInt_t Dvar_RegisterInt = (Dvar_RegisterInt_t)0x479830;
Dvar_RegisterEnum_t Dvar_RegisterEnum = (Dvar_RegisterEnum_t)0x412E40;
Dvar_RegisterString_t Dvar_RegisterString = (Dvar_RegisterString_t)0x4FC7E0;
Dvar_RegisterColor_t Dvar_RegisterColor = (Dvar_RegisterColor_t)0x471500;

Dvar_FindVar_t Dvar_FindVar = (Dvar_FindVar_t)0x4D5390;
Dvar_InfoString_Big_t Dvar_InfoString_Big = (Dvar_InfoString_Big_t)0x4D98A0;
Dvar_SetCommand_t Dvar_SetCommand = (Dvar_SetCommand_t)0x4EE430;
Dvar_SetStringByName_t Dvar_SetStringByName = (Dvar_SetStringByName_t)0x4F52E0;

FS_FreeFile_t FS_FreeFile = (FS_FreeFile_t)0x4416B0;
FS_ReadFile_t FS_ReadFile = (FS_ReadFile_t)0x4F4B90;
FS_ListFiles_t FS_ListFiles = (FS_ListFiles_t)0x441BB0;
FS_FreeFileList_t FS_FreeFileList = (FS_FreeFileList_t)0x4A5DE0;
FS_FOpenFileAppend_t FS_FOpenFileAppend = (FS_FOpenFileAppend_t)0x410BB0;
FS_FOpenFileAppend_t FS_FOpenFileWrite = (FS_FOpenFileAppend_t)0x4BA530;
FS_FOpenFileRead_t FS_FOpenFileRead = (FS_FOpenFileRead_t)0x46CBF0;
FS_FCloseFile_t FS_FCloseFile = (FS_FCloseFile_t)0x462000;
FS_WriteFile_t FS_WriteFile = (FS_WriteFile_t)0x426450;
FS_Write_t FS_Write = (FS_Write_t)0x4C06E0;
FS_Read_t FS_Read = (FS_Read_t)0x4A04C0;
FS_Seek_t FS_Seek = (FS_Seek_t)0x4A63D0;
FS_FTell_t FS_FTell = (FS_FTell_t)0x4E6760;

G_LogPrintf_t G_LogPrintf = (G_LogPrintf_t)0x4B0150;

MSG_Init_t MSG_Init = (MSG_Init_t)0x45FCA0;
MSG_ReadData_t MSG_ReadData = (MSG_ReadData_t)0x4527C0;
MSG_ReadLong_t MSG_ReadLong = (MSG_ReadLong_t)0x4C9550;
MSG_ReadShort_t MSG_ReadShort = (MSG_ReadShort_t)0x40BDD0;
MSG_ReadInt64_t MSG_ReadInt64 = (MSG_ReadInt64_t)0x4F1850;
MSG_ReadString_t MSG_ReadString = (MSG_ReadString_t)0x60E2B0;
MSG_WriteByte_t MSG_WriteByte = (MSG_WriteByte_t)0x48C520;
MSG_WriteData_t MSG_WriteData = (MSG_WriteData_t)0x4F4120;
MSG_WriteLong_t MSG_WriteLong = (MSG_WriteLong_t)0x41CA20;
MSG_ReadByte_t MSG_ReadByte = (MSG_ReadByte_t)0x4C1C20;

NET_AdrToString_t NET_AdrToString = (NET_AdrToString_t)0x469880;
NET_CompareAdr_t NET_CompareAdr = (NET_CompareAdr_t)0x4D0AA0;
NET_StringToAdr_t NET_StringToAdr = (NET_StringToAdr_t)0x409010;

SV_GameSendServerCommand_t SV_GameSendServerCommand = (SV_GameSendServerCommand_t)0x4BC3A0;

TeleportPlayer_t TeleportPlayer = (TeleportPlayer_t)0x496850;

Sys_SendPacket_t Sys_SendPacket = (Sys_SendPacket_t)0x48F500;
Sys_ListFiles_t Sys_ListFiles = (Sys_ListFiles_t)0x45A660;
Sys_FreeFileList_t Sys_FreeFileList = (Sys_FreeFileList_t)0x4D8580;

Cmd_TokenizeString_t Cmd_TokenizeString = (Cmd_TokenizeString_t)0x463E40;
Cmd_EndTokenizedString_t Cmd_EndTokenizedString = (Cmd_EndTokenizedString_t)0x4D9540;

SE_Load_t SE_Load = (SE_Load_t)0x502A30;

// other stuff
CommandCB_t Cbuf_AddServerText_f = (CommandCB_t)0x4BB9B0;
};

// swapping
BigShort_t BigShort = (BigShort_t)0x446E10;

// variables
searchpath_t* fs_searchpaths = (searchpath_t*)0x63D96E0;
int* clientState = (int*)0xB2C540;
gentity_t* g_entities = (gentity_t*)0x18835D8;
int* svs_numclients = (int*)0x31D938C;
client_t* svs_clients = (client_t*)0x31D9390;

componentExpressionDef_t* componentExpressions = (componentExpressionDef_t*)0x745FA0;

// code
const char* CL_GetServerIPAddress()
{
	static char szServerIPAddress[128];

	if (*clientState > 4)
	{
		netadr_t* adr = (netadr_t*)0xA1E888;

		sprintf_s(szServerIPAddress, sizeof(szServerIPAddress), "%i.%i.%i.%i:%i",
			adr->ip[0],
			adr->ip[1],
			adr->ip[2],
			adr->ip[3],
			BigShort(adr->port));
	}
	else
	{
		memset(szServerIPAddress, 0, sizeof(szServerIPAddress));
	}

	return szServerIPAddress;
}

// console commands
DWORD* cmd_id = (DWORD*)0x1AAC5D0;
DWORD* cmd_argc = (DWORD*)0x1AAC614;
DWORD** cmd_argv = (DWORD**)0x1AAC634;

/*
============
Cmd_Argc
============
*/
/*int		Cmd_Argc( void ) {
	return cmd_argc[*cmd_id];
}*/

/*
============
Cmd_Argv
============
*/
/*char	*Cmd_Argv( int arg ) {
	if ( (unsigned)arg >= cmd_argc[*cmd_id] ) {
		return "";
	}
	return (char*)(cmd_argv[*cmd_id][arg]);	
}*/

DWORD* cmd_id_sv = (DWORD*)0x1ACF8A0;
DWORD* cmd_argc_sv = (DWORD*)0x1ACF8E4;
DWORD** cmd_argv_sv = (DWORD**)0x1ACF904;

/*
============
Cmd_Argc
============
*/
int		Cmd_ArgcSV( void ) {
	return cmd_argc_sv[*cmd_id_sv];
}

/*
============
Cmd_Argv
============
*/
char	*Cmd_ArgvSV( int arg ) {
	if ( (unsigned)arg >= cmd_argc_sv[*cmd_id_sv] ) {
		return "";
	}
	return (char*)(cmd_argv_sv[*cmd_id_sv][arg]);	
}

void SV_GetStatus(svstatus_t* status)
{
	if (!status) return;

	int clientCount = 0;
	BYTE* clientAddress = (BYTE*)svs_clients;

	for (int i = 0; i < *svs_numclients; i++) {
		if (*clientAddress >= 3) {
			clientCount++;
		}

		clientAddress += 681872;
	}

	status->curClients = clientCount;
	status->maxClients = Party_NumPublicSlots();

	const char* mapname = GetStringConvar("mapname");
	strcpy(status->map, mapname);
}

bool SV_IsClientIP(unsigned int ip)
{
	BYTE* clientAddress = (BYTE*)svs_clients;

	for (int i = 0; i < *svs_numclients; i++) {
		if (*clientAddress >= 3) {
			netadr_t* adr = (netadr_t*)(clientAddress + 40);
			unsigned int clientIP = (adr->ip[3]) | (adr->ip[2] << 8) | (adr->ip[1] << 16) | (adr->ip[0] << 24);

			if (clientIP == ip)
			{
				return true;
			}
		}

		clientAddress += 681872;
	}

	return false;
}

typedef void (__cdecl* sendOOB_t)(int, int, int, int, int, int, const char*);
sendOOB_t OOBPrint = (sendOOB_t)0x4AEF00;

void OOBPrintT(int type, netadr_t netadr, const char* message)
{
	int* adr = (int*)&netadr;

	OOBPrint(type, *adr, *(adr + 1), *(adr + 2), 0xFFFFFFFF, *(adr + 4), message);
}

void NET_OutOfBandPrint(int type, netadr_t adr, const char* message, ...)
{
	va_list args;
	char buffer[65535];

	va_start(args, message);
	_vsnprintf(buffer, sizeof(buffer), message, args);
	va_end(args);

	OOBPrintT(type, adr, buffer);
}

typedef struct party_s
{
	BYTE pad1[544];
	int privateSlots;
	int publicSlots;
} party_t;

static party_t** partyIngame = (party_t**)0x1081C00;

int Party_NumPublicSlots(party_t* party)
{
	return party->publicSlots + party->privateSlots;
}

int Party_NumPublicSlots()
{
	return Party_NumPublicSlots(*partyIngame);
}

// get convar string
char* GetStringConvar(char* key) {
	dvar_t* var = Dvar_FindVar(key);

	if (!var) return "";

	return var->current.string;
}