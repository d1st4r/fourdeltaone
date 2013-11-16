// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Increase maxclient limit.
//
// Initial author: zxz0O0
// Started: 2013-03-18
//
// Thanks to: NTAuthority, Nukem, master131
// ==========================================================
 
#include "StdInc.h"
 
void PatchMW2_MaxclientsCommon()
{
	//ui_maxclients & sv_maxclients
	*(BYTE*)0x426189 = MAX_USABLE_CLIENTS;
	*(BYTE*)0x42618D = MAX_USABLE_CLIENTS;
	*(BYTE*)0x4D3750 = MAX_USABLE_CLIENTS;
	*(BYTE*)0x4D3754 = MAX_USABLE_CLIENTS;
	*(BYTE*)0x5E376C = MAX_USABLE_CLIENTS;
	*(BYTE*)0x5E3770 = MAX_USABLE_CLIENTS;
	//sv_privateClientsForClients
	*(BYTE*)0x4D3712 = MAX_CLIENTS;
	//sv_privateClients
	*(BYTE*)0x4D3731 = MAX_CLIENTS;
	//party_maxplayers
	*(BYTE*)0x4D5D5E = MAX_CLIENTS;
	//party_minplayers
	*(BYTE*)0x4D5D47 = MAX_CLIENTS;
 
	//iw4m.exe+FC5C6 - 8D 70 12              - lea esi,[eax+12]
	//starting entityNum for nonplayers
	*(BYTE*)0x4FC5C8 = MAX_CLIENTS;
	//iw4m.exe+E8564 - 8D 70 EE              - lea esi,[eax-12]
	*(BYTE*)0x4E8566 = -MAX_CLIENTS;
 
	//showToClient check
	*(BYTE*)0x5F6E52 = MAX_CLIENTS;
 
	//some entity num check
	*(BYTE*)0x628562 = MAX_CLIENTS;
 
	//iw4m.exe+195295 - 83 EE 12              - sub esi,12
	//esi is entityNum of the corpse entity, so subtract MAX_CLIENTS to get the index of clientInfo_t corpseinfo[]
	*(BYTE*)0x595297 = MAX_CLIENTS;
	//iw4m_srv.exe+D76F4 - 83 E8 12              - sub eax,12
	//same as above
	*(BYTE*)0x4D76F6 = MAX_CLIENTS;
 
	//CG_Obituary: target out of range --> checking vicitim's clientNum
	*(BYTE*)0x586D9B = MAX_CLIENTS;
	//this one checking attacker's clientNum
	*(BYTE*)0x586E07 = MAX_CLIENTS - 1;
 
	//check if sv_maxclients (?) is > limit
	*(BYTE*)0x591F61 = MAX_CLIENTS;
	//if yes, replace by this
	*(int*)0x591F6F = MAX_CLIENTS;
 
	//parameter int numGEntities for SV_LocateGameData
	*(BYTE*)0x48EF4D = MAX_CLIENTS + MAX_CORPSES;
	//iw4m_server.exe+8EF53 - C7 05 B031A801 1A000000 - mov [iw4m_server2.exe+16831B0],0000001A
	*(int*)0x48EF59 = MAX_CLIENTS + MAX_CORPSES;
 
	//some check in G_FreeEntity
	*(BYTE*)0x44CB09 = MAX_CLIENTS + MAX_CORPSES;
 
	//Limit for loop in G_FreeEntityRefs
	*(int*)0x4163A6 = gentity_s_start + 0x158 + MAX_CLIENTS * sizeof(gentity_t);
	*(int*)0x41635D = gentity_s_start + 0x158 + MAX_CLIENTS * sizeof(gentity_t);
 
	//+0x94 may be some entity limit for usuable items
	//iw4m_server.exe+3837 - B8 12000000           - mov eax,00000012
	//iw4m_server.exe+383F - 89 86 94000000        - mov [esi+00000094],eax
	*(int*)0x403838 = MAX_CLIENTS;
	//iw4m_server.exe+1BC6C - C7 86 94000000 12000000 - mov [esi+00000094],00000012
	*(int*)0x41BC72 = MAX_CLIENTS;
	//iw4m_server.exe+74E13 - C7 86 94000000 12000000 - mov [esi+00000094],00000012
	*(int*)0x474E19 = MAX_CLIENTS;
	//iw4m.exe+DD026 - 8B 85 94000000        - mov eax,[ebp+00000094]
	//iw4m.exe+DD034 - 83 F8 12              - cmp eax,12
	*(BYTE*)0x4DD036 = MAX_CLIENTS;
 
 
	//Limit for loop, probably some gsc function
	*(int*)0x5F9923 = MAX_CLIENTS * sizeof(gclient_t);
 
	//Various checks which I did not document
	//iw4m_srv.exe+188041 - 83 F8 12              - cmp eax,12
	*(BYTE*)0x588043 = MAX_CLIENTS;
	//iw4m_srv.exe+422D5 - 83 FF 12              - cmp edi,12
	*(BYTE*)0x4422D7 = MAX_CLIENTS;
	//iw4m_srv.exe+42249 - 83 FF 12              - cmp edi,12
	*(BYTE*)0x44224B = MAX_CLIENTS;
	//iw4m_srv.exe+1713FE - 83 F9 12              - cmp ecx,12
	*(BYTE*)0x571400 = MAX_CLIENTS;
	//iw4m.exe+17FC31 - 83 F9 12              - cmp ecx,12
	*(BYTE*)0x57FC33 = MAX_CLIENTS;
	//iw4m.exe+4232B - 83 FF 12              - cmp edi,12
	*(BYTE*)0x44232D = MAX_CLIENTS;
	//iw4m_srv.exe+E1CE6 - 83 FD 11              - cmp ebp,11
	//CG_ProcessClientNoteTracks
	*(BYTE*)0x4E1CE8 = MAX_CLIENTS - 1;
 
	//"client not pointing to the level.clients array"
	*(BYTE*)0x5D847E = MAX_CLIENTS;
	//"forceSpectatorClient can only be set to -1 or a valid client number"
	*(BYTE*)0x5D8744 = MAX_CLIENTS;
 
	//unknown checks
	//iw4m.exe+EFC40 - 83 FF 12              - cmp edi,12
	*(BYTE*)0x4EFC42 = MAX_CLIENTS;
	//iw4m_srv.exe+25238 - BE 12000000           - mov esi,00000012
	*(int*)0x425239 = MAX_CLIENTS;
	//iw4m_srv.exe+25295 - 83 FF 12              - cmp edi,12
	*(BYTE*)0x425297 = MAX_CLIENTS;
	//iw4m.exe+1EB631 - 83 F8 12              - cmp eax,12
	*(BYTE*)0x5EB633 = MAX_CLIENTS;
}
 
void PatchMW2_MaxclientsServer();
void PatchMW2_MaxclientsClient();
 
void PatchMW2_Maxclients()
{
	PatchMW2_MaxclientsCommon();
 
	if (GAME_FLAG(GAME_FLAG_DEDICATED))
		PatchMW2_MaxclientsServer();
	else
		PatchMW2_MaxclientsClient();
 
	DWORD clientStart = *(DWORD*)0x412AE4; // address of one user patched by a function above
	svs_clients = (client_t*)clientStart;
}
