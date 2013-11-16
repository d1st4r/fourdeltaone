// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Obtain playlists from XNP/dedicated server.
//
// Initial author: NTAuthority
// Started: 2011-10-31
// ==========================================================

#include "StdInc.h"

typedef struct  
{
	int offset;
	int size;
	char name[24];
} online_img_entry_t;

typedef struct  
{
	char signature[4];
	int numFiles;
	online_img_entry_t entries[1];
} online_img_header_t;

char g_onlineImg[262144];
bool* gotPlaylists = (bool*)0x1AD3680;

typedef void (__cdecl * Live_ParsePlaylists_t)(char* data);
Live_ParsePlaylists_t Live_ParsePlaylists = (Live_ParsePlaylists_t)0x4295A0;

void Live_HandleNPFile(const char* name, char* data, size_t size)
{
	if (!_stricmp(name, "playlists_mp.info"))
	{
		//Live_ParsePlaylists(data);
		g_scriptability->cbParsePlaylists(data);

		*gotPlaylists = true;
	}
}

void Live_OnDownloadNPImg(NPAsync<NPGetPublisherFileResult>* async)
{
	char* buffer;

	if (FS_ReadFile("playlists.info", (char**)&buffer) > 0)
	{
		g_scriptability->cbParsePlaylists(buffer);
		FS_FreeFile(buffer);

		*gotPlaylists = true;

		//aylist_enabled->current.boolean = true;
	}
	return;

	// blah
	NPGetPublisherFileResult* result = async->GetResult();

	if (result->result == GetFileResultOK)
	{
		// cast the header
		online_img_header_t* header = (online_img_header_t*)&g_onlineImg;

		// check if this is a VER2 IMG
		if (header->signature[0] == 'V' && header->signature[1] == 'E' && header->signature[2] == 'R' && header->signature[3] == '2')
		{
			Com_Printf(0, "%d entries in online_mp.img\n", header->numFiles);

			for (int i = 0; i < header->numFiles; i++)
			{
				Live_HandleNPFile(header->entries[i].name, &g_onlineImg[header->entries[i].offset * 2048], header->entries[i].size * 2048);
			}
		}
		else
		{
			Com_Printf(0, "online_mp.img wasn't a VER2.\n");
		}
	}
	else
	{
		Com_Printf(0, "Downloading online_mp.img failed. Error %d.\n", result->result);
	}
}

void Live_DownloadNPImg()
{
	NPAsync<NPGetPublisherFileResult>* fileResult = NP_GetPublisherFile("online_mp.img", (uint8_t*)&g_onlineImg, sizeof(g_onlineImg));
	fileResult->SetCallback(Live_OnDownloadNPImg, NULL);
	//*gotPlaylists = true;
}

dvar_t* playlist_enabled;

void Live_LoadPlaylists()
{
	playlist_enabled = Dvar_RegisterBool("playlist_enabled", false, DVAR_FLAG_NONE, "Enable playlists");

	dvar_t* dvar = Dvar_FindVar("playlistFilename");
	const char* filename = dvar->current.string;

	char* buffer;

	if (FS_ReadFile(filename, (char**)&buffer) > 0)
	{
		Live_ParsePlaylists(buffer);
		FS_FreeFile(buffer);

		playlist_enabled->current.boolean = true;
		*gotPlaylists = true;
	}
	else
	{
		playlist_enabled->current.boolean = false;
	}
}

StompHook liveInitHook;
DWORD liveInitHookLoc = 0x420B5A;

cmd_function_s playlistReloadCmd;

void PatchMW2_Playlists()
{
	// disable playlist download function
	*(BYTE*)0x4D4790 = 0xC3;

	// disable playlist.ff loading function
	*(BYTE*)0x4D6E60 = 0xC3;

	// playlist dvar 'validity check'
	*(BYTE*)0x4B1170 = 0xC3;

	// hook end of Live_Init to perform playlist download
	liveInitHook.initialize(liveInitHookLoc);

	if (GAME_FLAG(GAME_FLAG_DEDICATED))
	{
		//playlist_enabled = Dvar_RegisterBool("playlist_enabled", true, DVAR_FLAG_NONE, "Enable playlists");
		playlist_enabled = Dvar_RegisterBool("playlist_enabled", false, DVAR_FLAG_NONE, "Enable playlists");

		liveInitHook.installHook(Live_DownloadNPImg);

		Cmd_AddCommand("playlist_reload", Live_DownloadNPImg, &playlistReloadCmd, 0);

		// disable playlist checking
		*(BYTE*)0x5B69E9 = 0xEB; // too new
		*(BYTE*)0x5B696E = 0xEB; // too old
	}
	else
	{
		*gotPlaylists = true;
	}
}