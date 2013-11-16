// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Modern Warfare 2 patches: out-of-band messages
//
// Initial author: NTAuthority
// Started: 2011-11-25 (copied from PatchMW2Status.cpp)
// ==========================================================

#include "StdInc.h"

static enum
{
	GSR_RCON,
	GSR_GETSERVERSRESPONSE,
	GSR_INFORESPONSE,
	GSR_STATUSRESPONSE,
} lastGsrCommand;

void SVC_RemoteCommand(netadr_t from, void* msg);
void CL_ServerInfoPacket(netadr_t from, msg_t* msg);
void CL_ServersResponsePacket(msg_t* msg);

void CL_DeployOOB(netadr_t from, msg_t* msg)
{
	if (lastGsrCommand == GSR_RCON)
	{
		return SVC_RemoteCommand(from, msg);
	}

	if (lastGsrCommand == GSR_INFORESPONSE)
	{
		return CL_ServerInfoPacket(from, msg);
	}

	if (lastGsrCommand == GSR_GETSERVERSRESPONSE)
	{
		return CL_ServersResponsePacket(msg);
	}
	
#ifdef WE_DO_WANT_NUI
	if (lastGsrCommand == GSR_STATUSRESPONSE)
	{
		return g_nuiDraw->HandleStatusResponse(from, msg);
	}
#endif
}

CallHook gsrCmpHook;
DWORD gsrCmpHookLoc = 0x5AA709;

int GsrCmpHookFunc(const char* a1, const char* a2)
{
	//int result = _strnicmp(a1, "rcon", 4);

	if (!_strnicmp(a1, "rcon", 4))
	{
		lastGsrCommand = GSR_RCON;
		return 0;
	}

	if (!_strnicmp(a1, "infoResponse", 12))
	{
		lastGsrCommand = GSR_INFORESPONSE;
		return 0;
	}

	if (!_strnicmp(a1, "getServersResponse", 18))
	{
		lastGsrCommand = GSR_GETSERVERSRESPONSE;
		return 0;
	}

#ifdef WE_DO_WANT_NUI
	if (!_strnicmp(a1, "statusResponse", 14))
	{
		lastGsrCommand = GSR_STATUSRESPONSE;
		return 0;
	}
#endif

	return 1;
}

void __declspec(naked) GsrCmpHookStub()
{
	__asm jmp GsrCmpHookFunc
}

bool wasGetServers;

void __declspec(naked) CL_DeployOOBStub()
{
	__asm
	{
		push ebp //C54
		// esp = C54h?
		mov eax, [esp + 0C54h + 14h]
		push eax
		mov eax, [esp + 0C58h + 10h]
		push eax
		mov eax, [esp + 0C5Ch + 0Ch]
		push eax
		mov eax, [esp + 0C60h + 08h]
		push eax
		mov eax, [esp + 0C64h + 04h]
		push eax
		call CL_DeployOOB
		add esp, 14h
		add esp, 4h
		mov al, 1
		//C50
		pop edi //C4C
		pop esi //C48
		pop ebp //C44
		pop ebx //C40
		add esp, 0C40h
		retn
	}
}

void PatchMW2_OOB()
{
	// maximum size in NET_OutOfBandPrint
	*(DWORD*)0x4AEF08 = 0x1FFFC;
	*(DWORD*)0x4AEFA3 = 0x1FFFC;

	// client-side OOB handler
	*(int*)0x5AA715 = ((DWORD)CL_DeployOOBStub) - 0x5AA713 - 6;

	gsrCmpHook.initialize(gsrCmpHookLoc, GsrCmpHookStub);
	gsrCmpHook.installHook();
}