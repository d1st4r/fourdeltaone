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

#include "StdInc.h"
#include "ServerStorage.h"

#define SERVERSTORAGE_LIST_VERSION 1
#define SERVERSTORAGE_MAX_ENTRIES 512
#define SERVERSTORAGE_GET_BUFFER_SIZE ((sizeof(uint32_t) * 2) + (sizeof(ServerStorageEntry) * SERVERSTORAGE_MAX_ENTRIES))
#define SERVERSTORAGE_DOWNLOAD_RETRY_TIME 2000 // can't take too long as otherwise the user may lose data

ServerStorageList favoritesList; // no u in this word
ServerStorageList historyList;

void ServerStorage_InitList(ServerStorageList* list, const char* name, int maxEntries)
{
	list->initialized = true;
	list->name = name;
	list->maxEntries = maxEntries;
}

void ServerStorage_Init()
{
	ServerStorage_InitList(&favoritesList, "favorites_iw4_gen3", -1);
	ServerStorage_InitList(&historyList, "history_iw4_gen3", 50);
}

struct ServerStorageSerializedList
{
	uint32_t listVersion;
	uint32_t numEntries;
	ServerStorageEntry entries[1];
};

void DeserializeListFromNPCB(NPAsync<NPGetUserFileResult>* async)
{
	// more fun in this function. if you didn't notice yet, comments are written from bottom to top.
	NPGetUserFileResult* result = async->GetResult();
	ServerStorageList* list = (ServerStorageList*)async->GetUserData();

	list->retrieved = true;

	if (result->result != GetFileResultOK)
	{
		Trace("ServerStorage", "get %s failed with %x\n", list->name, result->result); // what if Trace actually adds a \n itself?
		return;
	}

	ServerStorageSerializedList* serializedList = (ServerStorageSerializedList*)result->buffer; // look at us assuming it actually is a serialized list
	if (serializedList->listVersion != SERVERSTORAGE_LIST_VERSION)
	{
		Trace("ServerStorage", "get %s failed as the list isn't actually the version we like (%i) but was actually more like %i.\n", list->name, SERVERSTORAGE_LIST_VERSION, serializedList->listVersion);
		return;
	}

	// insanity checks complete, let's do some real work.
	if (serializedList->numEntries > SERVERSTORAGE_MAX_ENTRIES)
	{
		return; // oh, wait, that was another insanity check.
	}

	// now, let's actually build the list we've all grown to love. (have you found the 11 easter eggs? Rockstar Leeds would like to thank you for playing the game, and
	// by the way, the christmas party was great.)
	//
	// wait, this isn't VCS 'USJ 005a'.
	list->data.reserve(serializedList->numEntries); // the bit of detail less experienced developers would lack. it also wouldn't matter on the fast PCs nowadays.

	for (int i = 0; i < serializedList->numEntries; i++)
	{
		ServerStorageEntry entry = serializedList->entries[i];
		list->data.push_back(entry);
	}

	// are we even done? guess we are! is that even the last function in this module? guess so! integration time!
}

void ServerStorage_DeserializeListFromNP(ServerStorageList* list, uint8_t* buffer)
{
	// let's have the same fun we had when writing the serialization function *all over again*!
	// damn, this is repetitive. nearly am starting to understand these kids with their 'education is boring'
	// lines. the difference is that I actually have a 'choice' to not do this. well, I would have - 
	// if it weren't for the fact that it's the only thing I can do outside of sleeping and idling on IRC/the
	// forums.
	//
	// and don't start that 'take some completely different non-tech direction with your life' story
	// again; I've become too used to this over the years, and my brain is pretty much fried anyway,
	// so there's no way in hell that I'm going to learn how to do anything else sensibly.
	// plus that I don't want to bother with things I don't understand instantly, due to the
	// ease I've had with learning this 'as a side effect' and now just using the stuff I know
	// without even having to think - it's also why I decided to 'screw general education' as
	// I wouldn't understand a thing in history, biology and anything else that isn't obvious logic to me
	// (i.e. English-as-second-language and mathematics, and I even have difficulty in the latter)
	//
	// anyway, let's stop with writing this rant that should've been on the forums, and let's get to coding.

	// the buffer reuse issue is easily fixed by making the caller pass a buffer, so...
	NPID npID;
	NP_GetNPID(&npID); // the number that many people decide to turn into weird stuff like 0x133713371337xxxx; no support given for such cases

	NPAsync<NPGetUserFileResult>* async = NP_GetUserFile(list->name, npID, buffer, SERVERSTORAGE_GET_BUFFER_SIZE); // we assume the buffer is of the required size here
	async->SetCallback(DeserializeListFromNPCB, list);

	// hm, the above comment was a really long comment for such a short function.
}

static uint8_t favoritesBuffer[SERVERSTORAGE_GET_BUFFER_SIZE];
static uint8_t historyBuffer[SERVERSTORAGE_GET_BUFFER_SIZE];

void ServerStorage_DeserializeFromNP()
{
	ServerStorage_DeserializeListFromNP(&favoritesList, favoritesBuffer);
	ServerStorage_DeserializeListFromNP(&historyList, historyBuffer);
}

void ServerStorage_EnsureSafeListRetrieval(ServerStorageList* list, uint8_t* buffer)
{
	if (list->retrieved)
	{
		return;
	}

	if ((GetTickCount() - list->lastRetrieval) > SERVERSTORAGE_DOWNLOAD_RETRY_TIME)
	{
		ServerStorage_DeserializeListFromNP(list, buffer);
		list->lastRetrieval = GetTickCount();
	}
}

void ServerStorage_EnsureSafeRetrieval()
{
	ServerStorage_EnsureSafeListRetrieval(&favoritesList, favoritesBuffer);
	ServerStorage_EnsureSafeListRetrieval(&historyList, historyBuffer);
}

void SerializeListToNPCB(NPAsync<NPWriteUserFileResult>* async)
{
	free(async->GetUserData());
}

void ServerStorage_SerializeListToNP(ServerStorageList* list)
{
	// allocate a ServerStorageSerializedList large enough to hold our entries
	int serializedSize = sizeof(ServerStorageSerializedList) + (sizeof(ServerStorageEntry) * (list->data.size() - 1));
	ServerStorageSerializedList* serializedList = (ServerStorageSerializedList*)malloc(serializedSize);
	serializedList->listVersion = SERVERSTORAGE_LIST_VERSION;
	serializedList->numEntries = list->data.size();

	// and loop through the list. how amazing. I so want to be doing this all day long. it's so *new* and *interesting*.
	// oh, wait, that's why my preference is to do something sysadmin-y. oh how I don't feel sorry for all those kids
	// who think writing 'regular' code is cool and interesting and actually intend on or are actually following courses
	// teaching them such, and yet would only wind up having a very slight bit of the experience I wound up having.
	for (int i = 0; i < list->data.size(); i++)
	{
		serializedList->entries[i] = list->data[i];
	}

	// now it gets a bit more interesting as I call my ever-glitchy NP functions around this part
	// note the NPID parameter; I'm unsure if it's even checked server-side, so you may be able to abuse this
	// and give your favorite users a favorites list containing your server. or clear their stats.
	NPID npID;
	NP_GetNPID(&npID); // why is it even 2 lines? guess I was trying to follow the Xenon APIs there

	NPAsync<NPWriteUserFileResult>* async = NP_WriteUserFile(list->name, npID, (uint8_t*)serializedList, serializedSize);

	// we don't care about the NPAsync or the result from the call, if it fails, they'll whine
	// if it succeeds, they'll whine anyway, as there's only whining around here.

	// it may very well be uploading 2 files at once fails horribly, with no clue as to where the issue might be
	// in that case, screw this, I'm going hanggliding. that's a Wave 103 reference, by the way.

	// oh, I forgot, still need to free that malloc'd buffer. oh well, time to declare more callbacks than I had
	// expected to be needed. *sigh*
	async->SetCallback(SerializeListToNPCB, serializedList); // no ServerStorage_ prefix as I can't be bothered to type, and R*N hasn't done so either in their RW callbacks
}

void ServerStorage_SerializeToNP()
{
	ServerStorage_SerializeListToNP(&favoritesList);
	ServerStorage_SerializeListToNP(&historyList);
}

// retrieve data from a storage list
int ServerStorage_GetNumEntries(ServerStorageList* list)
{
	return list->data.size();
}

ServerStorageEntry* ServerStorage_GetEntry(ServerStorageList* list, int num)
{
	return &list->data[num];
}

// add data to a storage list
void ServerStorage_AddEntry(ServerStorageList* list, netadr_t adr, const char* name)
{
	// if this isn't NA_IP, return - it is likely NA_LOOPBACK in that case
	if (adr.type != NA_IP)
	{
		return;
	}

	// look for an existing entry in the storage list
	for (std::vector<ServerStorageEntry>::iterator iter = list->data.begin(); iter != list->data.end(); iter++)
	{
		if (adr.type == iter->address.type)
		{
			if (memcmp(&adr.ip, &iter->address.ip, sizeof(adr.ip)) == NULL)
			{
				if (adr.port == iter->address.port)
				{
					// it's the same!
					return;
				}
			}
		}
	}

	// as we didn't return up there, we presumably have a new server. how fun!
	ServerStorageEntry newEntry;
	memset(&newEntry, 0, sizeof(newEntry));
	newEntry.address = adr;
	StringCbCopy(newEntry.name, sizeof(newEntry.name), name); // strsafe is cool, we can't assume everyone #define's strcpy_s

	// if the maximum entry count was reached
	int maxEntries = (list->maxEntries == -1) ? SERVERSTORAGE_MAX_ENTRIES : list->maxEntries;

	if (list->data.size() >= maxEntries)
	{
		// we'll have to move all the entries up a bit, also really fun
		for (int i = 1; i < list->data.size(); i++)
		{
			list->data[i - 1] = list->data[i];
		}

		list->data.pop_back();
	}

	// and add the entry we created above
	list->data.push_back(newEntry);

	// and serialize the stuff
	ServerStorage_SerializeListToNP(list);
}

// remove data from a storage list
void ServerStorage_RemoveEntry(ServerStorageList* list, netadr_t adr)
{
	// look for an existing entry in the storage list
	for (std::vector<ServerStorageEntry>::iterator iter = list->data.begin(); iter != list->data.end(); iter++)
	{
		if (adr.type == iter->address.type)
		{
			if (memcmp(&adr.ip, &iter->address.ip, sizeof(adr.ip)) == NULL)
			{
				if (adr.port == iter->address.port)
				{
					// it's the same!
					list->data.erase(iter);

					// and serialize the list
					ServerStorage_SerializeListToNP(list);

					return;
				}
			}
		}
	}
}

// this source file is sponsored by -- another rock coming through what used to be my window -- the following localized string entry:
// REFERENCE           ERR_MULTITEX_INIT_FAIL
// LANG_ENGLISH        "OpenGL 1.3 multitexture found, but it failed to initialize."
// 
// found in IW4.
//
// another random note: I still wish some IWx/Tx game would ever have custom CMod stuff loaded into it... I was close with T5, but
// never bothered finishing it. now, with the lack of an 'open multiplayer network' maintained by me (and running on the new dwentry
// code, rather than the crazy server emulator) that's going to be a bit hard to do anyway.
// oh, and CMod in this case stands for 'clip model' or 'collision model' or something; the type is named clipMap_t (Q3 remnant)
// but the asset string type is col_map_mp. now, just a clipmap won't do much, it'd also need a GfxWorld (the main struct
// which didn't allow cross-game compat for IW4/IW5 levels) and seemingly a ComWorld (which only contains primary lights or something)
// mm, and apparently primary lights changed a lot between T4 and T5. well, they got a lot of (unknown, obviously) additional data.
//
// these comments are dated the same date as the 'Started' time in the header comment, btw; well, around such.