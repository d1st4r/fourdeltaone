// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Increase maxclient limit - client part.
//
// Initial author: zxz0O0
// Started: 2013-03-18
// ==========================================================
 
#include "StdInc.h"
 
//For raising max corpse count
/*clientInfo_t corpseInfo [MAX_CORPSES];
void Patch_clientInfo_t_corpse()
{
	int clientInfo_t_corpse_patches [2] = { 0x7EE5F4, 0x7EEAF0 };
	DWORD allocated = (DWORD)corpseInfo;
 
	SearchAndPatch(clientInfo_t_corpse_patches, 2, clientInfo_t_corpse_start, allocated);
	//iw4m_server.exe+189E4D - 81 FE 50147F00        - cmp esi,iw4m_server.exe+3F1450
	*(int*)0x589E4F = sizeof(clientInfo_t) * MAX_CORPSES + allocated + 0x4FC;	
	//iw4m_server.exe+189EA9 - 81 FE 50147F00        - cmp esi,iw4m_server.exe+3F1450
	*(int*)0x589EAB = sizeof(clientInfo_t) * MAX_CORPSES + allocated + 0x4FC;	
}*/
 
clientState_s cS_clients[MAX_CLIENTS * 0x20];//0x20 *should* be correct
//just (2*(MAXCLIENTS * sizeof(clientState_s)) + 0x33134) is actually needed for the following but this looks cleaner
snapshot_s activeSnapshots[2];
void Patch_clientState_s()
{
	int patches[2] = { clientState_s_start, 0x51B15C };
	SearchAndPatch(patches, 2, clientState_s_start, (DWORD)cS_clients);
	//iw4m.exe+195082 - 81 3D E8427F00 F0427F00 - cmp [iw4m.exe+3F42E8],iw4m.exe+3F42F0
	*(int*)0x595088 = (int)activeSnapshots;
	//iw4m.exe+195097 - 69 C9 EC390300        - imul ecx,ecx,000339EC
	*(int*)0x595099 = sizeof(snapshot_s);
	//iw4m.exe+19509D - 81 C1 F0427F00        - add ecx,iw4m.exe+3F42F0
	*(int*)0x59509F = (int)activeSnapshots;
	//iw4m.exe+502C2 - 8B 8F E8390300        - mov ecx,[edi+000339E8]
	*(int*)0x4502C4 = (int)&activeSnapshots[0].serverCommandSequence - (int)activeSnapshots; //get size until serverCommandSequence
	//iw4m.exe+6B855 - 89 86 E8390300        - mov [esi+000339E8],eax
	*(int*)0x46B857 = (int)&activeSnapshots[0].serverCommandSequence - (int)activeSnapshots;
 
 
	//iw4m.exe+6B980 - 83 F8 12              - cmp eax,12
	*(BYTE*)0x46B982 = MAX_CLIENTS;
	//iw4m.exe+6B989 - C7 44 24 1C 12000000  - mov [esp+1C],00000012
	*(int*)0x46B98D = MAX_CLIENTS;
}
 
clientInfo_t c_clientinfo[MAX_CLIENTS];
int c_clientInfo_t_patches [31] = { 0x8E77B0, 0x8E77BC, 0x8E77CC, 0x8E77D4, 0x8E77D8, 0x8E77DC, 0x8E7CAC, 0x8E77E4, 
								   0xF6838, 0xF6844, 0xF6854, 0xF6C54, 0xF6C58, 0xF685C, 0xF6860,
								   0x82988, 0x82DF4, 0x82DF8, 0x82E04, 0x82E0C, 0x82E10, 0x82E14, 0x82E1C, 0x82E24, 0x82E2C, 0x82E30, 0x82E34, 0x82E54, 0x82E5C, 0x82E64, 0x82E7C };
void Patch_c_clientInfo_t()
{
	SearchAndPatch(c_clientInfo_t_patches, 31, c_clientInfo_t_start, (DWORD)c_clientinfo);
 
	//Various limits for loops
	//iw4m_server.exe+FE65E - 81 FD C8D48E00        - cmp ebp,iw4m_server.exe+4ED4C8
	*(int*)0x4FE660 = sizeof(clientInfo_t) * MAX_CLIENTS + (DWORD)c_clientinfo;
	//iw4m_server.exe+17FDD1 - 81 FF C8D48E00        - cmp edi,iw4m_server.exe+4ED4C8
	*(int*)0x57FDD3 = sizeof(clientInfo_t) * MAX_CLIENTS + (DWORD)c_clientinfo;
	//iw4m_server.exe+F23B8 - 81 FE D4D48E00        - cmp esi,iw4m_server.exe+4ED4D4
	*(int*)0x4F23BA = sizeof(clientInfo_t) * MAX_CLIENTS + (DWORD)c_clientinfo + 0xC;
	//iw4m_server.exe+7464F - 81 FE ECD48E00        - cmp esi,iw4m_server.exe+4ED4EC
	*(int*)0x474651 = sizeof(clientInfo_t) * MAX_CLIENTS + (DWORD)c_clientinfo + 0x24;
	//iw4m.exe+189E23 - 81 FE C4D98E00        - cmp esi,iw4m.exe+4ED9C4
	*(int*)0x589E25 = sizeof(clientInfo_t) * MAX_CLIENTS + (DWORD)c_clientinfo + 0x4FC;	
	//iw4m.exe+189E86 - 81 FE C4D98E00        - cmp esi,iw4m.exe+4ED9C4
	*(int*)0x589E88 = sizeof(clientInfo_t) * MAX_CLIENTS + (DWORD)c_clientinfo + 0x4FC;
	//iw4m.exe+19E820 - 81 FE F4D48E00        - cmp esi,iw4m.exe+4ED4F4
	*(int*)0x59E822 = sizeof(clientInfo_t) * MAX_CLIENTS + (DWORD)c_clientinfo + 0x2C;
}
 
score_t scores [MAX_CLIENTS+1]; //why +1 ? it also uses the offset 0x863C10 to calculate which is -0x28 before the real start
int scores_t_patches [4] = { 0x863C38, 0x863C48, 0x863C4C, 0x863C10 };
void Patch_scores_t()
{
	SearchAndPatch(scores_t_patches, 4, scores_t_start, (DWORD)&scores[1]); //the actual start is at scores[1]
}
 
void __declspec(naked) fastzeroHk()
{
	_asm
	{
		pushad
		mov ebp, esp
		sub esp, __LOCAL_SIZE
	}
	int StructSize;
	int ReturnAd;
	__asm mov eax,[ebp+0x70]
	__asm mov ReturnAd,eax
	//we don't care about it setting old structs to 0, we just also want to set our new structs to 0
	switch(ReturnAd)
	{
	case 0x4AD343:
		__asm
		{
			push 0xBEEF
			sub esp,0x4C
			call fastzeroHk
			add esp,0x50
		}
		StructSize = sizeof(scores);
		__asm lea edi,[scores]
		break;
	case 0x4D54B4:
		StructSize = sizeof(cS_clients);
		__asm lea edi,[cS_clients]//clientState_s in clientActive_t
		break;
	case 0x4E32C4:
		StructSize = sizeof(c_clientinfo);
		__asm lea edi,[c_clientinfo]
		break;
	case 0xBEEF:
		StructSize = sizeof(activeSnapshots);
		__asm lea edi,[activeSnapshots];
		break;
	default:
		goto End;
	}
 
	__asm
	{
		mov eax,[StructSize]
		push eax
		push 00
		push edi
		call memset
		add esp,0xC
	}
End:
	__asm
	{
		mov esp, ebp
		popad
		ret
	}
}
 
void PatchMW2_MaxclientsClient()
{
	//map structs
	Patch_c_clientInfo_t();
	Patch_clientState_s();
	Patch_scores_t();
 
	//fix -> show scavenger bags and stuff
	*(BYTE*)0x5E2AB7 = MAX_CLIENTS;
 
	//show clients on radar
	//iw4m_srv.exe+6CD97 - C7 44 24 28 12000000  - mov [esp+28],00000012
	*(int*)0x46CD9B = MAX_CLIENTS;
 
	//red crosshair
	*(BYTE*)0x581E36 = MAX_CLIENTS;
	//draw enemy names over head check
	*(BYTE*)0x582376 = MAX_CLIENTS;
 
	//Fix for laggy movement of players > 17
	*(BYTE*)0x4D7685 = MAX_CLIENTS;
 
	//CL_ParseGamestate: bad clientNum %i
	*(BYTE*)0x5AC630 = MAX_CLIENTS;
 
	//Hook fastzero func which resets struct's mem to 0
	*(int*)0x6BDA56 = 0x909003EB;
	*(short*)0x6BDA5A = (short)0xE890;
	*(int*)0x6BDA5C = (int)&fastzeroHk - 0x6BDA60;
}
