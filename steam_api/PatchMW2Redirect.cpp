// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Modern Warfare 2 patches: console/log redirection and streaming
//
// Initial author: NTAuthority
// Started: 2010-09-29
// ==========================================================

#include "StdInc.h"
#include <vector>

std::vector<netadr_t> addresses;
std::vector<netadr_t> gaddresses;

cmd_function_t sv_log_add;
cmd_function_t sv_log_add2;

cmd_function_t sv_log_del;
cmd_function_t sv_log_del2;

cmd_function_t sv_log_list;
cmd_function_t sv_log_list2;

cmd_function_t sv_glog_add;
cmd_function_t sv_glog_add2;

cmd_function_t sv_glog_del;
cmd_function_t sv_glog_del2;

cmd_function_t sv_glog_list;
cmd_function_t sv_glog_list2;

extern CommandCB_t Cbuf_AddServerText_f;

// code
static char *rd_buffer;
static unsigned int rd_buffersize;
static void ( *rd_flush )( char *buffer );

void Com_BeginRedirect( char *buffer, int buffersize, void ( *flush )( char *) ) {
	if ( !buffer || !buffersize || !flush ) {
		return;
	}
	rd_buffer = buffer;
	rd_buffersize = buffersize;
	rd_flush = flush;

	*rd_buffer = 0;
}

void Com_EndRedirect( void ) {
	if ( rd_flush ) {
		rd_flush( rd_buffer );
	}

	rd_buffer = NULL;
	rd_buffersize = 0;
	rd_flush = NULL;
}

// r39 - hook com_printmessage, and not the call to - catches other print functions
StompHook comPrintMessageHook;
DWORD comPrintMessageHookLoc = 0x4AA830;
DWORD comPrintMessageHookRet = 0x4AA835;

CallHook glogprintfHook;
DWORD glogprintfHookLoc = 0x4B0218;

const char* msg;
int channel;
size_t i = 0;
void Capture() {
	//TODO: follow console filtering rules
	if(addresses.size()){
		for(i = 0; i < addresses.size(); i++){
			const char* toSend = va("%i %s", channel, msg);
			Sys_SendPacket(strlen(toSend), toSend, addresses[i]);
		}
	}
	if (rd_buffer)
	{
		if ( ( strlen( msg ) + strlen( rd_buffer ) ) > ( rd_buffersize - 1 ) ) {
			rd_flush( rd_buffer );
			*rd_buffer = 0;
		}
		strncat( rd_buffer, msg, rd_buffersize );
	}
}
void __declspec(naked) Com_PrintMessageHookStub()
{
	__asm {
		mov eax, [esp + 4 + 0]
		mov channel, eax
		mov eax, [esp + 4 + 4]
		mov msg, eax

		call Capture

		push esi
		mov  esi, [esp+0Ch]
		jmp comPrintMessageHookRet
	}
}

void __declspec(naked) GLogPrintfHookStub(){
	__asm mov msg, edx
	if(gaddresses.size()){
		for(i = 0; i < gaddresses.size(); i++){
			Sys_SendPacket(strlen(msg+7), msg+7, gaddresses[i]); //+7 to remove the timestamp
		}
	}
	__asm jmp glogprintfHook.pOriginal;
}

bool validInt(char* str){
	for(size_t i = 0; i < strlen(str); i++){
		if(str[i] < '0' || str[i] > '9'){
			return false;
		}
	}
	return true;
}


void SV_GLog_Add_f() {
	if(Cmd_Argc() != 2){
		Com_Printf (0, "USAGE: %s <IP[:Port]/Hostname[:Port]>\n", Cmd_Argv(0));
		return;
	}
	netadr_t ip;
	if(!NET_StringToAdr(Cmd_Argv(1), &ip)){
		Com_Printf (0, "Invalid address: %s\n", Cmd_Argv(1));
		return;
	}
	for(size_t i = 0; i < gaddresses.size(); i++){
		if(NET_CompareAdr(gaddresses[i], ip)){
			Com_Printf(0, "Address %s already exists (#%i)\n", Cmd_Argv(1), i);
			return;
		}
	}
	//all good
	gaddresses.push_back(ip);
	int size = gaddresses.size();
	Com_Printf(101, "Address %s (#%i) added to games_mp.log stream list\n", NET_AdrToString(gaddresses[size-1]), size-1);
}
void SV_Log_Add_f() {
	if(Cmd_Argc() != 2){
		Com_Printf (0, "USAGE: %s <IP[:Port]/Hostname[:Port]>\n", Cmd_Argv(0));
		return;
	}
	netadr_t ip;
	if(!NET_StringToAdr(Cmd_Argv(1), &ip)){
		Com_Printf (0, "Invalid address: %s\n", Cmd_Argv(1));
		return;
	}
	for(size_t i = 0; i < addresses.size(); i++){
		if(NET_CompareAdr(addresses[i], ip)){
			Com_Printf(0, "Address %s already exists (#%i)\n", Cmd_Argv(1), i);
			return;
		}
	}
	//all good
	addresses.push_back(ip);
	int size = addresses.size();
	Com_Printf(101, "Address %s (#%i) added to console_mp.log stream list\n", NET_AdrToString(addresses[size-1]), size-1);
}


void SV_GLog_Del_f() {
	if(Cmd_Argc() != 2){
		Com_Printf (0, "USAGE: %s <ID>\n", Cmd_Argv(0));
		return;
	}
	int index = 0;
	if(!validInt(Cmd_Argv(1))){
		Com_Printf (0, "%s is NaN\n", Cmd_Argv(1));
		return;
	}
	index = atoi(Cmd_Argv(1));
	if(index > -1 && index < (int)gaddresses.size()){
		Com_Printf(0, "Address %s (ID %i) removed\n", NET_AdrToString(gaddresses[index]), index);
		gaddresses.erase(gaddresses.begin()+index);
	} else {
		Com_Printf(0, "ID %i is not valid\n", index);
	}
}

void SV_Log_Del_f() {
	if(Cmd_Argc() != 2){
		Com_Printf (0, "USAGE: %s <ID>\n", Cmd_Argv(0));
		return;
	}
	int index = 0;
	if(!validInt(Cmd_Argv(1))){
		Com_Printf (0, "%s is NaN\n", Cmd_Argv(1));
		return;
	}
	index = atoi(Cmd_Argv(1));
	if(index > -1 && index < (int)addresses.size()){
		Com_Printf(0, "Address %s (ID %i) removed\n", NET_AdrToString(addresses[index]), index);
		addresses.erase(addresses.begin()+index);
	} else {
		Com_Printf(0, "ID %i is not valid\n", index);
	}
}

void SV_GLog_List_f() {
	Com_Printf(0, "# ID: Address\n");
	Com_Printf(0, "-------------\n");
	for(size_t i = 0; i < gaddresses.size(); i++) {
		Com_Printf(0, "#%03d: %5s\n", i,  NET_AdrToString(gaddresses[i]));
	}
}
void SV_Log_List_f() {
	Com_Printf(0, "# ID: Address\n");
	Com_Printf(0, "-------------\n");
	for(size_t i = 0; i < addresses.size(); i++) {
		Com_Printf(0, "#%03d: %5s\n", i,  NET_AdrToString(addresses[i]));
	}
}

StompHook fsBuildOSPathForThreadHook;
DWORD fsBuildOSPathForThreadHookLoc = 0x642139;
DWORD fsBuildOSPathForThreadHookLocRet = 0x64213F;

static char* writeFile;
static char* writeFolder;
dvar_t* iw4m_onelog;

void FS_BuildOSPathForThreadHookTest()
{
	dvar_t* g_log = *(dvar_t**)0x1A45D9C;

	if (g_log && strcmp(writeFile, g_log->current.string) == 0)
	{
		if (strcmp(writeFolder, "m2demo") != 0)
		{
			if (iw4m_onelog->current.boolean)
			{
				strcpy_s(writeFolder, 256, "m2demo");
			}
		}
	}
}
void __declspec(naked) FS_BuildOSPathForThreadHookFunc()
{
	__asm
	{
		mov eax, [esp + 8h]
		mov writeFolder, eax
		mov eax, [esp + 0Ch]
		mov writeFile, eax
	}

	FS_BuildOSPathForThreadHookTest();

	__asm
	{
		mov eax, [esp + 8h]
		push ebp
		push esi
		mov esi, [esp + 0Ch]

		jmp fsBuildOSPathForThreadHookLocRet
	}
}

void PatchMW2_Redirect()
{
	comPrintMessageHook.initialize(comPrintMessageHookLoc, Com_PrintMessageHookStub);
	comPrintMessageHook.installHook();

	glogprintfHook.initialize(glogprintfHookLoc, GLogPrintfHookStub);
	glogprintfHook.installHook();

	fsBuildOSPathForThreadHook.initialize(fsBuildOSPathForThreadHookLoc, FS_BuildOSPathForThreadHookFunc);
	fsBuildOSPathForThreadHook.installHook();

	Cmd_AddCommand("log_add", Cbuf_AddServerText_f, &sv_log_add, 0);
	Cmd_AddServerCommand("log_add", SV_Log_Add_f, &sv_log_add2);

	Cmd_AddCommand("log_del", Cbuf_AddServerText_f, &sv_log_del, 0);
	Cmd_AddServerCommand("log_del", SV_Log_Del_f, &sv_log_del2);
	
	Cmd_AddCommand("log_list", Cbuf_AddServerText_f, &sv_log_list, 0);
	Cmd_AddServerCommand("log_list", SV_Log_List_f, &sv_log_list2);

	Cmd_AddCommand("g_log_add", Cbuf_AddServerText_f, &sv_glog_add, 0);
	Cmd_AddServerCommand("g_log_add", SV_GLog_Add_f, &sv_glog_add2);

	Cmd_AddCommand("g_log_del", Cbuf_AddServerText_f, &sv_glog_del, 0);
	Cmd_AddServerCommand("g_log_del", SV_GLog_Del_f, &sv_glog_del2);
	
	Cmd_AddCommand("g_log_list", Cbuf_AddServerText_f, &sv_glog_list, 0);
	Cmd_AddServerCommand("g_log_list", SV_GLog_List_f, &sv_glog_list2);

	iw4m_onelog = (dvar_t*)Dvar_RegisterBool("iw4m_onelog", false, DVAR_FLAG_LATCHED || DVAR_FLAG_SAVED, "Only write the game log to the 'm2demo' OS folder");
}