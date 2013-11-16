// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Modern Warfare 2 patches: status queries
//
// Initial author: NTAuthority
// Started: 2010-09-27
// ==========================================================

#include "StdInc.h"

#pragma unmanaged
// leaky bucket handling code from ioq3
typedef struct leakyBucket_s leakyBucket_t;
typedef unsigned __int8 byte;
struct leakyBucket_s {
	netadrtype_t	type;

	union {
		byte	_4[4];
		byte	_6[16];
	} ipv;

	int						lastTime;
	signed char		burst;

	long					hash;

	leakyBucket_t *prev, *next;
};

// This is deliberately quite large to make it more of an effort to DoS
#define MAX_BUCKETS			16384
#define MAX_HASHES			1024

static leakyBucket_t buckets[ MAX_BUCKETS ];
static leakyBucket_t *bucketHashes[ MAX_HASHES ];

/*
================
SVC_HashForAddress
================
*/


static long SVC_HashForAddress( netadr_t address ) {
	byte 		*ip = NULL;
	size_t	size = 0;
	int			i;
	long		hash = 0;

	switch ( address.type ) {
		case NA_IP:  ip = address.ip;  size = 4; break;
		default: break;
	}

	for ( i = 0; i < (int)size; i++ ) {
		hash += (long)( ip[ i ] ) * ( i + 119 );
	}

	hash = ( hash ^ ( hash >> 10 ) ^ ( hash >> 20 ) );
	hash &= ( MAX_HASHES - 1 );

	return hash;
}

/*
================
SVC_BucketForAddress

Find or allocate a bucket for an address
================
*/
static leakyBucket_t *SVC_BucketForAddress( netadr_t address, int burst, int period ) {
	leakyBucket_t	*bucket = NULL;
	int						i;
	long					hash = SVC_HashForAddress( address );
	int						now = GetTickCount();

	for ( bucket = bucketHashes[ hash ]; bucket; bucket = bucket->next ) {
		switch ( bucket->type ) {
			case NA_IP:
				if ( memcmp( bucket->ipv._4, address.ip, 4 ) == 0 ) {
					return bucket;
				}
				break;

			default:
				break;
		}
	}

	for ( i = 0; i < MAX_BUCKETS; i++ ) {
		int interval;

		bucket = &buckets[ i ];
		interval = now - bucket->lastTime;

		// Reclaim expired buckets
		if ( bucket->lastTime > 0 && interval > ( burst * period ) ) {
			if ( bucket->prev != NULL ) {
				bucket->prev->next = bucket->next;
			} else {
				bucketHashes[ bucket->hash ] = bucket->next;
			}

			if ( bucket->next != NULL ) {
				bucket->next->prev = bucket->prev;
			}

			memset( bucket, 0, sizeof( leakyBucket_t ) );
		}

		//if ( bucket->type == NA_BAD ) {
		if ( bucket->type == NULL ) {
			bucket->type = address.type;
			switch ( address.type ) {
				case NA_IP:  memcpy( bucket->ipv._4, address.ip, 4 );   break;
				default: break;
			}

			bucket->lastTime = now;
			bucket->burst = 0;
			bucket->hash = hash;

			// Add to the head of the relevant hash chain
			bucket->next = bucketHashes[ hash ];
			if ( bucketHashes[ hash ] != NULL ) {
				bucketHashes[ hash ]->prev = bucket;
			}

			bucket->prev = NULL;
			bucketHashes[ hash ] = bucket;
			return bucket;
		}
	}

	// Couldn't allocate a bucket for this address
	return NULL;
}

/*
================
SVC_RateLimit
================
*/
static bool SVC_RateLimit( leakyBucket_t *bucket, int burst, int period ) {
	if ( bucket != NULL ) {
		int now = GetTickCount();
		int interval = now - bucket->lastTime;
		int expired = interval / period;
		int expiredRemainder = interval % period;
		//Com_Printf( 0, "expired = %d, burst = %d, bucketburst = %d\n", expired, burst, bucket->burst);
		if ( expired > bucket->burst ) {
			bucket->burst = 0;
			bucket->lastTime = now;
		} else {
			bucket->burst -= expired;
			bucket->lastTime = now - expiredRemainder;
		}
		
		if ( bucket->burst < burst ) {
			bucket->burst++;

			return false;
		}
	}

	return true;
}

/*
================
SVC_RateLimitAddress

Rate limit for a particular address
================
*/
static bool SVC_RateLimitAddress( netadr_t from, int burst, int period ) {
	leakyBucket_t *bucket = SVC_BucketForAddress( from, burst, period );
	if (bucket == NULL) {
		//Com_Printf(0, "bucket is null!");
	}
	return SVC_RateLimit( bucket, burst, period );
}

// getstatus/getinfo OOB packets
CallHook oobHandlerHook;
DWORD oobHandlerHookLoc = 0x6267EB;

typedef DWORD (__cdecl* SV_GameClientNum_Score_t)(int clientID);
SV_GameClientNum_Score_t SV_GameClientNum_Score = (SV_GameClientNum_Score_t)0x469AC0;

dvar_t** sv_privateClients = (dvar_t**)0x2098D8C;

void SVC_Info(netadr_t from, void* msg) {
	int clientCount = 0;
	char infostring[MAX_INFO_STRING];

	// prevent oversized infostrings
	if (strlen(Cmd_ArgvSV(1)) > 64)
	{
		return;
	}

	// check if it's a legacy master server querying
	bool legacyMaster = false;

	if (from.ip[0] == 85 && from.ip[1] == 131 && from.ip[2] == 163 && from.ip[3] == 180)
	{
		legacyMaster = true;
	}

	if (from.ip[0] == 85 && from.ip[1] == 214 && from.ip[2] == 230 && from.ip[3] == 158)
	{
		legacyMaster = true;
	}

	for (int i = 0; i < *svs_numclients; i++) {
		if (svs_clients[i].state >= 3) {
			clientCount++;
		}
	}

	infostring[0] = 0;

	Info_SetValueForKey(infostring, "challenge", Cmd_ArgvSV(1));
	Info_SetValueForKey(infostring, "hostname", GetStringConvar("sv_hostname"));
	Info_SetValueForKey(infostring, "gamename", "IW4");

	// set protocol (lmaobox says oddities in legacy code make it needed to be here)
	Info_SetValueForKey(infostring, "protocol", va("%i", PROTOCOL));

	// legacy server; master.alterrev.net
	if (legacyMaster)
	{
		Info_SetValueForKey(infostring, "protocol", va("%i", 142));
	}

	// and more data
	Info_SetValueForKey(infostring, "mapname", GetStringConvar("mapname"));
	Info_SetValueForKey(infostring, "clients", va("%i", clientCount));
	Info_SetValueForKey(infostring, "sv_privateClients", va("%i", (*sv_privateClients)->current.integer));
	Info_SetValueForKey(infostring, "sv_maxclients", va("%i", *svs_numclients));
	Info_SetValueForKey(infostring, "gametype", GetStringConvar("g_gametype"));
	Info_SetValueForKey(infostring, "pure", "1");
	Info_SetValueForKey(infostring, "fs_game", GetStringConvar("fs_game"));
	Info_SetValueForKey(infostring, "shortversion", VERSION);

	if (legacyMaster)
	{
		Info_SetValueForKey(infostring, "shortversion", "0.3c");
	}

	// NPID
	NPID npID;
	NP_GetNPID(&npID);

	Info_SetValueForKey(infostring, "npid", va("%16llX", npID));

	bool hardcore = 0;
	dvar_t* g_hardcore = 0;
	g_hardcore = Dvar_FindVar("g_hardcore");
	
	if (g_hardcore)
	{
		hardcore = g_hardcore->current.boolean;
	}

	Info_SetValueForKey(infostring, "hc", va("%i", hardcore));

	NET_OutOfBandPrint(1, from, "infoResponse\n%s", infostring);
}

void SVC_Status(netadr_t from, void* msg) {
	char infostring[8192];
	char player[1024];
	char status[2048];
	int playerLength = 0;
	int statusLength = 0;

	// Prevent using getstatus as an amplifier
	if ( SVC_RateLimitAddress( from, 10, 100 ) ) {
		return;
	}

	// prevent oversized infostrings
	if (strlen(Cmd_ArgvSV(1)) > 64)
	{
		return;
	}

	//strncpy(infostring, Dvar_InfoString_Big(1028), 8192);
	strncpy(infostring, Dvar_InfoString_Big(1024), 1024);

	char* hostname = GetStringConvar("sv_hostname");

	Info_SetValueForKey(infostring, "challenge", Cmd_ArgvSV(1));
	//Info_SetValueForKey(infostring, "sv_maxclients", va("%i", *(int*)0x31D938C));
	Info_SetValueForKey(infostring, "sv_maxclients", va("%i", Party_NumPublicSlots()));
	Info_SetValueForKey(infostring, "protocol", va("%i", PROTOCOL));
	Info_SetValueForKey(infostring, "shortversion", VERSION);

	for (int i = 0; i < *svs_numclients; i++) {
		if (svs_clients[i].state >= 3) { // connected
			int score = SV_GameClientNum_Score(i);
			int ping = svs_clients[i].ping;
			char* name = svs_clients[i].name;

			_snprintf(player, sizeof(player), "%i %i \"%s\"\n", score, ping, name);

			playerLength = strlen(player);
			if (statusLength + playerLength >= sizeof(status) ) {
				break;
			}

			strcpy (status + statusLength, player);
			statusLength += playerLength;
		}
	}

	status[statusLength] = '\0';

	NET_OutOfBandPrint(1, from, "statusResponse\n%s\n%s", infostring, status);
}

void HandleCustomOOB(const char* commandName, netadr_t from, void* msg) {
	if (!strcmp(commandName, "getinfo")) {
		return SVC_Info(from, msg);
	}

	if (!strcmp(commandName, "getstatus")) {
		return SVC_Status(from, msg);
	}
}

void __declspec(naked) OobHandlerHookStub() {
	__asm {
		// esp + 408h
		push esi
		mov eax, [esp + 40Ch + 14h]
		push eax
		mov eax, [esp + 410h + 10h]
		push eax
		mov eax, [esp + 414h + 0Ch]
		push eax
		mov eax, [esp + 418h + 08h]
		push eax
		mov eax, [esp + 41Ch + 04h]
		push eax
		push edi
		call HandleCustomOOB
		add esp, 1Ch
		jmp oobHandlerHook.pOriginal
	}
}

// entry point
void PatchMW2_Status()
{
	oobHandlerHook.initialize(oobHandlerHookLoc, OobHandlerHookStub);
	oobHandlerHook.installHook();

	// remove SV_WaitServer; might be optional these days
	//memset((void*)0x446EF6, 0x90, 5);
}