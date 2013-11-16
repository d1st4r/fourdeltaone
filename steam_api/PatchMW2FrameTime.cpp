// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Alternate frame waiting function, should
//          fix maxFPS variances on some systems.
//
// Initial author: ioquake3 developers; patchset by NTA
// Started: 2013-05-26
// ==========================================================

#include "StdInc.h"
#include <ws2tcpip.h>

static SOCKET& ip_socket = *(SOCKET*)0x64A3008;
extern SOCKET ip6_socket;
dvar_t*& com_sv_running = *(dvar_t**)0x1AD7934;

typedef const char* (__cdecl * NET_ErrorString_t)();
NET_ErrorString_t NET_ErrorString = (NET_ErrorString_t)0x4E7720;

void NET_Sleep(int msec)
{
	struct timeval timeout;
	fd_set fdr;
	int retval;
	SOCKET highestfd = INVALID_SOCKET;

	if(msec < 0)
		msec = 0;

	FD_ZERO(&fdr);

	if(ip_socket != INVALID_SOCKET)
	{
		FD_SET(ip_socket, &fdr);

		highestfd = ip_socket;
	}

	if (ip6_socket != INVALID_SOCKET)
	{
		FD_SET(ip6_socket, &fdr);

		if (ip6_socket > highestfd)
		{
			highestfd = ip6_socket;
		}
	}

#ifdef _WIN32
	if(highestfd == INVALID_SOCKET)
	{
		// windows ain't happy when select is called without valid FDs
		SleepEx(msec, 0);
		return;
	}
#endif

	timeout.tv_sec = msec/1000;
	timeout.tv_usec = (msec%1000)*1000;

	retval = select(highestfd + 1, &fdr, NULL, NULL, &timeout);

	if(retval == SOCKET_ERROR)
		Com_Printf(0, "Warning: select() syscall failed: %s\n", NET_ErrorString());
	else if (retval > 0)
	{
		// process packets
		if (com_sv_running->current.boolean)
		{
			__asm
			{
				mov eax, 458160h
				call eax
			}
		}
		else
		{
			__asm
			{
				mov eax, 49F0B0h
				call eax
			}
		}
	}
}

uint32_t& com_frameTime = *(uint32_t*)0x1AD8F3C;
dvar_t* com_busyWait;

int Com_TimeVal(int minMsec)
{
	int timeVal;

	timeVal = Com_Milliseconds() - com_frameTime;

	if(timeVal >= minMsec)
		timeVal = 0;
	else
		timeVal = minMsec - timeVal;

	return timeVal;
}

typedef void (__cdecl * Com_EventLoop_t)();
Com_EventLoop_t Com_EventLoop = (Com_EventLoop_t)0x43D140;

uint32_t Com_FrameWait(int minMsec)
{
	int timeVal;
	int timeValSV;

	if (!com_busyWait)
	{
		com_busyWait = Dvar_RegisterBool("com_busyWait", false, DVAR_FLAG_SAVED, "Use a classic-style idle loop for waiting.");
	}

	do
	{
		if(com_sv_running->current.boolean)
		{
			// should not be needed with IW4's rate settings?
			//timeValSV = SV_SendQueuedPackets();

			timeVal = Com_TimeVal(minMsec);

			//if(timeValSV < timeVal)
				//timeVal = timeValSV;
		}
		else
			timeVal = Com_TimeVal(minMsec);

		if(com_busyWait->current.boolean || timeVal < 1)
			NET_Sleep(0);
		else
			NET_Sleep(timeVal - 1);
	} while(Com_TimeVal(minMsec));

	uint32_t lastTime = com_frameTime;
	Com_EventLoop();
	com_frameTime = Com_Milliseconds();

	return com_frameTime - lastTime;
}

void __declspec(naked) Com_FrameWaitHookStub()
{
	__asm
	{
		push edi // minMsec
		call Com_FrameWait // this function should write com_frameTime
		add esp, 4

		mov ecx, eax // msec. msec should also remain in eax for now

		// return stuff, skipping the multi-register move
		mov edx, 1AD7934h // com_sv_running
		cmp byte ptr [edx + 10h], 0

		push 47DDC1h
		retn
	}
}

int& sv_residualTime = *(int*)0x2089E14;

void SV_FrameWaitFunc()
{
	if (sv_residualTime < 50)
	{
		NET_Sleep(50 - sv_residualTime);
	}
}

void __declspec(naked) SV_FrameWaitHookStub()
{
	__asm
	{
		call SV_FrameWaitFunc

		push 4CD420h
		retn
	}
}

void PatchMW2_FrameTime()
{
	if (!GAME_FLAG(GAME_FLAG_DEDICATED))
	{
		call(0x47DD80, Com_FrameWaitHookStub, PATCH_JUMP);
	}
	else
	{
		call(0x4BAAAD, SV_FrameWaitHookStub, PATCH_CALL);
	}
}