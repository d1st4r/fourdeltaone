// ==========================================================
// alterIWnet project
// 
// Component: xnp
// Sub-component: libnp
// Purpose: external include file
//
// Initial author: NTAuthority
// Started: 2011-06-28
// ==========================================================

#pragma once

#include "NPTypeDefs.h"
#include "NPLists.h"

#include "NPAsync.h"
#include "NPAuthenticate.h"
#include "NPStorage.h"
#include "NPFriends.h"
#include "NPServers.h"

// ----------------------------------------------------------
// Initialization/shutdown functions
// ----------------------------------------------------------

// starts up the network platform functions
LIBNP_API bool LIBNP_CALL NP_Init();

// cleans up and shuts down the network platform
LIBNP_API bool LIBNP_CALL NP_Shutdown();

// connects to a NP server
LIBNP_API bool LIBNP_CALL NP_Connect(const char* server, uint16_t port);

// log output callback
typedef void (LIBNP_CALL * NPLogCB)(const char* message);

// sets the output callback for log messages
LIBNP_API void LIBNP_CALL NP_SetLogCallback(NPLogCB callback);

// ----------------------------------------------------------
// Dictionary type creation
// ----------------------------------------------------------

// creates a string/string dictionary
// use NPDictionary constructor instead!
LIBNP_API NPDictionaryInternal* LIBNP_CALL NP_CreateDictionary();

// ----------------------------------------------------------
// Callback handling
// ----------------------------------------------------------

// handles and dispatches callbacks
// must be called every frame since it now handles sockets
LIBNP_API bool LIBNP_CALL NP_RunFrame();

// ----------------------------------------------------------
// Authentication service
// ----------------------------------------------------------

// authenticates using an external auth token
LIBNP_API NPAsync<NPAuthenticateResult>* LIBNP_CALL NP_AuthenticateWithToken(const char* authToken);

// authenticates using a username/password
LIBNP_API NPAsync<NPAuthenticateResult>* LIBNP_CALL NP_AuthenticateWithDetails(const char* username, const char* password);

// authenticates using a license key
LIBNP_API NPAsync<NPAuthenticateResult>* LIBNP_CALL NP_AuthenticateWithLicenseKey(const char* licenseKey);

// registers a game server license key
LIBNP_API NPAsync<NPRegisterServerResult>* LIBNP_CALL NP_RegisterServer(const char* configPath);

// validates a user ticket
LIBNP_API NPAsync<NPValidateUserTicketResult>* LIBNP_CALL NP_ValidateUserTicket(const void* ticket, size_t ticketSize, uint32_t clientIP, NPID clientID);

// obtains a user ticket for server authentication
LIBNP_API bool LIBNP_CALL NP_GetUserTicket(void* buffer, size_t bufferSize, NPID targetServer);

// gets the NPID for the current client. returns false (and does not change the output buffer) if not yet authenticated
LIBNP_API bool LIBNP_CALL NP_GetNPID(NPID* pID);

// gets the user group for the current client, returns 0 if not authenticated
LIBNP_API int LIBNP_CALL NP_GetUserGroup();

// function to register a callback to kick a client by NPID
LIBNP_API void LIBNP_CALL NP_RegisterKickCallback(void (__cdecl * callback)(NPID, const char*));

// function to register a callback when external authentication status changes
LIBNP_API void LIBNP_CALL NP_RegisterEACallback(void (__cdecl * callback)(EExternalAuthState));

// loads the game DLL for the specified version number
LIBNP_API void* LIBNP_CALL NP_LoadGameModule(int version);

// ----------------------------------------------------------
// Storage service
// ----------------------------------------------------------

// obtains a file from the remote global per-title storage
LIBNP_API NPAsync<NPGetPublisherFileResult>* LIBNP_CALL NP_GetPublisherFile(const char* fileName, uint8_t* buffer, size_t bufferLength);

// obtains a file from the remote per-user storage
LIBNP_API NPAsync<NPGetUserFileResult>* LIBNP_CALL NP_GetUserFile(const char* fileName, NPID npID, uint8_t* buffer, size_t bufferLength);

// uploads a file to the remote per-user storage
LIBNP_API NPAsync<NPWriteUserFileResult>* LIBNP_CALL NP_WriteUserFile(const char* fileName, NPID npID, const uint8_t* buffer, size_t bufferLength);

// sends a random string to the NP server
LIBNP_API void LIBNP_CALL NP_SendRandomString(const char* str);

// ----------------------------------------------------------
// Friends service
// ----------------------------------------------------------

// sets the external platform ID for Steam
LIBNP_API void LIBNP_CALL NP_SetExternalSteamID(uint64_t steamID);

// obtains profile data for a list of NPIDs
LIBNP_API NPAsync<NPGetProfileDataResult>* LIBNP_CALL NP_GetProfileData(uint32_t numIDs, const NPID* npIDs, NPProfileData* outData);

// returns if the friends API is available
LIBNP_API bool LIBNP_CALL NP_FriendsConnected();

// gets the number of registered friends
LIBNP_API uint32_t LIBNP_CALL NP_GetNumFriends();

// gets a specific friend's NPID - index is from 0...[NP_GetNumFriends() - 1]
LIBNP_API NPID LIBNP_CALL NP_GetFriend(int32_t index);

// gets the name for a friend
// will only work with NPIDs known to the client - currently only if they're friends
LIBNP_API const char* LIBNP_CALL NP_GetFriendName(NPID npID);

// gets the presence state for a friend
LIBNP_API EPresenceState LIBNP_CALL NP_GetFriendPresence(NPID npID);

// sets a presence key/value pair
// a value of NULL removes the key, if existent
LIBNP_API void LIBNP_CALL NP_SetRichPresence(const char* key, const char* value);

// sets the presence body
LIBNP_API void LIBNP_CALL NP_SetRichPresenceBody(const char* body);

// uploads the rich presence data
LIBNP_API void LIBNP_CALL NP_StoreRichPresence();

// gets a rich presence value for a friend
// will only work with friends, not other known NPIDs
LIBNP_API const char* LIBNP_CALL NP_GetFriendRichPresence(NPID npID, const char* key);

// gets the rich presence body for a friend
LIBNP_API const char* LIBNP_CALL NP_GetFriendRichPresenceBody(NPID npID);

// gets an avatar for any client
LIBNP_API NPAsync<NPGetUserAvatarResult>* LIBNP_CALL NP_GetUserAvatar(int id, uint8_t* buffer, size_t bufferLength);

// ----------------------------------------------------------
// Server list service
// ----------------------------------------------------------

// creates a remote session
LIBNP_API NPAsync<NPCreateSessionResult>* LIBNP_CALL NP_CreateSession(NPSessionInfo* data);

// updates a session
LIBNP_API NPAsync<EServersResult>* LIBNP_CALL NP_UpdateSession(NPSID sid, NPSessionInfo* data);

// deletes a session
LIBNP_API NPAsync<EServersResult>* LIBNP_CALL NP_DeleteSession(NPSID sid);

// refreshes the session list - tags are separated by single spaces
LIBNP_API NPAsync<bool>* LIBNP_CALL NP_RefreshSessions(NPDictionary& infos);

// gets the number of sessions
LIBNP_API int32_t LIBNP_CALL NP_GetNumSessions();

// gets a single session's info
LIBNP_API void LIBNP_CALL NP_GetSessionData(int32_t index, NPSessionInfo* out);

// ----------------------------------------------------------
// Messaging service
// ----------------------------------------------------------

// sends arbitrary data to a client
LIBNP_API void LIBNP_CALL NP_SendMessage(NPID npid, const uint8_t* data, uint32_t length);

// function to register a callback when a message is received
// arguments: source NPID, data, length
LIBNP_API void LIBNP_CALL NP_RegisterMessageCallback(void (__cdecl * callback)(NPID, const uint8_t*, uint32_t));