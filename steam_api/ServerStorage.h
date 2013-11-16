// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: User server list storage functionality.
//
// Initial author: NTAuthority
// Started: 2012-10-19
// ==========================================================

#pragma once
#include <list>

struct ServerStorageEntry
{
	netadr_t address; // depends on a Quake definition, might be a bad idea for code reusability?
	char name[256];
};

struct ServerStorageList
{
	bool initialized;
	bool retrieved;
	const char* name;
	int maxEntries; // -1 if 'unlimited' (although limited by the NP retrieve buffer)
	int lastRetrieval; // time of last download attempt
	std::vector<ServerStorageEntry> data; // vector is used seeing as it can directly index data
};

extern ServerStorageList favoritesList;
extern ServerStorageList historyList;

// initialize the server storage
void ServerStorage_Init();

// serialization/deserialization and such
void ServerStorage_DeserializeFromNP();
void ServerStorage_SerializeToNP();
void ServerStorage_EnsureSafeRetrieval();

// retrieve data from a storage list
int ServerStorage_GetNumEntries(ServerStorageList* list);
ServerStorageEntry* ServerStorage_GetEntry(ServerStorageList* list, int num);

// add data to a storage list
void ServerStorage_AddEntry(ServerStorageList* list, netadr_t adr, const char* name);

// remove data from a storage list
void ServerStorage_RemoveEntry(ServerStorageList* list, netadr_t adr);