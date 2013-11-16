// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: reproduction of script crashes caused by dropping
//          clients.
//
// Initial author: NTAuthority
// Started: 2011-07-05
// ==========================================================

#include "StdInc.h"

static int lastCase = 0;

void SV_KickClient(void* client, const char* reason)
{
	DWORD func = 0x6249A0;
	__asm
	{
		push edi
		push esi
		mov edi, 0
		mov esi, client
		push reason
		push 0
		push 0
		call func
		add esp, 0Ch
		pop esi
		pop edi
	}
}

void ReproCase_ClientDrop()
{
	if ((Com_Milliseconds() - lastCase) > 10000)
	{
		lastCase = Com_Milliseconds();

		BYTE* clientAddress = (BYTE*)svs_clients;

		for (int i = 0; i < *svs_numclients; i++) {
			if (*clientAddress >= 5) {
				int clientNum = CLIENTNUM(clientAddress);

				SV_KickClient(clientAddress, "Hi!");
			}

			clientAddress += 681872;
		}
	}
}