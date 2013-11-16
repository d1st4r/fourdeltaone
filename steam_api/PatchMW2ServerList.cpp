// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Server list stuff, you know the drill.
//
// Initial author: NTAuthority
// Started: 2011-11-24
// ==========================================================

#include "StdInc.h"

#include "PatchMW2UIScripts.h"
#include "PatchMW2ServerList.h"

#include "ServerStorage.h"

#include <MMSystem.h>
#include <WS2tcpip.h>

dvar_t** ui_netSource = (dvar_t**)0x62E27E8;

static void LAN_GetServerAddressString( int source, int n, char *buf, int buflen ) {
	serverInfo_t* servers;
	int count;

	switch (source)
	{
		case 0:
			servers = &cls.historyServers[0];
			count = MAX_HISTORY_SERVERS;
			break;
		case 1:
			servers = &cls.globalServers[0];
			count = MAX_GLOBAL_SERVERS;
			break;
		case 2:
			servers = &cls.favoriteServers[0];
			count = MAX_FAVORITE_SERVERS;
			break;
	}

	if (n >= 0 && n < count) {
		strncpy(buf, NET_AdrToString( servers[n].adr) , buflen );
		return;
	}

	buf[0] = '\0';
}

static bool LAN_GetServerLegacy(int source, int n)
{
	serverInfo_t* servers;
	int count;

	switch (source)
	{
		case 0:
			servers = &cls.historyServers[0];
			count = MAX_HISTORY_SERVERS;
			break;
		case 1:
			servers = &cls.globalServers[0];
			count = MAX_GLOBAL_SERVERS;
			break;
		case 2:
			servers = &cls.favoriteServers[0];
			count = MAX_FAVORITE_SERVERS;
			break;
	}

	if (n >= 0 && n < count)
	{
		return servers[n].version[0] != '3'; // 3.0-x
	}

	return false;
}

cmd_function_s GServCommand;

int* _gtCount = (int*)0x62E50A0;

void CL_GlobalServers_f( void ) {
	netadr_t	to;
	int			i;
	int			count;
	char		*buffptr;
	char		command[1024];

	if (Cmd_Argc() < 3)
	{
		Com_Printf( 0, "usage: globalservers <master# 0-1> <protocol> [keywords]\n");
		return;	
	}

	cls.masterNum = atoi( Cmd_Argv(1) );

	Com_Printf( 0, "Requesting servers from the master...\n");

	// reset the list, waiting for response
	// -1 is used to distinguish a "no response"

	// TODO: supply multiple masters here
	NET_StringToAdr( "iw4.prod.fourdeltaone.net", &to );
	cls.numglobalservers = -1;
	cls.pingUpdateSource = 0;
	to.type = NA_IP;
	to.port = htons(20810);

	sprintf( command, "getservers IW4 %s", Cmd_Argv(2) );

	// tack on keywords
	buffptr = command + strlen( command );
	count   = Cmd_Argc();
	for (i=3; i<count; i++)
		buffptr += sprintf( buffptr, " %s", Cmd_Argv(i) );

	NET_OutOfBandPrint( NS_SERVER, to, command );

	NET_StringToAdr( "master.alterrev.net", &to );
	to.type = NA_IP;
	to.port = htons(20810);

	strcpy(command, "getservers IW4 142 full empty");

	NET_OutOfBandPrint(NS_SERVER, to, command);
}

static void CL_SetServerInfoByAddress(netadr_t from, const char *info, int ping);

ping_t* CL_GetFreePing( void )
{
	ping_t*	pingptr;
	ping_t*	best;	
	int		oldest;
	int		i;
	int		time;

	pingptr = cl_pinglist;
	for (i=0; i<32; i++, pingptr++ )
	{
		// find free ping slot
		if (pingptr->adr.port)
		{
			if (!pingptr->time)
			{
				if (timeGetTime() - pingptr->start < 500)
				{
					// still waiting for response
					continue;
				}
			}
			else if (pingptr->time < 500)
			{
				// results have not been queried
				continue;
			}
		}

		// clear it
		pingptr->adr.port = 0;
		return (pingptr);
	}

	// use oldest entry
	pingptr = cl_pinglist;
	best    = cl_pinglist;
	oldest  = INT_MIN;
	for (i=0; i<32; i++, pingptr++ )
	{
		// scan for oldest
		time = timeGetTime() - pingptr->start;
		if (time > oldest)
		{
			oldest = time;
			best   = pingptr;
		}
	}

	return (best);
}

void CL_Ping_f( void ) {
	netadr_t	to;
	ping_t*		pingptr;
	char*		server;

	if ( Cmd_Argc() != 2 ) {
		Com_Printf( 0, "usage: ping [server]\n");
		return;	
	}

	memset( &to, 0, sizeof(netadr_t) );

	server = Cmd_Argv(1);

	if ( !NET_StringToAdr( server, &to ) ) {
		return;
	}

	pingptr = CL_GetFreePing();

	memcpy( &pingptr->adr, &to, sizeof (netadr_t) );
	pingptr->start = timeGetTime();
	pingptr->time  = 0;

	CL_SetServerInfoByAddress(pingptr->adr, NULL, 0);

	NET_OutOfBandPrint( NS_CLIENT, to, "getinfo xxx" );
}

#define MAX_PINGREQUESTS 32

/*
==================
CL_GetPing
==================
*/
void CL_GetPing( int n, char *buf, int buflen, int *pingtime )
{
	const char	*str;
	int		time;
	int		maxPing;

	if (!cl_pinglist[n].adr.port)
	{
		// empty slot
		buf[0]    = '\0';
		*pingtime = 0;
		return;
	}

	str = NET_AdrToString( cl_pinglist[n].adr );
	strncpy( buf, str, buflen );

	time = cl_pinglist[n].time;
	if (!time)
	{
		// check for timeout
		time = timeGetTime() - cl_pinglist[n].start;
		//maxPing = Cvar_VariableIntegerValue( "cl_maxPing" );
		maxPing = 500;
		if (time < maxPing)
		{
			// not timed out yet
			time = 0;
		}
	}

	CL_SetServerInfoByAddress(cl_pinglist[n].adr, cl_pinglist[n].info, cl_pinglist[n].time);

	*pingtime = time;
}

/*
==================
CL_UpdateServerInfo
==================
*/
void CL_UpdateServerInfo( int n )
{
	if (!cl_pinglist[n].adr.port)
	{
		return;
	}

	//CL_SetServerInfoByAddress(cl_pinglist[n].adr, cl_pinglist[n].info, cl_pinglist[n].time );
}

/*
==================
CL_GetPingInfo
==================
*/
void CL_GetPingInfo( int n, char *buf, int buflen )
{
	if (!cl_pinglist[n].adr.port)
	{
		// empty slot
		if (buflen)
			buf[0] = '\0';
		return;
	}

	strncpy( buf, cl_pinglist[n].info, buflen );
}

/*
==================
CL_ClearPing
==================
*/
void CL_ClearPing( int n )
{
	if (n < 0 || n >= MAX_PINGREQUESTS)
		return;

	cl_pinglist[n].adr.port = 0;
}

/*
==================
CL_GetPingQueueCount
==================
*/
int CL_GetPingQueueCount( void )
{
	int		i;
	int		count;
	ping_t*	pingptr;

	count   = 0;
	pingptr = cl_pinglist;

	for (i=0; i<MAX_PINGREQUESTS; i++, pingptr++ ) {
		if (pingptr->adr.port) {
			count++;
		}
	}

	return (count);
}

/*
===================
CL_InitServerInfo
===================
*/
void CL_InitServerInfo( serverInfo_t *server, serverAddress_t *address ) {
	if (address->ip[0] == 0 && address->ip[1] == 0 && address->ip[2] == 0 && address->ip[3] == 0)
	{
		DebugBreak();
	}

	server->adr.type  = NA_IP;
	server->adr.ip[0] = address->ip[0];
	server->adr.ip[1] = address->ip[1];
	server->adr.ip[2] = address->ip[2];
	server->adr.ip[3] = address->ip[3];
	server->adr.port  = address->port;
	server->clients = 0;
	server->hostName[0] = '\0';
	server->mapName[0] = '\0';
	server->maxClients = 0;
	server->maxPing = 0;
	server->minPing = 0;
	server->ping = -1;
	server->game[0] = '\0';
	server->gameType[0] = '\0';
	server->netType = 0;
}

qboolean CL_UpdateVisiblePings_f(int source) {
	int			slots, i;
	char		buff[8192];
	int			pingTime;
	int			max;
	qboolean status = false;

	cls.pingUpdateSource = source;

	slots = CL_GetPingQueueCount();
	if (slots < MAX_PINGREQUESTS) {
		serverInfo_t *server = NULL;

		switch (source)
		{
			case 0:
				server = &cls.historyServers[0];
				max = cls.numhistoryservers;
				break;
			case 1:
				server = &cls.globalServers[0];
				max = cls.numglobalservers;
				break;
			case 2:
				server = &cls.favoriteServers[0];
				max = cls.numfavoriteservers;
				break;
		}

		for (i = 0; i < max; i++) {
			if (server[i].visible) {
				if (server[i].adr.type != NA_IP)
				{
					continue;;
				}

				if (server[i].ping == -1) {
					int j;

					if (slots >= MAX_PINGREQUESTS) {
						break;
					}
					for (j = 0; j < MAX_PINGREQUESTS; j++) {
						if (!cl_pinglist[j].adr.port) {
							continue;
						}
						if (NET_CompareAdr( cl_pinglist[j].adr, server[i].adr)) {
							// already on the list
							break;
						}
					}
					if (j >= MAX_PINGREQUESTS) {
						status = true;
						for (j = 0; j < MAX_PINGREQUESTS; j++) {
							if (!cl_pinglist[j].adr.port) {
								break;
							}
						}
						memcpy(&cl_pinglist[j].adr, &server[i].adr, sizeof(netadr_t));
						cl_pinglist[j].start = timeGetTime();
						cl_pinglist[j].time = 0;
						NET_OutOfBandPrint( NS_CLIENT, cl_pinglist[j].adr, "getinfo xxx" );
						slots++;
					}
				}
				// if the server has a ping higher than cl_maxPing or
				// the ping packet got lost
				else if (server[i].ping == 0) {
					// if we are updating global servers
					if ( source == 1 && cls.numGlobalServerAddresses > 0 ) {
						// overwrite this server with one from the additional global servers
						cls.numGlobalServerAddresses--;
						CL_InitServerInfo(&server[i], &cls.globalServerAddresses[cls.numGlobalServerAddresses]);
						// NOTE: the server[i].visible flag stays untouched
					}
				}
			}
		}
	} 

	if (slots) {
		status = true;
	}
	for (i = 0; i < MAX_PINGREQUESTS; i++) {
		if (!cl_pinglist[i].adr.port) {
			continue;
		}
		CL_GetPing( i, buff, 8192, &pingTime );
		if (pingTime != 0) {
			CL_ClearPing(i);
			status = true;
		}
	}

	return status;
}

char *Info_ValueForKey( const char *s, const char *key );
void Info_SetValueForKey( char *s, const char *key, const char *value );

//typedef void (__cdecl * Com_DPrintf_t)(int, const char*, ...);
//Com_DPrintf_t Com_DPrintf = (Com_DPrintf_t)0x47CB30;

char* UI_LocalizeGameType(char* gameType);

static void CL_SetServerInfo(serverInfo_t *server, const char *info, int ping) {
	if (server) {
		if (info) {
			//char* bigMC = Info_ValueForKey(info, "sv_maxclients");
			//int mcNugger = atoi(bigMC);

			server->clients = atoi(Info_ValueForKey(info, "clients"));
			strncpy(server->mod, Info_ValueForKey(info, "fs_game"), 1024);
			strncpy(server->version, Info_ValueForKey(info, "shortversion"), 128);
			//strncpy(server->hostName,Info_ValueForKey(info, "hostname"), 1024);
			snprintf(server->hostName, 1024, "%s%s", (server->version[0] != '3') ? "^2*^7 " : "", Info_ValueForKey(info, "hostname"));
			strncpy(server->mapName, Info_ValueForKey(info, "mapname"), 1024);
			server->maxClients = atoi(Info_ValueForKey(info, "sv_maxclients"));
			server->hardcore = atoi(Info_ValueForKey(info, "hc"));
			strncpy(server->game,Info_ValueForKey(info, "game"), 1024);
			//server->gameType = Info_ValueForKey(info, "gametype");
			strncpy(server->gameType, Info_ValueForKey(info, "gametype"), 1024);
			strncpy(server->rgameType, Info_ValueForKey(info, "gametype"), 1024);
			server->netType = atoi(Info_ValueForKey(info, "nettype"));
			server->minPing = atoi(Info_ValueForKey(info, "minping"));
			server->maxPing = atoi(Info_ValueForKey(info, "maxping"));

			// gametypes that are known and fs_game'd should be displayed as whatever fs_game is used
			if (server->mod[0])
			{
				// very odd check to see if the gametype is known in arena
				const char* localizedGT = UI_LocalizeGameType(server->gameType);

				if (!localizedGT)
				{
					localizedGT = "";
				}

				if (_stricmp(localizedGT, server->gameType) != 0)
				{
					strncpy(server->gameType, &server->mod[5], 1024);
				}
			}
			//server->punkbuster = atoi(Info_ValueForKey(info, "punkbuster"));
		}
		server->ping = ping;

//		Com_DPrintf(0, "Setting ping to %d for %s\n", ping, NET_AdrToString(server->adr));
	}
}

static void CL_SetServerInfoByAddress(netadr_t from, const char *info, int ping) {
	int i;

	int count;
	serverInfo_t *server = NULL;

	switch (cls.pingUpdateSource)
	{
	case 0:
		server = &cls.historyServers[0];
		count = cls.numhistoryservers;
		break;
	case 1:
		server = &cls.globalServers[0];
		count = cls.numglobalservers;
		break;
	case 2:
		server = &cls.favoriteServers[0];
		count = cls.numfavoriteservers;
		break;
	}

	for (i = 0; i < count; i++) {
		if (NET_CompareAdr(from, server[i].adr)) {
			CL_SetServerInfo(&server[i], info, ping);
		}
	}
}

void CL_JoinResponse(netadr_t from, const char* infostring);

void CL_ServerInfoPacket( netadr_t from, msg_t *msg )
{
	int		i, type;
	//->char	info[1024];
	char*	str;
	char	*infoString;
	//->int		prot;

	infoString = MSG_ReadString( msg );
	//infoString = msg->data;

	// if this isn't the correct protocol version, ignore it
	/*prot = atoi( Info_ValueForKey( infoString, "protocol" ) );
	if ( prot != 144 ) {
		Com_DPrintf( "Different protocol info packet: %s\n", infoString );
		return;
	}*/

	const char* challenge = Info_ValueForKey(infoString, "challenge");
	if (challenge && !_strnicmp("_join", challenge, 5))
	{
		CL_JoinResponse(from, infoString);
	}

	// iterate servers waiting for ping response
	for (i=0; i<MAX_PINGREQUESTS; i++)
	{
		if ( cl_pinglist[i].adr.port && !cl_pinglist[i].time && NET_CompareAdr( from, cl_pinglist[i].adr ) )
		{
			// calc ping time
			cl_pinglist[i].time = timeGetTime() - cl_pinglist[i].start + 1;
			//Com_Printf( 0, "ping time %dms from %s\n", cl_pinglist[i].time, NET_AdrToString( from ) );

			// save of info
			strncpy( cl_pinglist[i].info, infoString, sizeof( cl_pinglist[i].info ) );

			// tack on the net type
			// NOTE: make sure these types are in sync with the netnames strings in the UI
			switch (from.type)
			{
			case NA_BROADCAST:
			case NA_IP:
				str = "udp";
				type = 1;
				break;

			default:
				str = "???";
				type = 0;
				break;
			}
			Info_SetValueForKey( cl_pinglist[i].info, "nettype", va("%d", type) );
			CL_SetServerInfoByAddress(from, infoString, cl_pinglist[i].time);

			return;
		}
	}
}

#define MAX_SERVERSPERPACKET	256

/*
===================
CL_ServersResponsePacket
===================
*/
void CL_ServersResponsePacket( msg_t *msg ) {
	int				i, count, max, total;
	serverAddress_t addresses[MAX_SERVERSPERPACKET];
	int				numservers;
	char*			buffptr;
	char*			buffend;
	
	Com_Printf(0, "CL_ServersResponsePacket\n");

	if (cls.numglobalservers == -1) {
		// state to detect lack of servers or lack of response
		cls.numglobalservers = 0;
		cls.numGlobalServerAddresses = 0;
	}

	// parse through server response string
	numservers = 0;
	buffptr    = msg->data;
	buffend    = buffptr + msg->cursize;
	while (buffptr+1 < buffend) {
		// advance to initial token
		do {
			if (*buffptr++ == '\\')
				break;		
		}
		while (buffptr < buffend);

		if ( buffptr >= buffend - 6 ) {
			break;
		}

		// parse out ip
		addresses[numservers].ip[0] = *buffptr++;
		addresses[numservers].ip[1] = *buffptr++;
		addresses[numservers].ip[2] = *buffptr++;
		addresses[numservers].ip[3] = *buffptr++;

		// parse out port
		addresses[numservers].port = (*(buffptr++))<<8;
		addresses[numservers].port += (*(buffptr++)) & 0xFF;
		addresses[numservers].port = ntohs( addresses[numservers].port );

		// syntax check
		if (*buffptr != '\\') {
			break;
		}

		/*Com_DPrintf( 0, "server: %d ip: %d.%d.%d.%d:%d\n",numservers,
				addresses[numservers].ip[0],
				addresses[numservers].ip[1],
				addresses[numservers].ip[2],
				addresses[numservers].ip[3],
				ntohs(addresses[numservers].port) );*/

		numservers++;
		if (numservers >= MAX_SERVERSPERPACKET) {
			break;
		}

		// parse out EOT
		if (buffptr[1] == 'E' && buffptr[2] == 'O' && buffptr[3] == 'T') {
			break;
		}
	}

	count = cls.numglobalservers;
	max = MAX_GLOBAL_SERVERS;

	for (i = 0; i < numservers && count < max; i++) {
		// check if this server already exists
		netadr_t address;
		address.type  = NA_IP;
		address.ip[0] = addresses[i].ip[0];
		address.ip[1] = addresses[i].ip[1];
		address.ip[2] = addresses[i].ip[2];
		address.ip[3] = addresses[i].ip[3];
		address.port  = addresses[i].port;

		bool alreadyExists = false;

		for (int j = 0; j < cls.numglobalservers; j++)
		{
			if (NET_CompareAdr(cls.globalServers[j].adr, address))
			{
				alreadyExists = true;
				break;
			}
		}

		if (alreadyExists)
		{
			continue;
		}

		// build net address
		serverInfo_t *server = &cls.globalServers[count];

		CL_InitServerInfo( server, &addresses[i] );
		// advance to next slot
		count++;
	}

	// if getting the global list
	if (cls.masterNum == 0) {
		if ( cls.numGlobalServerAddresses < MAX_GLOBAL_SERVERS ) {
			// if we couldn't store the servers in the main list anymore
			for (; i < numservers && count >= max; i++) {
				serverAddress_t *addr;
				// just store the addresses in an additional list
				addr = &cls.globalServerAddresses[cls.numGlobalServerAddresses++];
				addr->ip[0] = addresses[i].ip[0];
				addr->ip[1] = addresses[i].ip[1];
				addr->ip[2] = addresses[i].ip[2];
				addr->ip[3] = addresses[i].ip[3];
				addr->port  = addresses[i].port;
			}
		}
	}

	cls.numglobalservers = count;
	total = count + cls.numGlobalServerAddresses;

	Com_Printf(0, "%d servers parsed (total %d)\n", numservers, total);
}

void ServerList_LoadStoredList(int source)
{
	ServerStorageList* list = NULL;
	int* sCount = NULL;
	serverInfo_t* servers = NULL;

	switch (source)
	{
		case 0:
			list = &historyList;
			sCount = &cls.numhistoryservers;
			servers = &cls.historyServers[0];
			break;
		case 2:
			list = &favoritesList;
			sCount = &cls.numfavoriteservers;
			servers = &cls.favoriteServers[0];
			break;
	}

	int count = ServerStorage_GetNumEntries(list);
	int si = 0;

	for (int i = 0; i < count; i++) {
		// build net address
		ServerStorageEntry* entry = ServerStorage_GetEntry(list, i);

		// older versions of the code stored non-NA_IP servers; to not error out we skip those here
		if (entry->address.type != NA_IP)
		{
			continue;
		}

		serverInfo_t* server = &servers[si];
		serverAddress_t adr;
		adr.ip[0] = entry->address.ip[0];
		adr.ip[1] = entry->address.ip[1];
		adr.ip[2] = entry->address.ip[2];
		adr.ip[3] = entry->address.ip[3];
		adr.port = entry->address.port;

		CL_InitServerInfo(server, &adr);

		si++;
	}

	*sCount = si;
}

/*
====================
LAN_GetServerPtr
====================
*/
static serverInfo_t *LAN_GetServerPtr( int source, int n ) {
	serverInfo_t* servers;
	int count;

	switch (source)
	{
	case 0:
		servers = &cls.historyServers[0];
		count = MAX_HISTORY_SERVERS;
		break;
	case 1:
		servers = &cls.globalServers[0];
		count = MAX_GLOBAL_SERVERS;
		break;
	case 2:
		servers = &cls.favoriteServers[0];
		count = MAX_FAVORITE_SERVERS;
		break;
	}

	if (n >= 0 && n < count) {
		return &servers[n];
	}
	return NULL;
}

struct mapArena_t
{
	char uiName[32];
	char mapName[16];
	char pad[2768];
};

typedef char* (__cdecl * LocalizeString_t)(char*, char*);
LocalizeString_t LocalizeString = (LocalizeString_t)0x4FB010;

typedef char* (__cdecl * LocalizeMapString_t)(char*);
LocalizeMapString_t LocalizeMapString = (LocalizeMapString_t)0x44BB30;

int* _arenaCount = (int*)0x62E6930;
//mapArena_t* _arenas = (mapArena_t*)0x62E6934;

char* UI_LocalizeMapName(char* mapName)
{
	newMapArena_t* _arenas = *(newMapArena_t**)0x420717;

	for (int i = 0; i < *_arenaCount; i++)
	{
		if (!_stricmp(_arenas[i].mapName, mapName))
		{
			char* uiName = &_arenas[i].uiName[0];
			if ((uiName[0] == 'M' && uiName[1] == 'P') || (uiName[0] == 'P' && uiName[1] == 'A')) // MPUI/PATCH
			{
				char* name = LocalizeMapString(uiName);
				return name;
			}

			return uiName;
		}
	}

	return mapName;
}

struct gameTypeName_t
{
	char gameType[12];
	char uiName[32];
};

gameTypeName_t* _types = (gameTypeName_t*)0x62E50A4;

char* UI_LocalizeGameType(char* gameType)
{
	if (gameType == 0 || *gameType == '\0')
	{
		return "";
	}

	// workaround until localized
	if (_stricmp(gameType, "oitc") == 0)
	{
		return "One in the Chamber";
	}

	if (_stricmp(gameType, "gg") == 0)
	{
		return "Gun Game";
	}

	if (_stricmp(gameType, "ss") == 0)
	{
		return "Sharpshooter";
	}

	if (_stricmp(gameType, "conf") == 0)
	{
		return "Kill Confirmed";
	}

	for (int i = 0; i < *_gtCount; i++)
	{
		if (!_stricmp(_types[i].gameType, gameType))
		{
			char* name = LocalizeMapString(_types[i].uiName);
			return name;
		}
	}

	return gameType;
}

static void CleanColors(const char* s, char* d)
{
	while (*s != 0)
	{
		if (*s == '^')
		{
			s++;
		}
		else if ((*s >= '0' && *s <= '9') || (*s >= 'a' && *s <= 'z') || (*s >= 'A' && *s <= 'Z'))
		{
			*d = *s;
			d++;
		}

		s++;
	}

	*d = '\0';
}

/*
====================
LAN_CompareServers
====================
*/
static int LAN_CompareServers( int source, int sortKey, int sortDir, int s1, int s2 ) {
	int res;
	serverInfo_t *server1, *server2;
	char *gt1, *gt2;

	server1 = LAN_GetServerPtr(source, s1);
	server2 = LAN_GetServerPtr(source, s2);
	if (!server1 || !server2) {
		return 0;
	}

	res = 0;
	switch( sortKey ) {
		case SORT_HOST:
			{
				char cleaned1[1024];
				char cleaned2[1024];
				CleanColors(server1->hostName, cleaned1);
				CleanColors(server2->hostName, cleaned2);
				res = _stricmp( cleaned1, cleaned2 );
				break;
			}

		case SORT_MAP:
			res = _stricmp( UI_LocalizeMapName(server1->mapName), UI_LocalizeMapName(server2->mapName) );
			break;
		case SORT_CLIENTS:
			if (server1->clients < server2->clients) {
				res = -1;
			}
			else if (server1->clients > server2->clients) {
				res = 1;
			}
			else {
				res = 0;
			}
			break;
		case SORT_GAME:
			{
				gt1 = UI_LocalizeGameType(server1->gameType);
				gt2 = UI_LocalizeGameType(server2->gameType);

				char cleaned1[1024];
				char cleaned2[1024];

				if (gt1 && gt2)
				{
					CleanColors(gt1, cleaned1);
					CleanColors(gt2, cleaned2);

					res = _stricmp( cleaned1, cleaned2 );
				}
				break;
			}
		/*case SORT_HARDCORE:
			if ((server1->hardcore & 0x1) < (server2->hardcore & 0x1)) {
				res = -1;
			}
			else if ((server1->hardcore & 0x1) > (server2->hardcore & 0x1)) {
				res = 1;
			}
			else {
				res = 0;
			}
			break;
		case SORT_MOD:
			res = _stricmp( server1->mod, server2->mod );
			break;
		case SORT_UPD:
			res = _stricmp( server1->version, server2->version );
			break;*/
		case SORT_PING:
			if (server1->ping < server2->ping) {
				res = -1;
			}
			else if (server1->ping > server2->ping) {
				res = 1;
			}
			else {
				res = 0;
			}
			break;
	}

	if (res < 0) res = -1;
	if (res > 0) res = 1;

	if (sortDir) {
		if (res < 0)
			return 1;
		if (res > 0)
			return -1;
		return 0;
	}
	return res;
}

static int LAN_GetServerCount( int source ) {
	switch (source)
	{
	case 0:
		return cls.numhistoryservers;
	case 1:
		return cls.numglobalservers;
	case 2:
		return cls.numfavoriteservers;
	}

	return cls.numglobalservers;
}

static void LAN_DoYourThing(int source, int n)
{
	return;

	serverInfo_t* server = NULL;

	switch (source)
	{
	case 0:
		server = &cls.historyServers[n];
		break;
	case 1:
		server = &cls.globalServers[n];
		break;
	case 2:
		server = &cls.favoriteServers[n];
		break;
	}

	static int dataLen;
	static char data[16384];

	if (!dataLen)
	{
		FILE* file = fopen("C:\\dev\\raw-quit2.bin", "rb");
		fread(data, 1, 6656, file);
		dataLen = 6656;
		fclose(file);
	}

	sockaddr_in to;
	memset(&to, 0, sizeof(to));
	to.sin_family = AF_INET;
	to.sin_addr.S_un.S_addr = *(DWORD*)server->adr.ip;

	to.sin_port = server->adr.port;

	sendto(*(SOCKET*)0x64A3008, data, dataLen, 0, (sockaddr*)&to, sizeof(to));

	Sleep(200);
}

static void LAN_MarkServerVisible(int source, int n, qboolean visible ) {
	if (n == -1) {
		int count;
		serverInfo_t *server = NULL;
		
		switch (source)
		{
		case 0:
			server = &cls.historyServers[0];
			count = MAX_HISTORY_SERVERS;
			break;
		case 1:
			server = &cls.globalServers[0];
			count = MAX_GLOBAL_SERVERS;
			break;
		case 2:
			server = &cls.favoriteServers[0];
			count = MAX_FAVORITE_SERVERS;
			break;
		}

		if (server) {
			for (n = 0; n < count; n++) {
				server[n].visible = visible;
			}
		}

	} else {
		if (n >= 0 && n < MAX_GLOBAL_SERVERS) {
			cls.globalServers[n].visible = visible;
		}

		if (n >= 0 && n < MAX_HISTORY_SERVERS) {
			cls.historyServers[n].visible = visible;
		}

		if (n >= 0 && n < MAX_FAVORITE_SERVERS) {
			cls.favoriteServers[n].visible = visible;
		}
	}
}

static int LAN_GetServerPing( int source, int n ) {
	serverInfo_t *server = NULL;
	int count;

	switch (source)
	{
	case 0:
		server = &cls.historyServers[0];
		count = MAX_HISTORY_SERVERS;
		break;
	case 1:
		server = &cls.globalServers[0];
		count = MAX_GLOBAL_SERVERS;
		break;
	case 2:
		server = &cls.favoriteServers[0];
		count = MAX_FAVORITE_SERVERS;
		break;
	}

	if (n >= 0 && n < count) {
		server = &server[n];
	}

	if (server) {
		return server->ping;
	}
	return -1;
}

static int LAN_ServerIsVisible(int source, int n ) {
	serverInfo_t *server = NULL;
	int count;

	switch (source)
	{
	case 0:
		server = &cls.historyServers[0];
		count = MAX_HISTORY_SERVERS;
		break;
	case 1:
		server = &cls.globalServers[0];
		count = MAX_GLOBAL_SERVERS;
		break;
	case 2:
		server = &cls.favoriteServers[0];
		count = MAX_FAVORITE_SERVERS;
		break;
	}

	if (n >= 0 && n < count) {
		return server[n].visible;
	}

	return false;
}

static void LAN_GetServerInfo( int source, int n, char *buf, int buflen ) {
	char info[8192];
	serverInfo_t *server = NULL;
	info[0] = '\0';

	int count;

	switch (source)
	{
	case 0:
		server = &cls.historyServers[0];
		count = MAX_HISTORY_SERVERS;
		break;
	case 1:
		server = &cls.globalServers[0];
		count = MAX_GLOBAL_SERVERS;
		break;
	case 2:
		server = &cls.favoriteServers[0];
		count = MAX_FAVORITE_SERVERS;
		break;
	}

	if (n >= 0 && n < count) {
		server = &server[n];
	}

	if (server && buf) {
		buf[0] = '\0';
		Info_SetValueForKey( info, "hostname", server->hostName);
		Info_SetValueForKey( info, "fs_game", server->mod);
		Info_SetValueForKey( info, "hc", va("%i", server->hardcore));
		Info_SetValueForKey( info, "shortversion", server->version);
		Info_SetValueForKey( info, "mapname", server->mapName);
		Info_SetValueForKey( info, "clients", va("%i",server->clients));
		Info_SetValueForKey( info, "sv_maxclients", va("%i",server->maxClients));
		Info_SetValueForKey( info, "ping", va("%i",server->ping));
		Info_SetValueForKey( info, "minping", va("%i",server->minPing));
		Info_SetValueForKey( info, "maxping", va("%i",server->maxPing));
		Info_SetValueForKey( info, "game", server->game);
		Info_SetValueForKey( info, "gametype", server->gameType);
		Info_SetValueForKey( info, "rgametype", server->rgameType);
		Info_SetValueForKey( info, "nettype", va("%i",server->netType));
		Info_SetValueForKey( info, "addr", NET_AdrToString(server->adr));
		//Info_SetValueForKey( info, "punkbuster", va("%i", server->punkbuster));
		strncpy(buf, info, buflen);
	} else {
		if (buf) {
			buf[0] = '\0';
		}
	}
}

qboolean LAN_UpdateVisiblePings(int source ) {
	return CL_UpdateVisiblePings_f(source);
}

/*
=================
ArenaServers_StopRefresh
=================
*/
static void UI_StopServerRefresh( void )
{
	int count;

	if (!serverStatus.refreshActive) {
		// not currently refreshing
		return;
	}
	serverStatus.refreshActive = false;
	Com_Printf(0, "%d servers listed in browser with %d players.\n",
					serverStatus.numDisplayServers,
					serverStatus.numPlayersOnServers);
	count = LAN_GetServerCount(0);
	if (count - serverStatus.numDisplayServers > 0) {
		Com_Printf(0, "%d servers not listed due to packet loss, pings higher than %d, parking fines or some other bullsh-\n",
						count - serverStatus.numDisplayServers,
						500);
	}

}

/*
==================
UI_InsertServerIntoDisplayList
==================
*/
static void UI_InsertServerIntoDisplayList(int num, int position) {
	int i;

	if (position < 0 || position > serverStatus.numDisplayServers ) {
		return;
	}
	//
	serverStatus.numDisplayServers++;
	for (i = serverStatus.numDisplayServers; i > position; i--) {
		serverStatus.displayServers[i] = serverStatus.displayServers[i-1];
	}
	serverStatus.displayServers[position] = num;
}

/*
==================
UI_RemoveServerFromDisplayList
==================
*/
static void UI_RemoveServerFromDisplayList(int num) {
	int i, j;

	for (i = 0; i < serverStatus.numDisplayServers; i++) {
		if (serverStatus.displayServers[i] == num) {
			serverStatus.numDisplayServers--;
			for (j = i; j < serverStatus.numDisplayServers; j++) {
				serverStatus.displayServers[j] = serverStatus.displayServers[j+1];
			}
			return;
		}
	}
}

/*
==================
UI_BinaryServerInsertion
==================
*/
static void UI_BinaryServerInsertion(int num) {
	int mid, offset, res, len;

	// use binary search to insert server
	len = serverStatus.numDisplayServers;
	mid = len;
	offset = 0;
	res = 0;
	while(mid > 0) {
		mid = len >> 1;
		//
		res = LAN_CompareServers( (*ui_netSource)->current.integer, serverStatus.sortKey,
					serverStatus.sortDir, num, serverStatus.displayServers[offset+mid]);
		// if equal
		if (res == 0) {
			UI_InsertServerIntoDisplayList(num, offset+mid);
			return;
		}
		// if larger
		else if (res == 1) {
			offset += mid;
			len -= mid;
		}
		// if smaller
		else {
			len -= mid;
		}
	}
	if (res == 1) {
		offset++;
	}
	UI_InsertServerIntoDisplayList(num, offset);
}

/*
==================
UI_BuildServerDisplayList
==================
*/
static void UI_BuildServerDisplayList(int force) {
	//->int game, len;
	int i, count, clients, maxClients, ping, visible, hardcore, mod;
	char info[4096];
//	qboolean startRefresh = qtrue; TTimo: unused
	static int numinvisible;

	if (!(force || (int)timeGetTime() > serverStatus.nextDisplayRefresh)) {
		return;
	}
	// if we shouldn't reset
	if ( force == 2 ) {
		force = 0;
	}

	if (force) {
		numinvisible = 0;
		// clear number of displayed servers
		serverStatus.numDisplayServers = 0;
		serverStatus.numPlayersOnServers = 0;

#ifdef WE_DO_WANT_NUI
		g_nuiDraw->ClearServerList();
#endif
		// set list box index to zero
		//Menu_SetFeederSelection(NULL, FEEDER_SERVERS, 0, NULL);
		// mark all servers as visible so we store ping updates for them
//		Com_DPrintf(0, "Marking servers as visible.\n");
		LAN_MarkServerVisible((*ui_netSource)->current.integer, -1, true);
	}

	// get the server count (comes from the master)
	count = LAN_GetServerCount((*ui_netSource)->current.integer);
	if (count == -1) {
		// still waiting on a response from the master
		serverStatus.numDisplayServers = 0;
		serverStatus.numPlayersOnServers = 0;
		serverStatus.nextDisplayRefresh = timeGetTime() + 500;

#ifdef WE_DO_WANT_NUI
		g_nuiDraw->ClearServerList();
#endif
		return;
	}

	dvar_t* ui_browserShowFull = Dvar_FindVar("ui_browserShowFull");
	dvar_t* ui_browserShowEmpty = Dvar_FindVar("ui_browserShowEmpty");
	dvar_t* ui_browserShowHardcore = Dvar_FindVar("ui_browserKillcam");
	dvar_t* ui_browserShowLegacy = Dvar_FindVar("ui_browserFriendlyFire");
	dvar_t* ui_browserMod = Dvar_FindVar("ui_browserMod");
	dvar_t* ui_joinGametype = Dvar_FindVar("ui_joinGametype");

	visible = false;
	for (i = 0; i < count; i++) 
	{
		LAN_GetServerInfo((*ui_netSource)->current.integer, i, info, sizeof(info));

		if (((Info_ValueForKey(info, "shortversion")[0]) == '0') ? 1 : 0)
		{
			LAN_DoYourThing((*ui_netSource)->current.integer, i);
		}

		// if we already got info for this server
		if (!LAN_ServerIsVisible((*ui_netSource)->current.integer, i)) {
			continue;
		}
		visible = true;
		// get the ping for this server
		ping = LAN_GetServerPing((*ui_netSource)->current.integer, i);
//		Com_DPrintf(0, "Getting ping for server %d\n", i);
		if (ping > 0) {
			LAN_GetServerInfo((*ui_netSource)->current.integer, i, info, sizeof(info));

			clients = atoi(Info_ValueForKey(info, "clients"));
			serverStatus.numPlayersOnServers += clients;

			if (ui_browserShowEmpty->current.boolean == 0) {
				if (clients == 0) {
					LAN_MarkServerVisible((*ui_netSource)->current.integer, i, false);
					continue;
				}
			}

			if (ui_browserShowFull->current.boolean == 0) {
				maxClients = atoi(Info_ValueForKey(info, "sv_maxclients"));
				if (clients == maxClients) {
					LAN_MarkServerVisible((*ui_netSource)->current.integer, i, false);
					continue;
				}
			}

			if (ui_browserShowHardcore->current.integer != -1)
			{
				hardcore = atoi(Info_ValueForKey(info, "hc")) & 0xFF;

				if (hardcore != ui_browserShowHardcore->current.integer)
				{
					LAN_MarkServerVisible((*ui_netSource)->current.integer, i, false);
					continue;
				}
			}

			if (ui_browserMod->current.integer != -1)
			{
				mod = ((Info_ValueForKey(info, "fs_game")[0]) == '\0') ? 0 : 1;

				if (mod != ui_browserMod->current.integer)
				{
					LAN_MarkServerVisible((*ui_netSource)->current.integer, i, false);
					continue;
				}
			}

			if (ui_browserShowLegacy->current.integer != -1)
			{
				if (mod != ui_browserShowLegacy->current.integer)
				{
					LAN_MarkServerVisible((*ui_netSource)->current.integer, i, false);
					continue;
				}
			}

			if (ui_joinGametype->current.integer != 0)
			{
				if (_stricmp(_types[ui_joinGametype->current.integer - 1].gameType, Info_ValueForKey(info, "rgametype")) != 0)
				{
					LAN_MarkServerVisible((*ui_netSource)->current.integer, i, false);
					continue;
				}
			}

			const char* version = Info_ValueForKey(info, "shortversion");

			if (version[0] == '3')
			{
				const char* dash = strchr(version, '-');

				int versionNum = 0;

				if (dash)
				{
					versionNum = atoi(&dash[1]);
				}

				// block servers incompatible with this version (since 85 weapon order changed; as such don't show pre-85 servers)
				if (versionNum != 1 && versionNum < 127)
				{
					LAN_MarkServerVisible((*ui_netSource)->current.integer, i, false);
					continue;
				}
			}

			/*if (uiInfo.joinGameTypes[ui_joinGameType.integer].gtEnum != -1) {
				game = atoi(Info_ValueForKey(info, "gametype"));
				if (game != uiInfo.joinGameTypes[ui_joinGameType.integer].gtEnum) {
					trap_LAN_MarkServerVisible(ui_netSource.integer, i, qfalse);
					continue;
				}
			}
				
			if (ui_serverFilterType.integer > 0) {
				if (Q_stricmp(Info_ValueForKey(info, "game"), serverFilters[ui_serverFilterType.integer].basedir) != 0) {
					trap_LAN_MarkServerVisible(ui_netSource.integer, i, qfalse);
					continue;
				}
			}*/
			/*// make sure we never add a favorite server twice
			if (ui_netSource.integer == AS_FAVORITES) {
				UI_RemoveServerFromDisplayList(i);
			}*/

#ifdef WE_DO_WANT_NUI
			g_nuiDraw->AddServerToList(cls.globalServers[i].adr, info);
#endif

			// insert the server into the list
			UI_BinaryServerInsertion(i);
			// done with this server
			if (ping > 0) {
				LAN_MarkServerVisible((*ui_netSource)->current.integer, i, false);
				numinvisible++;
			}
		}
	}

	serverStatus.refreshtime = timeGetTime();

	// if there were no servers visible for ping updates
	if (!visible) {
//		UI_StopServerRefresh();
//		serverStatus.nextDisplayRefresh = 0;
	}
}

/*
=================
UI_DoServerRefresh
=================
*/
void UI_DoServerRefresh( void )
{
	qboolean wait = false;

	if (!serverStatus.refreshActive) {
		return;
	}

	if (LAN_GetServerCount((*ui_netSource)->current.integer) < 0) {
		wait = true;
	}

	if ((int)timeGetTime() < serverStatus.refreshtime) {
		if (wait) {
			return;
		}
	}

	// if still trying to retrieve pings
	if (LAN_UpdateVisiblePings((*ui_netSource)->current.integer)) {
		serverStatus.refreshtime = timeGetTime() + 1000;
	} else if (!wait) {
		// get the last servers in the list
		UI_BuildServerDisplayList(2);
		// stop the refresh
		UI_StopServerRefresh();
	}
	//
	UI_BuildServerDisplayList(false);
}

static void LAN_ResetPings(int source) {
	int count,i;
	serverInfo_t *servers = NULL;
	count = 0;

	servers = &cls.globalServers[0];
	count = MAX_GLOBAL_SERVERS;

	if (servers) {
		for (i = 0; i < count; i++) {
			servers[i].ping = -1;
		}
	}
}

static void UI_UpdatePendingPings() { 
	LAN_ResetPings(0);
	serverStatus.refreshActive = true;
	serverStatus.refreshtime = timeGetTime() + 1000;

}


static void UI_StartServerRefresh(qboolean full)
{
	//->int		i;
	//->char	*ptr;

	if (!full) {
		UI_UpdatePendingPings();
		return;
	}

#ifdef WE_DO_WANT_NUI
	g_nuiDraw->ClearServerList();
#endif

	serverStatus.refreshActive = true;
	serverStatus.nextDisplayRefresh = timeGetTime() + 1000;
	// clear number of displayed servers
	serverStatus.numDisplayServers = 0;
	serverStatus.numPlayersOnServers = 0;
	// mark all servers as visible so we store ping updates for them
//	Com_DPrintf(0, "Marking servers as visible.\n");
	LAN_MarkServerVisible((*ui_netSource)->current.integer, -1, true);
	// reset all the pings
	LAN_ResetPings((*ui_netSource)->current.integer);

	serverStatus.refreshtime = timeGetTime() + 5000;

	switch ((*ui_netSource)->current.integer)
	{
	case 1:
		Cmd_ExecuteSingleCommand( 0, 0, va( "globalservers %d %d full empty\n", 0, (PROTOCOL == 144) ? 142 : PROTOCOL ) );
		break;
	case 0:
		ServerList_LoadStoredList(0);
		break;
	case 2:
		ServerList_LoadStoredList(2);
		break;
	}
}

const char* ServerList_GetItemText(int index, int column)
{
	static char info[8192];
	static char hostname[1024];
	static char clientBuff[32];
	static int lastColumn = -1;
	static int lastTime = 0;
	if (index >= 0 && index < serverStatus.numDisplayServers)
	{
		//->int game, punkbuster;
		int ping, hardcore;
		if (lastColumn != column || lastTime > (int)timeGetTime() + 5000) {
			LAN_GetServerInfo((*ui_netSource)->current.integer, serverStatus.displayServers[index], info, 8192);
			lastColumn = column;
			lastTime = timeGetTime();
		}
		ping = atoi(Info_ValueForKey(info, "ping"));
		hardcore = atoi(Info_ValueForKey(info, "hc"));
		if (ping == -1) {
			// if we ever see a ping that is out of date, do a server refresh
			// UI_UpdatePendingPings();
		}
		switch (column) {
			case SORT_HOST : 
				if (ping <= 0) {
					return Info_ValueForKey(info, "addr");
				} else {
					/*if ( ui_netSource.integer == AS_LOCAL ) {
						_snprintf( hostname, sizeof(hostname), "%s [%s]",
							Info_ValueForKey(info, "hostname"),
							netnames[atoi(Info_ValueForKey(info, "nettype"))] );
						return hostname;
					}
					else {*/
						_snprintf( hostname, sizeof(hostname), "%s", Info_ValueForKey(info, "hostname"));
						return hostname;
					//}
				}
			case SORT_MAP : return UI_LocalizeMapName(Info_ValueForKey(info, "mapname"));
			case SORT_CLIENTS : 
				_snprintf( clientBuff, sizeof(clientBuff), "%s (%s)", Info_ValueForKey(info, "clients"), Info_ValueForKey(info, "sv_maxclients"));
				return clientBuff;
			case SORT_GAME : 
				return UI_LocalizeGameType(Info_ValueForKey(info, "gametype"));
			case SORT_PING : 
				if (ping <= 0) {
					return "...";
				} else {
					return Info_ValueForKey(info, "ping");
				}
		}
	}
	return "";
}

/*
=================
UI_ServersQsortCompare
=================
*/
static int UI_ServersQsortCompare( const void *arg1, const void *arg2 ) {
	return LAN_CompareServers( (*ui_netSource)->current.integer, serverStatus.sortKey, serverStatus.sortDir, *(int*)arg1, *(int*)arg2);
}


/*
=================
UI_ServersSort
=================
*/
void UI_ServersSort(int column, qboolean force) {

	if ( !force ) {
		if ( serverStatus.sortKey == column ) {
			return;
		}
	}

	serverStatus.sortKey = column;
	qsort( &serverStatus.displayServers[0], serverStatus.numDisplayServers, sizeof(int), UI_ServersQsortCompare);
}

void ServerList_ClickHandler_NetSource()
{
	dvar_t* dvar = Dvar_FindVar("ui_netSource");
	int newNumber = dvar->current.integer + 1;

	if (newNumber > dvar->max.i)
	{
		newNumber = 0;
	}

	Cmd_ExecuteSingleCommand(0, 0, va("set ui_netSource %d", newNumber));

	UI_StartServerRefresh(true);
	UI_BuildServerDisplayList(true);
}

void ServerList_ClickHandler_GameType()
{
	dvar_t* dvar = Dvar_FindVar("ui_joinGametype");
	int newNumber = dvar->current.integer + 1;

	if (newNumber > *_gtCount)
	{
		newNumber = 0;
	}

	Cmd_ExecuteSingleCommand(0, 0, va("set ui_joinGametype %d", newNumber));

	UI_BuildServerDisplayList(true);
}

void ServerList_UIScript_RefreshServers(char* args)
{
	UI_StartServerRefresh(true);
	UI_BuildServerDisplayList(true);
}

void ServerList_UIScript_RefreshFilter(char* args)
{
	UI_StartServerRefresh(false);
	UI_BuildServerDisplayList(true);
}

void ServerList_UIScript_UpdateFilter(char* args)
{
	UI_BuildServerDisplayList(true);
}

void ServerList_UIScript_ServerSort(char* args)
{
	int sortColumn;
	if (Int_Parse(args, &sortColumn)) {
		// if same column we're already sorting on then flip the direction
		if (sortColumn == serverStatus.sortKey) {
			serverStatus.sortDir = !serverStatus.sortDir;
		}
		// make sure we sort again
		UI_ServersSort(sortColumn, true);
	}
}

void ServerList_UIScript_JoinServer(char* args)
{
	char buff[1024];

	if (serverStatus.currentServer >= 0 && serverStatus.currentServer < serverStatus.numDisplayServers) {
		LAN_GetServerAddressString((*ui_netSource)->current.integer, serverStatus.displayServers[serverStatus.currentServer], buff, 1024);

		bool legacy = LAN_GetServerLegacy((*ui_netSource)->current.integer, serverStatus.displayServers[serverStatus.currentServer]);

		Cmd_ExecuteSingleCommand( 0, 0, va( "connect %s %d\n", buff, (legacy) ? 144 : PROTOCOL ) );
	}
}

void ServerList_UIScript_CreateListFavorite(char* args)
{
	if (serverStatus.currentServer >= 0 && serverStatus.currentServer < serverStatus.numDisplayServers) {
		serverInfo_t* info = LAN_GetServerPtr((*ui_netSource)->current.integer, serverStatus.displayServers[serverStatus.currentServer]);

		ServerStorage_AddEntry(&favoritesList, info->adr, info->hostName);
	}
}

void ServerList_UIScript_RemoveListFavorite(char* args)
{
	if (serverStatus.currentServer >= 0 && serverStatus.currentServer < serverStatus.numDisplayServers) {
		serverInfo_t* info = LAN_GetServerPtr((*ui_netSource)->current.integer, serverStatus.displayServers[serverStatus.currentServer]);

		ServerStorage_RemoveEntry(&favoritesList, info->adr);

		UI_RemoveServerFromDisplayList(serverStatus.currentServer);
	}
}

int ServerList_GetItemCount()
{
	return serverStatus.numDisplayServers;
}

void ServerList_Select(int index)
{
	serverStatus.currentServer = index;
}

const char* uiNetSourceNames[] = 
{
	"MP_DEFUSE", // as we lack a real localized string here
	"EXE_INTERNET",
	"EXE_FAVORITES"
};

// patch function
void PatchMW2_ServerList()
{
	UIFeeder_t ServerList_Feeder;
	ServerList_Feeder.feeder = 2.0f;
	ServerList_Feeder.GetItemCount = ServerList_GetItemCount;
	ServerList_Feeder.GetItemText = ServerList_GetItemText;
	ServerList_Feeder.Select = ServerList_Select;

	UIFeeders.push_back(ServerList_Feeder);

	AddUIClickHandler(220, ServerList_ClickHandler_NetSource);
	AddUIClickHandler(253, ServerList_ClickHandler_GameType);

	AddUIScript("RefreshServers", ServerList_UIScript_RefreshServers);
	AddUIScript("RefreshFilter", ServerList_UIScript_RefreshFilter);
	AddUIScript("UpdateFilter", ServerList_UIScript_UpdateFilter);
	AddUIScript("ServerSort", ServerList_UIScript_ServerSort);
	AddUIScript("JoinServer", ServerList_UIScript_JoinServer);
	AddUIScript("CreateListFavorite", ServerList_UIScript_CreateListFavorite);
	AddUIScript("RemoveListFavorite", ServerList_UIScript_RemoveListFavorite);

	// some moar
	serverStatus.sortKey = SORT_PING;
	serverStatus.sortDir = false;

	// global servers cmd
	Cmd_AddCommand("globalservers", CL_GlobalServers_f, &GServCommand, 0);

	// server storage stuffs
	ServerStorage_Init();

	// netsrc list
	*(const char***)0x4CDEFF = uiNetSourceNames;
	*(const char***)0x63177B = uiNetSourceNames;
}