// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Modern Warfare 2 patches: remote console support
//          (client-side component)
//
// Initial author: NTAuthority
// Started: 2011-05-22
// ==========================================================

#include "StdInc.h"

static struct
{
	char password[64];
	netadr_t address;
} rconGlob;

void CL_Rcon_f()
{
	if (Cmd_Argc() < 2)
	{
		Com_Printf(0, "USAGE: rcon <command> <options...>\n");
		return;
	}

	char* command = Cmd_Argv(1);
	if (!_stricmp(command, "login"))
	{
		if (Cmd_Argc() != 3)
		{
			Com_Printf(0, "USAGE: rcon login <password>\n");
			return;
		}

		char* password = Cmd_Argv(2);
		if (strlen(password) >= sizeof(rconGlob.password))
		{
			Com_Printf(0, "rcon password must be %i characters or less\n", sizeof(rconGlob.password));
			return;
		}

		strcpy(rconGlob.password, password);
	}
	else if (!_stricmp(command, "logout"))
	{
		if (!rconGlob.password[0])
		{
			Com_Printf(0, "Not logged in\n");
			return;
		}

		rconGlob.password[0] = '\0';
	}
	else if (!_stricmp(command, "host"))
	{
		if (Cmd_Argc() != 3)
		{
			Com_Printf(0, "USAGE: rcon host <address>\n");
			return;
		}

		if (!NET_StringToAdr(Cmd_Argv(2), &rconGlob.address))
		{
			Com_Printf(0, "bad host address\n");
			return;
		}

		if (!rconGlob.address.port)
		{
			rconGlob.address.port = 8305;
		}
	}
	else
	{
		if (!rconGlob.password[0])
		{
			Com_Printf(0, "You need to log in with 'rcon login <password>' before using rcon.\n");
			return;
		}

		size_t current = 0;
		char cmdBuffer[1024] = {0};

		current = Com_AddToString("rcon ", cmdBuffer, current, sizeof(cmdBuffer), false);
		current = Com_AddToString(rconGlob.password, cmdBuffer, current, sizeof(cmdBuffer), false);

		for (int arg = 1; arg < Cmd_Argc(); arg++)
		{
			current = Com_AddToString(" ", cmdBuffer, current, sizeof(cmdBuffer), false);
			current = Com_AddToString(Cmd_Argv(arg), cmdBuffer, current, sizeof(cmdBuffer), true);
		}

		if (current == sizeof(cmdBuffer))
		{
			Com_Printf(0, "rcon commands are limited to %i characters\n", sizeof(cmdBuffer) - 1);
			return;
		}

		netadr_t adr;

		if (*(int*)0xB2C540 >= 5)
		{
			memcpy(&adr, (void*)0xA5EA44, sizeof(adr));
		}
		else
		{
			adr = rconGlob.address;

			if (adr.type == NA_BAD)
			{
				Com_Printf(0, "Can't determine rcon target.  You can fix this by either:\n");
				Com_Printf(0, "1) Joining the server as a player.\n");
				Com_Printf(0, "2) Setting the host server with 'rcon host <address>'.\n");
				return;
			}
		}

		NET_OutOfBandPrint(0, adr, "%s", cmdBuffer);
	}
}

cmd_function_t cl_rcon;

void PatchMW2_RemoteConsoleClient()
{
	rconGlob.address.type = NA_BAD;
	rconGlob.password[0] = '\0';

	Cmd_AddCommand("rcon", CL_Rcon_f, &cl_rcon, 0);
}

bool RemoteConsole_HasServer()
{
	return ((*(int*)0xB2C540 >= 5) || rconGlob.address.type != NA_BAD);
}

bool RemoteConsole_LoggedIn()
{
	return (rconGlob.password[0] != '\0');
}