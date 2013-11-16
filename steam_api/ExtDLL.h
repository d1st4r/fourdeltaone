// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: shared
// Purpose: Extension DLL public API.
//
// Initial author: NTAuthority
// Started: 2013-02-07
// ==========================================================

#pragma once

#include <mongoose.h>

struct scriptability_s
{
	int notifyNumArgs;
	VariableValue* notifyStack;
	const char* notifyType;

	int tempEntRef;

	void* scriptManagerImage;

	bool (__cdecl * cbOnSay)(int client, char* name, char** textptr, int team);
	void (__cdecl * cbParsePlaylists)(const char* playlistFile);
	void (__cdecl * cbRotateMap)();
	void (__cdecl * cbInitNUI)();

	LPTOP_LEVEL_EXCEPTION_FILTER cbExceptionFilter;
};

struct nui_draw_s
{
	int nuiHeight;
	int nuiWidth;
	GfxImage* nuiImage;
	IDirect3DDevice9* device;

	void (__cdecl * AddServerToList)(netadr_t adr, const char* info);
	void (__cdecl * ClearServerList)();
	void (__cdecl * HandleStatusResponse)(netadr_t from, msg_t* msg);
};

class IClientDLL;

class IExtDLL
{
public:
	virtual void Initialize(DWORD gameFlags, IClientDLL* clientDLL) = 0;

	virtual bool* AssetRestrict_Trade1(bool* useEntryNames) = 0;

	virtual nui_draw_s* GetNUIDraw() = 0;
	virtual void NUIFrame() = 0;

	virtual scriptability_s* GetScriptability() = 0;

	virtual bool ScriptabilityIsWeb() = 0;
	virtual void HandleWebRequest(mg_connection* conn, const mg_request_info* request_info) = 0;
};

class IClientDLL
{
public:
	virtual char* GetUsername() = 0;
	virtual int GetUserID() = 0;

	virtual Material* RegisterMaterial(const char* name) = 0;
};

enum NUITeam
{
	TEAM_ARAB,
	TEAM_MILITIA,
	TEAM_RANGERS,
	TEAM_TASKFORCE141,
	TEAM_USSR,
	TEAM_SOCOM
};