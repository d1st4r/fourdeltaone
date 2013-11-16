// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Basic Steam interface functions for usage in main
//
// Initial author: NTAuthority
// Started: 2010-09-10
// ==========================================================

#include "StdInc.h"
#include "SteamRemoteStorage002.h"
#include "SteamUser012.h"
#include "SteamUtils005.h"
#include "SteamNetworking003.h"
#include "SteamFriends005.h"
#include "SteamMatchmaking007.h"
#include "SteamGameServer009.h"
#include "SteamMasterServerUpdater001.h"

#undef interface

// define so the template can be compiled
std::vector<SteamInterface2_s> CSteamBase::_instances;
std::unordered_map<SteamAPICall_t, bool> CSteamBase::_calls;
std::unordered_map<SteamAPICall_t, CCallbackBase*> CSteamBase::_resultHandlers;
std::vector<SteamAPIResult_t> CSteamBase::_results;
std::vector<CCallbackBase*> CSteamBase::_callbacks;
int CSteamBase::_callID;

void* CSteamBase::CreateInterface(SteamInterface_t interfaceID)
{
	// even though this might be done nicer, I'm doing it using a switch statement now
	switch (interfaceID)
	{
		case INTERFACE_STEAMREMOTESTORAGE002:
			return new CSteamRemoteStorage002;
		case INTERFACE_STEAMUSER012:
			return new CSteamUser012;
		case INTERFACE_STEAMUTILS005:
			return new CSteamUtils005;
		case INTERFACE_STEAMNETWORKING003:
			return new CSteamNetworking003;
		case INTERFACE_STEAMFRIENDS005:
			return new CSteamFriends005;
		case INTERFACE_STEAMMATCHMAKING007:
			return new CSteamMatchmaking007;
		case INTERFACE_STEAMGAMESERVER009:
			return new CSteamGameServer009;
		case INTERFACE_STEAMMASTERSERVERUPDATER001:
			return new CSteamMasterServerUpdater001;
		/*case INTERFACE_STEAMCLIENT008:
			return new CSteamClient008;
			break;*/
	}

	return NULL;
}

void* CSteamBase::GetInterface(SteamInterface_t interfaceID)
{
	for (size_t i = 0; i < _instances.size(); i++)
	{
		if (_instances[i].interface == interfaceID)
		{
			return _instances[i].instance;
		}
	}
	SteamInterface2_s temp;
	temp.interface = interfaceID;
	temp.instance = CreateInterface(interfaceID);
	_instances.push_back(temp);
	return temp.instance;
}

void CSteamBase::RegisterCallback(CCallbackBase *handler, int callback)
{
	handler->SetICallback(callback);
	_callbacks.push_back(handler);
}

void CSteamBase::RegisterCallResult(SteamAPICall_t call, CCallbackBase *result)
{
	_resultHandlers[call] = result;
}

SteamAPICall_t CSteamBase::RegisterCall()
{
	_callID++;

	_calls[_callID] = false;

	return _callID;
}

void CSteamBase::ReturnCall(void *data, int size, int type, SteamAPICall_t call)
{
	SteamAPIResult_t result;

	_calls[call] = true;

	result.call = call;
	result.data = data;
	result.size = size;
	result.type = type;

	_results.push_back(result);
}

void CSteamBase::RunCallbacks()
{
	std::vector<SteamAPIResult_t>::iterator iter;

	for (iter = _results.begin(); iter < _results.end(); iter++)
	{
		SteamAPIResult_t result = *iter;

		if (_resultHandlers.find(result.call) != _resultHandlers.end())
		{
			_resultHandlers[result.call]->Run(result.data, false, result.call);
		}

		std::vector<CCallbackBase*>::iterator cbiter;

		for (cbiter = _callbacks.begin(); cbiter < _callbacks.end(); cbiter++)
		{
			CCallbackBase* cb = *cbiter;

			if (cb && cb->GetICallback() == result.type)
			{
				cb->Run(result.data, false, 0);
			}
		}

		if (result.data)
		{
			free(result.data);
		}
	}

	_results.clear();
}