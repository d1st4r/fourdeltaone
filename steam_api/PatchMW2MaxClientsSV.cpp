// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Increase maxclient limit - server part.
//
// Initial author: zxz0O0
// Started: 2013-03-18
// ==========================================================
 
#include "StdInc.h"

// Increase stack of func to add a char[MAX_CLIENTS]
void Patch_SVSendClientMessages()
{
	//iw4m.exe+517B6 - 83 EC 70              - sub esp,70
	BYTE OldArrayOffset = 0x3C; //according to IDA this is the offset of the old char[18] -> so this is now at 0x70
	BYTE OldSize = *(BYTE*)0x4517B8; //the old size (0x70)
	//we need to increase the stack, but 0x7F is max for 3byte sub
	//then again a hook for such a simple patch is a bit overkill, so why not just use some space above the func
	*(int*)0x4517B6 = 0x5390EFEB; //jump to our sub esp,.. command above SV_SendClientMessages
	*(short*)0x4517A7 = 0xEC81; //sub esp,
	*(int*)0x4517A9 = OldSize + MAX_CLIENTS + (4 - (MAX_CLIENTS % 4)); //new stack size - align it to 4bytes
	*(short*)0x4517AD = 0x0AEB; //jump back inside SV_SendClientMessages

	//iw4m.exe+5181D - 6A 12                 - push 12
	*(BYTE*)0x45181E = MAX_CLIENTS;
	//redirect variables to our new char[MAX_CLIENTS]
	//iw4m.exe+5181F - 8D 4C 24 48           - lea ecx,[esp+48]
	*(BYTE*)0x451822 = *(BYTE*)0x451822 - OldArrayOffset + OldSize;//because of pushs, the variable might (and IS) not be anymore at esp+3C so get the difference and add it to the new offset (0x70)
	//iw4m.exe+51917 - C6 44 2C 44 01        - mov byte ptr [esp+ebp+44],01
	*(BYTE*)0x45191A = *(BYTE*)0x45191A - OldArrayOffset + OldSize;
	//iw4m.exe+51963 - 80 7C 14 44 00        - cmp byte ptr [esp+edx+44],00
	*(BYTE*)0x451966 = *(BYTE*)0x451966 - OldArrayOffset + OldSize;
}


gclient_t gclients[MAX_CLIENTS];
void Patch_gclient_s()
{
	int SearchFor = gclient_s_start;
	SearchAndPatch(&SearchFor, 1, gclient_s_start, (DWORD)gclients);
}
 
int clientInfo_t_patches [21] = { 0x1A40008, 0x1A4040C, 0x1A403D0, 0x1A403A0, 0x1A40504,
								 //Using TLS
								 0x82988, 0x82DF4, 0x82DF8, 0x82E04, 0x82E0C, 0x82E10, 0x82E14, 0x82E1C, 0x82E24, 0x82E2C, 0x82E30, 0x82E34, 0x82E54, 0x82E5C, 0x82E64, 0x82E7C };
clientInfo_t clientinfo[MAX_CLIENTS];
void Patch_clientInfo_t()
{
	SearchAndPatch(clientInfo_t_patches, 21, clientInfo_t_srv, (DWORD)clientinfo, 0x600000);
 
	//various checks for loops
	//iw4m.exe+1E48D3 - 81 FE 1C62A401        - cmp esi,iw4m.exe+164621C
	*(int*)0x5E48D5 = sizeof(clientInfo_t) * MAX_CLIENTS + 0x4FC + (DWORD)clientinfo;
	//iw4m.exe+1E50C6 - 81 FE 1C62A401        - cmp esi,iw4m.exe+164621C
	*(int*)0x5E50C8 = sizeof(clientInfo_t) * MAX_CLIENTS + 0x4FC + (DWORD)clientinfo;
	//iw4m_server.exe+FE65E - 81 FD C8D48E00        - cmp ebp,iw4m_server.exe+4ED4C8
	*(int*)0x4FE660 = sizeof(clientInfo_t) * MAX_CLIENTS + (DWORD)clientinfo;
	//iw4m_server.exe+17FDD1 - 81 FF C8D48E00        - cmp edi,iw4m_server.exe+4ED4C8
	*(int*)0x57FDD3 = sizeof(clientInfo_t) * MAX_CLIENTS + (DWORD)clientinfo;
	//iw4m_server.exe+F23B8 - 81 FE D4D48E00        - cmp esi,iw4m_server.exe+4ED4D4
	*(int*)0x4F23BA = sizeof(clientInfo_t) * MAX_CLIENTS + (DWORD)clientinfo + 0xC;
	//iw4m_server.exe+7464F - 81 FE ECD48E00        - cmp esi,iw4m_server.exe+4ED4EC
	*(int*)0x474651 = sizeof(clientInfo_t) * MAX_CLIENTS + (DWORD)clientinfo + 0x24;
	//iw4m.exe+189E23 - 81 FE C4D98E00        - cmp esi,iw4m.exe+4ED9C4
	*(int*)0x589E25 = sizeof(clientInfo_t) * MAX_CLIENTS + (DWORD)clientinfo + 0x4FC;	
	//iw4m.exe+189E86 - 81 FE C4D98E00        - cmp esi,iw4m.exe+4ED9C4
	*(int*)0x589E88 = sizeof(clientInfo_t) * MAX_CLIENTS + (DWORD)clientinfo + 0x4FC;
}
 
int client_s_patches [14] = { 0x31D9390, 0x31D93A0, 0x31D93B8, 0x31D99EC, 0x31FA204, 0x31FA630, 0x31FA634, 0x31FA648, 0x31FA658, 0x321AE6C, 0x321AE80,
							  0x321AE88, 0x321AE8D, 0x321D289, };
client_t clients[MAX_CLIENTS];
void Patch_client_s()
{
	SearchAndPatch(client_s_patches, 14, client_s_start, (DWORD)clients);
	//check other code for offsets to client_s
}
 
int clientCompressionData_patches [3] = { 0x62C8228, 0x62C8248, 0x62C8268 };
clientCompressionData cCompressionData[MAX_CLIENTS];
void Patch_clientCompressionData()
{
	SearchAndPatch(clientCompressionData_patches, 3, clientCompressionData_start, (DWORD)cCompressionData, 0x600000);
}
 
/*AntilagClientStore acs;
void __declspec(naked) AntiLagPtrPatch()
{
	_asm
	{
		push eax
		//get return address
		mov eax,[esp+4]
		cmp eax,0x4C112C
		jne Restore
		//from G_AntiLagRewindClientPos
		lea eax,[esp+0x1DC]
		lea edi,acs
		mov [eax],ebx
		pop eax
		ret
 
		//from G_AntiLag_RestoreClientPos
Restore:
		//store return address
		mov [esp],eax
		lea eax,acs
		//later used as argument
		mov [esp+4],ebx
		lea ebx,[esp+0xC]
		mov [ebx],eax
		ret
	}
}*/
 
void AntiLagPatches()
{
	//Those AntiLag functions require very much fixing
	//In IW3 they are skipped if g_antilag == 0
 
	//for now just ret
	//G_AntiLagRewindClientPos
	*(BYTE*)0x4C1120 = 0xC3;
	//G_AntiLag_RestoreClientPos
	*(BYTE*)0x440040 = 0xC3;
	/*(int*)0x4C1122 = MAX_CLIENTS * sizeof(float) * 3 + 0xF8 - 0x8; // * 3 because vector
	*(int*)0x4C12A7 = MAX_CLIENTS * sizeof(float) * 3 + 0xF8 - 0x8;
	*(int*)0x455C65 = MAX_CLIENTS * sizeof(float) * 3;
	//size for memset
	*(int*)0x4C112F = sizeof(AntilagClientStore);
	//offset from realClientPositions[0][0] to unk[0][2]
	int rCPSize = sizeof(acs.realClientPositions);
	//also need to patch at 004C1178 and 004C1233
	//stack
	//G_AntiLag_RestoreClientPos
	*(BYTE*)0x44004D = 0xE8;
	*(int*)0x44004E = (int)&AntiLagPtrPatch - 0x440052;
	*(int*)0x440067 = rCPSize  + 0x8;
	*(int*)0x440077 = -rCPSize - 0x8;
	*(int*)0x440087 = -rCPSize - 0x4;
	*(int*)0x440090 = -rCPSize;
	//G_AntiLagRewindClientPos
	*(BYTE*)0x4C1127 = 0xE8;
	*(int*)0x4C1128 = (int)&AntiLagPtrPatch - 0x4C112C;
	*(short*)0x4C112C = (short)0x9090;
	*(int*)0x4C11AB = rCPSize  + 0x8;
	*(int*)0x4C11F8 = -rCPSize - 0x8;
	*(int*)0x4C1202 = -rCPSize - 0x4;
	*(int*)0x4C120A = -rCPSize;*/
}
 
void PatchMW2_MaxclientsServer()
{
	Patch_SVSendClientMessages();
	AntiLagPatches();
	Patch_gclient_s();
	Patch_clientInfo_t();
	Patch_client_s();
	Patch_clientCompressionData();
}
