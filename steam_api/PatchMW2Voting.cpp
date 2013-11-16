// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: voting for the win!
//          sort ofbased on the smod playersvotes plugin.
//
// Initial author: NTAuthority
// Started: 2012-05-18
// ==========================================================

#include "StdInc.h"
#include <Shlwapi.h>

#define NUM_VOTE_TYPES 2
#define VOTE_BAN 0
#define VOTE_KICK 1

static dvar_t* voteRatio[NUM_VOTE_TYPES];
static dvar_t* voteMin[NUM_VOTE_TYPES];
static dvar_t* voteDelay[NUM_VOTE_TYPES];
static dvar_t* voteLimit[NUM_VOTE_TYPES];

static dvar_t* voteDebug;

void Vote_Init()
{
	voteRatio[VOTE_BAN] = Dvar_RegisterFloat("g_voteBanRatio", 0.80f, 0.0f, 1.0f, DVAR_FLAG_NONE, "ratio required for voteban");
	voteRatio[VOTE_KICK] = Dvar_RegisterFloat("g_voteKickRatio", 0.60f, 0.0f, 1.0f, DVAR_FLAG_NONE, "ratio required for votekick");

	voteMin[VOTE_BAN] = Dvar_RegisterInt("g_voteBanMin", 4, -1, MAX_CLIENTS, DVAR_FLAG_NONE, "minimum votes required for voteban. -1 to disable.");
	voteMin[VOTE_KICK] = Dvar_RegisterInt("g_voteKickMin", 4, -1, MAX_CLIENTS, DVAR_FLAG_NONE, "minimum votes required for votekick. -1 to disable.");

	voteDelay[VOTE_BAN] = Dvar_RegisterInt("g_voteBanDelay", 30, 0, 3600, DVAR_FLAG_NONE, "time in seconds after game init before voteban is allowed");
	voteDelay[VOTE_KICK] = Dvar_RegisterInt("g_voteKickDelay", 30, 0, 3600, DVAR_FLAG_NONE, "time in seconds after game init before votekick is allowed");

	voteLimit[VOTE_BAN] = Dvar_RegisterInt("g_voteBanLimit", 3, 0, 100, DVAR_FLAG_NONE, "maximum amount of votebans allowed per player");
	voteLimit[VOTE_KICK] = Dvar_RegisterInt("g_voteKickLimit", 3, 0, 100, DVAR_FLAG_NONE, "maximum amount of votekicks allowed per player");

	voteDebug = Dvar_RegisterBool("g_voteDebug", false, DVAR_FLAG_NONE, "Enable vote debugging");
}

static int startTime;
static int voteCount[NUM_VOTE_TYPES][MAX_CLIENTS];
static bool votedForKick[MAX_CLIENTS][MAX_CLIENTS];
static bool votedForBan[MAX_CLIENTS][MAX_CLIENTS];
static int votes[MAX_CLIENTS];
static int lastVote[MAX_CLIENTS];
static bool voteRunning;
static int voteExpire;
static int voteTarget;
static int voteType;
static bool voteAdvertised[MAX_CLIENTS];

void Vote_OnInitGame()
{
	startTime = Com_Milliseconds();

	voteRunning = false;
	voteExpire = 0;

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		votes[i] = 0;
	}

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		voteCount[VOTE_BAN][i] = 0;
		voteCount[VOTE_KICK][i] = 0;
	}
}

void Vote_OnDisconnect(int clientNum)
{
	voteCount[VOTE_BAN][clientNum] = 0;
	voteCount[VOTE_KICK][clientNum] = 0;
	
	for (int i = 0; i <= MAX_CLIENTS; i++)
	{
		votedForKick[i][clientNum] = false;
		votedForKick[clientNum][i] = false;

		votedForBan[i][clientNum] = false;
		votedForBan[clientNum][i] = false;
	}

	votes[clientNum] = 0;
	lastVote[clientNum] = 0;

	voteAdvertised[clientNum] = false;
}

bool Vote_IsTypeDisabled(int type)
{
	return (voteMin[type]->current.integer == -1 || voteLimit[type]->current.integer == 0);
}

int Vote_FindTarget(const char* pattern)
{
	if (voteDebug->current.boolean)
	{
		Com_Printf(0, "finding pattern %s\n", pattern);
	}

	if (isdigit(pattern[0]))
	{
		if (pattern[1] == '\0' || isdigit(pattern[1]))
		{
			int clientNum = atoi(pattern);

			if (clientNum >= MAX_CLIENTS)
			{
				return -1;
			}
			else
			{
				return clientNum;
			}
		}
	}

	if (strlen(pattern) == 0)
	{
		return -1;
	}

	int matches = 0;
	int matchNum = -1;

	for (int i = 0; i < *svs_numclients; i++)
	{
		if (svs_clients[i].state >= 3)
		{
			if (StrStrI(svs_clients[i].name, pattern))
			{
				matches++;
				matchNum = i;
			}
		}
	}

	if (matches > 1)
	{
		return -2;
	}
	else if (matches == 0)
	{
		return -1;
	}
	else
	{
		return matchNum;
	}
}

void Vote_LogTargets(int clientNum, const char* pattern)
{
	for (int i = 0; i < *svs_numclients; i++)
	{
		if (svs_clients[i].state >= 3)
		{
			if (StrStrI(svs_clients[i].name, pattern))
			{
				SV_GameSendServerCommand(clientNum, 0, va("%c \"%s (^2%i^7)\"", 104, svs_clients[i].name, i));
			}
		}
	}

	SV_GameSendServerCommand(clientNum, 0, va("%c \"Try again using a number instead of a name.\"", 104));
}

void Vote_StartVote(int type, int clientNum, const char* pattern)
{
	if (voteRunning)
	{
		SV_GameSendServerCommand(clientNum, 0, va("%c \"A vote is already running.\"", 104));
		return;
	}

	// find the target
	int targetNum = Vote_FindTarget(pattern);

	if (targetNum < 0)
	{
		if (targetNum == -1)
		{
			SV_GameSendServerCommand(clientNum, 0, va("%c \"No such client.\"", 104));
		}
		else if (targetNum == -2)
		{
			Vote_LogTargets(clientNum, pattern);
		}

		return;
	}

	if (voteDebug->current.boolean)
	{
		Com_Printf(0, "target is %i\ninitiator is %i\n", targetNum, clientNum);
	}

	lastVote[clientNum] = Com_Milliseconds();
	voteCount[type][clientNum]++;

	SV_GameSendServerCommand(-1, 0, va("%c \"%s started a vote.\"", 102, svs_clients[clientNum].name));
	
	if (type == VOTE_BAN)
	{
		SV_GameSendServerCommand(-1, 0, va("%c \"Ban ^2%s^7? Type /yes or /no.\"", 104, svs_clients[targetNum].name));

		votedForBan[clientNum][targetNum] = true;
	}
	else if (type == VOTE_KICK)
	{
		SV_GameSendServerCommand(-1, 0, va("%c \"Kick ^2%s^7? Type /yes or /no.\"", 104, svs_clients[targetNum].name));

		votedForKick[clientNum][targetNum] = true;
	}

	voteRunning = true;
	voteExpire = (Com_Milliseconds() + 45000);
	voteTarget = targetNum;
	voteType = type;

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		votes[i] = 0;
		
		if (type == VOTE_KICK)
		{
			if (votedForKick[i][targetNum])
			{
				votes[i] = 1;
			}
		}
		else if (type == VOTE_BAN)
		{
			if (votedForBan[i][targetNum])
			{
				votes[i] = 1;
			}
		}
	}
}

void Vote_HandleStartVote(int type, int clientNum, const char* pattern)
{
	if (!Vote_IsTypeDisabled(type))
	{
		if (voteCount[type][clientNum] < voteLimit[type]->current.integer)
		{
			if ((Com_Milliseconds() - lastVote[clientNum]) >= 90000 || Com_Milliseconds() < 90000)
			{
				if ((Com_Milliseconds() - startTime) >= (voteDelay[type]->current.integer * 1000))
				{
					Vote_StartVote(type, clientNum, pattern);
				}
				else
				{
					SV_GameSendServerCommand(clientNum, 0, va("%c \"You cannot call a vote yet.\"", 104));
				}
			}
			else
			{
				SV_GameSendServerCommand(clientNum, 0, va("%c \"You need to wait until you can call a vote again.\"", 104));
			}
		}
		else
		{
			SV_GameSendServerCommand(clientNum, 0, va("%c \"You have exceeded your vote limit.\"", 104));
		}
	}
}

void Vote_Count(int* yes, int* no)
{
	int nYes = 0;
	int nNo = 0;

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (votes[i] == 1)
		{
			nYes++;
		}
		else if (votes[i] == -1)
		{
			nNo++;
		}

		if (voteDebug->current.boolean)
		{
			Com_Printf(0, "vote counting: %i voted %i\n", i, votes[i]);
		}
	}

	*yes = nYes;
	*no = nNo;

	if (voteDebug->current.boolean)
	{
		Com_Printf(0, "vote counting: %i yes and %i no\n", *yes, *no);
	}
}

int Vote_RequiredVotes()
{
	svstatus_t status;
	SV_GetStatus(&status);

	int required = (voteRatio[voteType]->current.value * status.curClients);

	if (required < voteMin[voteType]->current.integer)
	{
		required = voteMin[voteType]->current.integer;
	}

	return required;
}

void Vote_Finalize();

void Vote_HandleVote(int clientNum, bool yes)
{
	if (!voteRunning)
	{
		SV_GameSendServerCommand(clientNum, 0, va("%c \"No vote is in progress.\"", 104));
		return;
	}

	if (voteDebug->current.boolean)
	{
		Com_Printf(0, "%i voted %i\n", clientNum, (yes) ? 1 : 0);
	}

	if (!yes && clientNum == voteTarget)
	{
		SV_GameSendServerCommand(clientNum, 0, va("%c \"You can not vote in favor of yourself.\"", 104));
		return;
	}

	if (voteType == VOTE_KICK)
	{
		votedForKick[clientNum][voteTarget] = yes;
	}
	else if (voteType == VOTE_BAN)
	{
		votedForBan[clientNum][voteTarget] = yes;
	}

	if (yes)
	{
		votes[clientNum] = 1;
	}
	else
	{
		votes[clientNum] = -1;
	}

	int nyes; int no;
	Vote_Count(&nyes, &no);

	SV_GameSendServerCommand(-1, 0, va("%c \"%i/%i votes. Type /yes or /no.\"", 104, nyes, no));
}

void Vote_Finalize()
{
	int yes; int no;
	Vote_Count(&yes, &no);

	float ratio = (float)yes / (float)no;
	int voteCount = yes + no;
	
	voteRunning = false;

	if (voteCount < voteMin[voteType]->current.integer)
	{
		SV_GameSendServerCommand(-1, 0, va("%c \"Vote failed - not enough votes.\"", 102));
		return;
	}

	if (ratio >= voteRatio[voteType]->current.value)
	{
		SV_GameSendServerCommand(-1, 0, va("%c \"Vote passed.\"", 102));

		if (voteType == VOTE_KICK)
		{
			Cmd_ExecuteSingleCommand(0, 0, va("clientkick %i", voteTarget));
		}
		else if (voteType == VOTE_BAN)
		{
			Cmd_ExecuteSingleCommand(0, 0, va("tempbanclient %i", voteTarget));
		}
	}
	else
	{
		SV_GameSendServerCommand(-1, 0, va("%c \"Vote failed.\"", 102));
	}
}

void Vote_Advertise(int clientNum)
{
	if (Vote_IsTypeDisabled(VOTE_KICK))
	{
		return;
	}

	if (!voteAdvertised[clientNum])
	{
		SV_GameSendServerCommand(clientNum, 0, va("%c \"To vote to kick a cheater, use /votekick [part of name].\"", 104));
		voteAdvertised[clientNum] = true;
	}
}

bool Vote_OnSay(int clientNum, const char* message)
{
	if (!GAME_FLAG(GAME_FLAG_DEDICATED))
	{
		return false;
	}

	if (!_strnicmp(message, "/votekick ", 10))
	{
		Vote_HandleStartVote(VOTE_KICK, clientNum, &message[10]);
		return true;
	}

	if (!_strnicmp(message, "/voteban ", 9))
	{
		Vote_HandleStartVote(VOTE_BAN, clientNum, &message[9]);
		return true;
	}

	if (!_stricmp(message, "/y") || !_stricmp(message, "/yes"))
	{
		Vote_HandleVote(clientNum, true);
		return true;
	}

	if (!_stricmp(message, "/n") || !_stricmp(message, "/no"))
	{
		Vote_HandleVote(clientNum, false);
		return true;
	}

	if (StrStrI(message, "hack") || StrStrI(message, "cheat") || StrStrI(message, " wh ") || StrStrI(message, "aimbot"))
	{
		Vote_Advertise(clientNum);
	}

	return false;
}

void Vote_OnFrame()
{
	if (!voteRunning)
	{
		return;
	}

	if (Com_Milliseconds() >= voteExpire)
	{
		Vote_Finalize();
	}
}