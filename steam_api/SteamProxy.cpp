// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Steam proxying functionality
//
// Initial author: NTAuthority
// Started: 2011-11-05
// ==========================================================

#include "StdInc.h"
#include "Interface_OSW.h"

static CSteamAPILoader loader;

struct
{
	CreateInterfaceFn clientFactory;
	HSteamPipe pipe;
	HSteamUser user;

	ISteamClient008* steamClient;

	bool initialized;

	EExternalAuthState eaState;
	int eaCall;
} g_steamProxy;

bool SteamProxy_InitInternal()
{
	g_steamProxy.clientFactory = loader.Load();

	if (g_steamProxy.clientFactory == NULL)
	{
		return false;
	}

	g_steamProxy.steamClient = (ISteamClient008*)g_steamProxy.clientFactory(STEAMCLIENT_INTERFACE_VERSION_008, NULL);

	if (g_steamProxy.steamClient == NULL)
	{
		return false;
	}

	g_steamProxy.pipe = g_steamProxy.steamClient->CreateSteamPipe();

	if (g_steamProxy.pipe == NULL)
	{
		return false;
	}

	g_steamProxy.user = g_steamProxy.steamClient->ConnectToGlobalUser(g_steamProxy.pipe);

	if (g_steamProxy.user == NULL)
	{
		return false;
	}

	ISteamUtils005* steamUtils = (ISteamUtils005*)g_steamProxy.steamClient->GetISteamUtils(g_steamProxy.pipe, STEAMUTILS_INTERFACE_VERSION_005);
	int appID = steamUtils->GetAppID();

	if (appID == 0)
	{
		return false;
	}

	ISteamUser012* steamUser = (ISteamUser012*)g_steamProxy.steamClient->GetISteamUser(g_steamProxy.user, g_steamProxy.pipe, STEAMUSER_INTERFACE_VERSION_012);
	CSteamID steamID = steamUser->GetSteamID();

	NP_SetExternalSteamID(steamID.ConvertToUint64());

	return true;
}

void SteamProxy_LoadGameOverlayRenderer()
{
	if (GetModuleHandle("gameoverlayrenderer.dll") != NULL)
	{
		return;
	}

	std::string gameOverlayRendererPath = loader.GetSteamDir() + "\\gameoverlayrenderer.dll";
	LoadLibrary(gameOverlayRendererPath.c_str());
}

void SteamProxy_Init()
{
	//SetEnvironmentVariable("SteamAppId", "10190");

	if (GAME_FLAG(GAME_FLAG_DEDICATED))
	{
		return;
	}

	FILE* file = fopen("steam_appid.txt", "w");
	if (file)
	{
		fprintf(file, "10190");
		fclose(file);
	}

	g_steamProxy.initialized = SteamProxy_InitInternal();

	if (g_steamProxy.initialized)
	{
		SteamProxy_LoadGameOverlayRenderer();
	}
}

void NPA_StateSet(EExternalAuthState result)
{
	g_steamProxy.eaState = result;
	g_steamProxy.eaCall++;
}

void SteamProxy_DoThatTwinklyStuff()
{
	if (!GAME_FLAG(GAME_FLAG_DEDICATED) && false)
	{
		Com_Printf(0, "Performing Steam authentication...\n");

		// wait until some non-unverified result is returned
		while (g_steamProxy.eaState == ExternalAuthStateUnverified)
		{
			Sleep(1);
			NP_RunFrame();

			if (g_steamProxy.eaCall == 2 && !g_steamProxy.initialized) // if on the second call, and we're not running Steam
			{
				Com_Error(1, "Steam must be running for initial authentication to succeed. Please start Steam and restart the game to continue.");
			}
		}

		// do stuff based on verification result
		if (g_steamProxy.eaState == ExternalAuthStatePirate)
		{
			Com_Error(1, "Your current Steam account does not own Call of Duty: Modern Warfare 2. Please purchase the game or try another Steam account.");
		}
		else if (g_steamProxy.eaState == ExternalAuthStateError)
		{
			Com_Error(1, "An error occurred during Steam verification. Please try again later. If this issue persists, ask on http://fourdeltaone.net/.");
		}

		// guess the twinkles were right, you really do own the game!
		Com_Printf(0, "Steam authentication passed.\n");
	}	
}