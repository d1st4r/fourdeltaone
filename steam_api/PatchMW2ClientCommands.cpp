// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Re-implementing of cheat-like client commands.
//
// Initial author: NTAuthority
// Started: 2012-04-27
// ==========================================================

#include "StdInc.h"

#define GENTITY_TO_CLIENTNUM(entity) (((DWORD)entity - (DWORD)g_entities) / sizeof(gentity_t))
dvar_t** g_cheats = (dvar_t**)0x2098DE0;

void Cmd_SetViewpos_f(gentity_t* entity)
{
	if (!(*g_cheats)->current.boolean)
	{
		SV_GameSendServerCommand(GENTITY_TO_CLIENTNUM(entity), 0, va("%c \"GAME_CHEATSNOTENABLED\"", 101));
		return;
	}

	if (Cmd_ArgcSV() < 4 || Cmd_ArgcSV() > 6)
	{
		SV_GameSendServerCommand(GENTITY_TO_CLIENTNUM(entity), 0, va("%c \"Usage: setviewpos x y z yaw pitch\"", 101));
		return;
	}

	float pos[3];
	for (int i = 1; i < 4; i++)
	{
		pos[i - 1] = (float)atof(Cmd_ArgvSV(i));
	}

	float orientation[3] = { 0.0f, 0.0f, 0.0f };
	if (Cmd_ArgcSV() >= 5)
	{
		orientation[1] = (float)atof(Cmd_ArgvSV(4));
	}

	if (Cmd_ArgcSV() == 6)
	{
		orientation[0] = (float)atof(Cmd_ArgvSV(5));
	}

	TeleportPlayer(entity, pos, orientation);
}

typedef gentity_t* (__cdecl * G_Spawn_t)();
G_Spawn_t G_Spawn = (G_Spawn_t)0x4226F0;

typedef void (__cdecl * G_CallSpawnEntity_t)(gentity_t* entity);
G_CallSpawnEntity_t G_CallSpawnEntity = (G_CallSpawnEntity_t)0x441A20;

typedef void (__cdecl * G_SetModel_t)(gentity_t* entity, const char* model);
G_SetModel_t G_SetModel = (G_SetModel_t)0x437670;

typedef void (__cdecl * G_FinishSetModel_t)(gentity_t* entity, bool linkEntity);
G_FinishSetModel_t G_FinishSetModel = (G_FinishSetModel_t)0x481510;

void Cmd_SpawnModel_f(gentity_t* player)
{
	if (!(*g_cheats)->current.boolean)
	{
		SV_GameSendServerCommand(GENTITY_TO_CLIENTNUM(player), 0, va("%c \"GAME_CHEATSNOTENABLED\"", 101));
		return;
	}

	if (Cmd_ArgcSV() != 2)
	{
		SV_GameSendServerCommand(GENTITY_TO_CLIENTNUM(player), 0, va("%c \"Usage: model modelname\"", 101));
		return;
	}

	gentity_t* entity = G_Spawn();
	entity->classname = *(WORD*)0x1AA2E9C; // script_model
	entity->origin[0] = player->origin[0];
	entity->origin[1] = player->origin[1];
	entity->origin[2] = player->origin[2];

	G_CallSpawnEntity(entity);
	
	G_SetModel(entity, Cmd_ArgvSV(1));
	G_FinishSetModel(entity, true);
}

extern netadr_t redirectAddress;;
extern void SV_FlushRedirect( char *outputbuf );
#define SV_OUTPUTBUF_LENGTH ( 16384 - 16 )
extern char sv_outputbuf[];

void Cmd_Status_f(gentity_t* player)
{
	redirectAddress = svs_clients[GENTITY_TO_CLIENTNUM(player)].adr;

	Com_BeginRedirect(sv_outputbuf, SV_OUTPUTBUF_LENGTH, SV_FlushRedirect);

	Cmd_ExecuteSingleCommand(0, 0, "status\n");

	Com_EndRedirect();
}

bool ClientCommand_custom(int clientNum)
{
	gentity_t* gentity = &g_entities[clientNum];

	const char* command = Cmd_ArgvSV(0);

	if (!_stricmp(command, "setviewpos"))
	{
		Cmd_SetViewpos_f(gentity);
		return true;
	}
	else if (!_stricmp(command, "model"))
	{
		Cmd_SpawnModel_f(gentity);
		return true;
	}
	else if (!_stricmp(command, "status"))
	{
		Cmd_Status_f(gentity);
		return true;
	}

	return false;
}

// SV_ClientCommand calling ClientCommand
CallHook clientCommandHook;
DWORD clientCommandHookLoc = 0x6259FA;

#pragma optimize("", off)
void __declspec(naked) ClientCommandHookStub()
{
	__asm
	{
		mov eax, [esp + 4] // clientNum
		push eax
		call ClientCommand_custom
		add esp, 4h

		test al, al
		jnz returnStuff
		jmp clientCommandHook.pOriginal

returnStuff:
		retn
	}
}
#pragma optimize("", on)

// stop setviewpos commands from being sent off for notifying
CallHook executeCommandNotifyHook;
DWORD executeCommandNotifyHookLoc = 0x609595;

bool Cmd_ShouldCommandBeForwarded()
{
	return (_stricmp(Cmd_Argv(0), "setviewpos") != 0);
}

#pragma optimize("", off)
void __declspec(naked) ExecuteCommandNotifyHookStub()
{
	__asm
	{
		call Cmd_ShouldCommandBeForwarded

		test al, al
		jz returnStuff
		jmp executeCommandNotifyHook.pOriginal

returnStuff:
		retn
	}
}
#pragma optimize("", on)
extern int* demoPlaying;
extern int* clientState;

void StatusCommandHook(int channel, char* msg)
{
	if((*clientState) == 9 && (*demoPlaying) != 1)
	{
		CL_AddReliableCommand(0, "status");
	}
	else
	{
		Com_Printf(0, "Use this command when connected to a server.\n");
	}
}

void PatchMW2_ClientCommands()
{
	clientCommandHook.initialize(clientCommandHookLoc, ClientCommandHookStub);
	clientCommandHook.installHook();

	executeCommandNotifyHook.initialize(executeCommandNotifyHookLoc, ExecuteCommandNotifyHookStub);
	executeCommandNotifyHook.installHook();

	call(0x624D56, StatusCommandHook, PATCH_CALL);
}