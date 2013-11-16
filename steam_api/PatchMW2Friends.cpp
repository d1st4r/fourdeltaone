// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Friend list support.
//
// Initial author: NTAuthority
// Started: 2012-06-11
// ==========================================================

#include "StdInc.h"
#include "PatchMW2UIScripts.h"
#include "ServerStorage.h"

#define MAX_FRIENDS 1024

static struct  
{
	NPID friends[MAX_FRIENDS];
	std::unordered_map<NPID, NPProfileData> friendProfiles;
	char connectHere[256];
} g_friends;

int FriendList_GetItemCount()
{
	return NP_GetNumFriends();
}

typedef int (__cdecl * GetRankForExperience_t)(int experience);
GetRankForExperience_t GetRankForExperience = (GetRankForExperience_t)0x4FF8A0;

typedef void (__cdecl * GetRankIcon_t)(int level, int prestige, Material** material);
GetRankIcon_t GetRankIcon = (GetRankIcon_t)0x4A7B30;

const char* FriendList_GetItemText(int index, int column)
{
	NPID friendID = NP_GetFriend(index);

	switch (column)
	{
		case 0:
		{
			static char str[32];
			strcpy(str, "^\x02\x30\x30????");
			
			int rank = GetRankForExperience(g_friends.friendProfiles[friendID].experience);
			int prestige = g_friends.friendProfiles[friendID].prestige;
			Material* material = NULL;

			GetRankIcon(rank, prestige, &material);			

			if (material)
			{
				*(Material**)(&str[4]) = material;
				return str;
			}
			else
			{
				return NULL;
			}
		}
		case 1: // rank
		{
			return va("%i", GetRankForExperience(g_friends.friendProfiles[friendID].experience) + 1);
		}
		case 2: // name
		{
			return NP_GetFriendName(friendID);
		}
		case 3: // status
		{
			EPresenceState presence = NP_GetFriendPresence(friendID);

			if (presence == PresenceStateOffline)
			{
				return "Offline";
			}

			// check in-game-ness
			const char* currentGame = NP_GetFriendRichPresence(friendID, "currentGame");

			if (currentGame && strcmp(currentGame, "iw4mp"))
			{
				if (presence == PresenceStateAway || presence == PresenceStateExtendedAway)
				{
					return "Away";
				}
				else if (PresenceStateOnline)
				{
					return "Online";
				}
			}

			const char* richPresenceBody = NP_GetFriendRichPresenceBody(friendID);

			// very dirty hack, this nonenonenone
			if (!richPresenceBody || !richPresenceBody[0] || richPresenceBody[0] == '>' || !_stricmp(richPresenceBody, "nonenonenone"))
			{
				return "Playing IW4M";
			}

			return richPresenceBody;
		}
	}

	return "";
}

void FriendList_Select(int item)
{
	NPID npID = NP_GetFriend(item);

	Com_Printf(0, "i %i npid %llx\n", item, npID);
	const char* connectString = NP_GetFriendRichPresence(npID, "connect");

	if (connectString && connectString[0])
	{
		strncpy(g_friends.connectHere, connectString, sizeof(g_friends.connectHere) - 1);
	}
	else
	{
		g_friends.connectHere[0] = '\0';
	}
}

StompHook cgParseServerInfoHook;
DWORD cgParseServerInfoHookLoc = 0x4CD023;

void CG_ParseServerInfoHookFunc()
{
	NP_SetRichPresenceBody((const char*)0x7ED3F8);
	NP_SetRichPresence("connect", CL_GetServerIPAddress());
	NP_StoreRichPresence();

	// not related to friends but as we're hooking here anyway
	netadr_t* adr = (netadr_t*)0xA1E888;
	ServerStorage_AddEntry(&historyList, *adr, (const char*)0x7ED3F8);
}

CallHook clDisconnectHook;
DWORD clDisconnectHookLoc = 0x403582;

void CL_DisconnectHookFunc()
{
	NP_SetRichPresenceBody("nonenonenone");
	NP_SetRichPresence("connect", NULL);
	NP_StoreRichPresence();

	NP_SendRandomString("dis");
}

void __declspec(naked) CL_DisconnectHookStub()
{
	__asm
	{
		call CL_DisconnectHookFunc
		jmp clDisconnectHook.pOriginal
	}
}

void UI_friendAction(char* args)
{
	// TODO: if ingame, invite
	if (g_friends.connectHere[0])
	{
		Cmd_ExecuteSingleCommand(0, 0, va("connect %s", g_friends.connectHere));
	}
	else
	{
		Cbuf_AddText(0, "snd_playLocal exit_prestige\n");
	}
}

typedef struct  
{
	char pad1[8];
	NPID xuid;
	char pad2[40];
} sessionPlayer_t;

typedef struct
{
	char stuff[560];
	sessionPlayer_t players[MAX_CLIENTS];
} session_t;

session_t* ingameSession = (session_t*)0x66B7008;

StompHook isClientInPartyHook;
DWORD isClientInPartyHookLoc = 0x493130;

extern int userIDs[19];

bool IsClientInParty(int controller, int clientNum)
{
	//NPID npID = (ingameSession->players[clientNum].xuid);
	NPID npID = (0x110000100000000 | userIDs[clientNum]);

	if (NP_GetFriendPresence(npID) != PresenceStateUnknown)
	{
		return true;
	}

	return false;
}

void PatchMW2_Friends()
{
	UIFeeder_t feeder;
	feeder.feeder = 15.0f;
	feeder.GetItemCount = FriendList_GetItemCount;
	feeder.GetItemText = FriendList_GetItemText;
	feeder.Select = FriendList_Select;

	UIFeeders.push_back(feeder);

	AddUIScript("friendAction", UI_friendAction);

	cgParseServerInfoHook.initialize(cgParseServerInfoHookLoc, CG_ParseServerInfoHookFunc);
	cgParseServerInfoHook.installHook();

	clDisconnectHook.initialize(clDisconnectHookLoc, CL_DisconnectHookStub);
	clDisconnectHook.installHook();

	isClientInPartyHook.initialize(isClientInPartyHookLoc, IsClientInParty);
	isClientInPartyHook.installHook();
}

static void Friends_StatsCB(NPAsync<NPGetProfileDataResult>* async)
{
	NPGetProfileDataResult* result = async->GetResult();

	for (int i = 0; i < result->numResults; i++)
	{
		g_friends.friendProfiles[result->results[i].npID] = result->results[i];
	}
}

void Friends_ObtainStats()
{
	// make a list of friends
	int i;

	for (i = 0; i < NP_GetNumFriends(); i++)
	{
		if (i >= MAX_FRIENDS)
		{
			break;
		}

		g_friends.friends[i] = NP_GetFriend(i);
	}

	// obtain friend stats
	static NPProfileData profiles[MAX_FRIENDS];

	NPAsync<NPGetProfileDataResult>* async = NP_GetProfileData(i, g_friends.friends, profiles);
	async->SetCallback(Friends_StatsCB, NULL);
}