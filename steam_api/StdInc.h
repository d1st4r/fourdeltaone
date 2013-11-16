// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: standard include file for system headers
//
// Initial author: NTAuthority
// Started: 2010-08-29
// ==========================================================

#pragma once

//#define WE_DO_WANT_NUI
//#define ENABLE_INVERSE
//#define INVERSE_OGRE
#define COMPILING_IW4M

#include "../dependencies/tools/buildnumber.h"

#define VERSION "4.0-" BUILDNUMBER_STR

#ifdef COMPILING_IW4M
#define VERSIONSTRING "IW4M " VERSION

#else

#define VERSIONSTRING "Warfare2 " VERSION
#endif

#define ENV "prod"
#define NP_SERVER "iw4." ENV ".fourdeltaone.net"
//#define NP_SERVER "192.168.178.83"

//#define KEY_DISABLED

// ---------------------------------------------------------
#define _CRT_SECURE_NO_WARNINGS
#define _USING_V110_SDK71_

// Windows headers
#define WIN32_LEAN_AND_MEAN
#define ReadDirectoryChangesW ReadDirectoryChangesW__
#define CreateRemoteThread CreateRemoteThread__
#include <windows.h>
#undef CreateRemoteThread
#undef ReadDirectoryChangesW

#if D3D_EXPERIMENTS
#include <d3d9.h>
#else
struct IDirect3DVertexShader9;
struct IDirect3DPixelShader9;
struct IDirect3DTexture9;
struct IDirect3DVertexDeclaration9;
#endif

// LibNP
#include <libnp.h>

// C/C++ headers
#define _DEBUG 1
#define _CRTDBG_MAP_ALLOC 1

#include <stdlib.h>
#include <crtdbg.h>

#define _SCL_SECURE_NO_WARNINGS

#include <map>
#include <vector>
#include <string>
#include <unordered_map>

// OSW headers
#define NO_STEAM // to reduce header amount needed
#include "CCallback.h"
#include "ISteamClient008.h"
#include "ISteamContentServer002.h"
#include "ISteamUser012.h"
#include "ISteamFriends005.h"
#include "ISteamGameServer009.h"
#include "ISteamMasterServerUpdater001.h"
#include "ISteamMatchmaking007.h"
#include "ISteamNetworking003.h"
#include "ISteamRemoteStorage002.h"
#include "ISteamUtils005.h"
#include "ISteamUserStats006.h"

// Steam base code
#include "SteamBase.h"

// Generic utilities
#include "Utils.h"

// MW2 structures
#include "MW2.h"

// Hooking
#include "Hooking.h"

// more names
void PatchMW2();
void Sys_RunInit();

void Inverse_Init();

extern dvar_t* sv_sayName;

void G_SayToClient(int client, DWORD color, const char* name, const char* text);
void G_SayToAll(DWORD color, const char* name, const char* text);

//void* custom_malloc(size_t size, char* file, int line);

//#define malloc(x) custom_malloc(x, __FILE__, __LINE__)

// safe string functions
#define STRSAFE_NO_DEPRECATE
#define STRSAFE_NO_CCH_FUNCTIONS
#include <tchar.h>
#include <strsafe.h>

#undef sprintf_s
#define sprintf_s StringCbPrintf
//#define sprintf_s(buf, size, format, ...) StringCbPrintf(buf, size, format, __VA_ARGS__)

#undef strcat_s
#define strcat_s StringCbCat
//#define strcat_s(dst, size, src) StringCbCat(dst, size, src)

#undef strcpy_s
#define strcpy_s StringCbCopy

bool Legacy_IsLegacyMode();

struct IDirect3DDevice9;

// shared structures
#include "ExtDLL.h"

extern nui_draw_s* g_nuiDraw;

extern scriptability_s* g_scriptability;

#ifdef BUILDING_EXTDLL
extern IClientDLL* g_clientDLL;
#else
extern IExtDLL* g_extDLL;
#endif