// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: ISteamGameServer009 implementation
//
// Initial author: NTAuthority
// Started: 2010-09-28
// ==========================================================

#include "StdInc.h"
#include "SteamGameServer009.h"

// TODO: cause Steam auth to be requested even after a map change (server reinit)
static void SteamGS_AddToGroupQueue(NPID npid, int groupID)
{
	char currentBuffer[8192] = { 0 };
	char thisID[1024];
	bool isStaff = (groupID == 12 || groupID == 10 || groupID == 4 || groupID == 11 || groupID == 22 || groupID == 5);

	dvar_t* scr_groupQueue = Dvar_FindVar("scr_groupQueue");

	if (scr_groupQueue)
	{
		strcpy_s(currentBuffer, sizeof(currentBuffer), scr_groupQueue->current.string);
	}

	if (isStaff)
	{
		sprintf(thisID, "%16llx %d ", npid, (isStaff) ? 1 : 0);
		strcat(currentBuffer, thisID);
	}

	Cmd_ExecuteSingleCommand(0, 0, va("set \"scr_groupQueue\" \"%s\"", currentBuffer));
}

static void SteamGS_OnValidateTicket(NPAsync<NPValidateUserTicketResult>* async)
{
	NPValidateUserTicketResult* result = async->GetResult();

#ifndef KEY_DISABLED
	if (result->result == ValidateUserTicketResultOK)
	{
		GSClientApprove_t* retvals = (GSClientApprove_t*)malloc(sizeof(GSClientApprove_t));
		retvals->m_SteamID = CSteamID(result->id);
		CSteamBase::ReturnCall(retvals, sizeof(GSClientApprove_t), GSClientApprove_t::k_iCallback, 0);

		SteamGS_AddToGroupQueue(result->id, result->groupID);
	}
	else
	{
		GSClientDeny_t* retvals = (GSClientDeny_t*)malloc(sizeof(GSClientDeny_t));

		retvals->m_SteamID = CSteamID(result->id);
		retvals->m_eDenyReason = k_EDenyNoLicense;

		CSteamBase::ReturnCall(retvals, sizeof(GSClientDeny_t), GSClientDeny_t::k_iCallback, 0);
	}
#else
	GSClientApprove_t* retvals = (GSClientApprove_t*)malloc(sizeof(GSClientApprove_t));
	retvals->m_SteamID = CSteamID(result->id);
	CSteamBase::ReturnCall(retvals, sizeof(GSClientApprove_t), GSClientApprove_t::k_iCallback, 0);

	SteamGS_AddToGroupQueue(result->id, result->groupID);
#endif
}

void CSteamGameServer009::LogOn() {
	
}

void CSteamGameServer009::LogOff() {
	
}

bool CSteamGameServer009::LoggedOn() {
	
	return true;
}

bool CSteamGameServer009::Secure() {
	
	return false;
}

CSteamID CSteamGameServer009::GetSteamID() {
	
	NPID npID;
	NP_GetNPID(&npID);
	return CSteamID(npID);
}

bool CSteamGameServer009::SendUserConnectAndAuthenticate( uint32 unIPClient, const void *pvAuthBlob, uint32 cubAuthBlobSize, CSteamID *pSteamIDUser ) {
	// this case was for legacy clients. screw them. with a big screw.
	/*if (cubAuthBlobSize == 4)
	{
		GSClientApprove_t* retvals = (GSClientApprove_t*)malloc(sizeof(GSClientApprove_t));
		retvals->m_SteamID = CSteamID(*(int*)pvAuthBlob, 1, k_EUniversePublic, k_EAccountTypeIndividual);
		CSteamBase::ReturnCall(retvals, sizeof(GSClientApprove_t), GSClientApprove_t::k_iCallback, 0);

		return true;
	}*/

#ifdef KEY_DISABLED
	NPAuthenticateTicket* ticket = (NPAuthenticateTicket*)pvAuthBlob;

	GSClientApprove_t* retvals = (GSClientApprove_t*)malloc(sizeof(GSClientApprove_t));
	retvals->m_SteamID = CSteamID(ticket->clientID);
	CSteamBase::ReturnCall(retvals, sizeof(GSClientApprove_t), GSClientApprove_t::k_iCallback, 0);
	return true;
#endif

	NPAsync<NPValidateUserTicketResult>* async = NP_ValidateUserTicket(pvAuthBlob, cubAuthBlobSize, unIPClient, pSteamIDUser->ConvertToUint64());
	async->SetCallback(SteamGS_OnValidateTicket, NULL);
	return true;
}

CSteamID CSteamGameServer009::CreateUnauthenticatedUserConnection() {
	
	NPID npID;
	NP_GetNPID(&npID);
	return CSteamID(npID);
}

void CSteamGameServer009::SendUserDisconnect( CSteamID steamIDUser ) {
	
}

bool CSteamGameServer009::UpdateUserData( CSteamID steamIDUser, const char *pchPlayerName, uint32 uScore ) {
	
	return true;
}

bool CSteamGameServer009::SetServerType( uint32 unServerFlags, uint32 unGameIP, uint16 unGamePort, uint16 unSpectatorPort, uint16 usQueryPort, const char *pchGameDir, const char *pchVersion, bool bLANMode ) {
	
	return true;
}

void CSteamGameServer009::UpdateServerStatus( int cPlayers, int cPlayersMax, int cBotPlayers, const char *pchServerName, const char *pSpectatorServerName, const char *pchMapName ) {
	
}

void CSteamGameServer009::UpdateSpectatorPort( uint16 unSpectatorPort ) { }

void CSteamGameServer009::SetGameType( const char *pchGameType ) { }

bool CSteamGameServer009::GetUserAchievementStatus( CSteamID steamID, const char *pchAchievementName ) {
	
	return false;
}

void CSteamGameServer009::GetGameplayStats( ) {}

bool CSteamGameServer009::RequestUserGroupStatus( CSteamID steamIDUser, CSteamID steamIDGroup ) {
	
	return false;
}

uint32 CSteamGameServer009::GetPublicIP() {
	
	return 0;
}

void CSteamGameServer009::SetGameData( const char *pchGameData) {
	
}

EUserHasLicenseForAppResult CSteamGameServer009::UserHasLicenseForApp( CSteamID steamID, AppId_t appID ) {
	return k_EUserHasLicenseResultHasLicense;
}
