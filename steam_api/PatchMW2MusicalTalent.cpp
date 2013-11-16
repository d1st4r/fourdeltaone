// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: The will of a single man. Makarov's men, fighting
//          Shepard's men. That, or a beautiful bridge...
//          about to be destroyed by some weird warfare.
//
// Initial author: NTAuthority
// Started: 2012-10-20
// ==========================================================

#include "StdInc.h"

CallHook findSoundAliasHook;
DWORD findSoundAliasHookLoc = 0x6441E9;

snd_alias_list_t* FindSoundAliasHookFunc(assetType_t type, const char* name)
{
	snd_alias_list_t* aliases = (snd_alias_list_t*)DB_FindXAssetHeader(type, name);
	
	// the most hated sound at this point
	if (!_stricmp(name, "music_mainmenu_mp"))
	{
		static snd_alias_list_t newList;
		static snd_alias_t newAlias;
		static StreamFile newFile;

		// duplicate the asset as we can't modify pointers
		memcpy(&newList, aliases, sizeof(newList));

		memcpy(&newAlias, newList.aliases, sizeof(newAlias));
		newList.aliases = &newAlias;

		memcpy(&newFile, newAlias.stream, sizeof(newFile));
		newAlias.stream = &newFile;

		// and replace the filename.
		//newFile.file = "hz_boneyard_intro_LR_1.mp3";
		newFile.file = "ui_lobby_answering_the_call.mp3";

		// oh and this too
		aliases = &newList;
	}

	return aliases;
}

void PatchMW2_MusicalTalent()
{
	findSoundAliasHook.initialize(findSoundAliasHookLoc, FindSoundAliasHookFunc);
	findSoundAliasHook.installHook();
}