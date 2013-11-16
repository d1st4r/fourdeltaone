// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Ingame mod list
//
// Initial author: Pigophone
// Started: 2012-05-01
// ==========================================================

#include "StdInc.h"
#include "PatchMW2UIScripts.h"

struct modInfo_t
{
	char** mods;
	int max;
	int current;
};
modInfo_t modInfo;
int ModList_GetItemCount()
{
	return modInfo.max;
}
void ModList_Select(int index)
{
	modInfo.current = index;
}

const char* ModList_GetItemText(int index, int column)
{
	if (modInfo.current >= 0 && modInfo.current < modInfo.max)
	{
		return modInfo.mods[index];
	}

	return "...";
}
extern dvar_t** fs_homepath;
extern dvar_t* cl_modVidRestart;

void ModList_UIScript_LoadMods(char* args)
{
	if (modInfo.mods != NULL && *modInfo.mods != NULL)
	{
		Sys_FreeFileList(modInfo.mods);
	}

	modInfo.mods = Sys_ListFiles((char*)va("%s\\%s", (*fs_homepath)->current.string, "mods"), NULL, NULL, &modInfo.max);
}

void ModList_UIScript_RunMod(char* args)
{
	if (modInfo.mods != NULL && *modInfo.mods != NULL && modInfo.current >= 0 && modInfo.current < modInfo.max)
	{
		Cmd_ExecuteSingleCommand(0, 0, va("fs_game \"mods/%s\"", modInfo.mods[modInfo.current]));
		if (cl_modVidRestart->current.boolean)
		{
			Cmd_ExecuteSingleCommand(0, 0, "vid_restart");
		}
		else
		{
			Cmd_ExecuteSingleCommand(0, 0, "closemenu mods_menu");
		}
	}
}

void ModList_UIScript_ClearMods(char* args)
{
	Cmd_ExecuteSingleCommand(0, 0, "fs_game \"\"");
	if (cl_modVidRestart->current.boolean)
	{
		Cmd_ExecuteSingleCommand(0, 0, "vid_restart");
	}
	else
	{
		Cmd_ExecuteSingleCommand(0, 0, "closemenu mods_menu");
	}
}
void PatchMW2_ModList()
{
	modInfo.max = modInfo.current = 0;

	UIFeeder_t Feeder_ModList;
	Feeder_ModList.feeder = 9.0f;
	Feeder_ModList.Select = ModList_Select;
	Feeder_ModList.GetItemCount = ModList_GetItemCount;
	Feeder_ModList.GetItemText = ModList_GetItemText;
	UIFeeders.push_back(Feeder_ModList);

	AddUIScript("LoadMods", ModList_UIScript_LoadMods);
	AddUIScript("RunMod", ModList_UIScript_RunMod);
	AddUIScript("ClearMods", ModList_UIScript_ClearMods);
}
