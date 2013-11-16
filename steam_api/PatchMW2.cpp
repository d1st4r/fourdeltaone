// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Modern Warfare 2 patches
//
// Initial author: NTAuthority
// Started: 2010-09-27
// ==========================================================

#include "StdInc.h"
#include <time.h>
#include <winsock.h>
#include <dbghelp.h>
#include <shellapi.h>

#define NO_UPNP
#define IW4M_DEV_BUILD
#define NETRT_DLL "netrt." BUILDNUMBER_STR ".dll"

void PatchMW2_159();
void Auth_VerifyIdentity();
void SteamProxy_Init();
void DLCInstaller_Run();

void Sys_RunInit()
{
	PatchMW2();
}

void PatchMW2()
{
	// check version
	if (!strcmp((char*)0x6E9638, "177"))
	{ // 1.0.159 (iw4m) version-specific address
		DLCInstaller_Run();
		DetermineGameFlags();
		PatchMW2_159();

		InitProfile();

		if (!GAME_FLAG(GAME_FLAG_DEDICATED))
		{
			Auth_VerifyIdentity();

			if (GetProcAddress(GetModuleHandle("kernel32.dll"), "InitializeSRWLock"))
			{
				LoadLibrary("gdipp_client_32.dll");
				#ifdef IW4M_DEV_BUILD
				LoadLibrary(NETRT_DLL);
				#endif
			}
		}
		return;
	}

	TerminateProcess(GetCurrentProcess(), 0);
}

CallHook winMainInitHook;
DWORD winMainInitHookLoc = 0x4513D0;

void InstallCrashHandler();

#pragma optimize("", off)
void __declspec(naked) WinMainInitHookStub()
{
	InstallCrashHandler();

	__asm {
		jmp winMainInitHook.pOriginal
	}
}
#pragma optimize("", on)

CallHook sysInitHook;
DWORD sysInitHookLoc = 0x60BDBF;

void DoSysInit();

#pragma optimize("", off)
void __declspec(naked) SysInitHookStub()
{
	DoSysInit();

	__asm
	{
		jmp sysInitHook.pOriginal
	}
}
#pragma optimize("", on)

void PatchMW2_FFHash();
void PatchMW2_Modding();
void PatchMW2_Dedicated();
void PatchMW2_Status();
void PatchMW2_Redirect();
void PatchMW2_Servers();
void PatchMW2_Load();
void PatchMW2_MatchData();
void PatchMW2_Console();
void PatchMW2_Hello();
void PatchMW2_Experimental();
void PatchMW2_Client();
void PatchMW2_Prefix();
void PatchMW2_AssetRestrict();
void PatchMW2_ClientConsole();
void PatchMW2_NoBorder();
void PatchMW2_OOB();
void PatchMW2_LogInitGame();
void PatchMW2_RemoteConsoleClient();
void PatchMW2_SPMaps();
void PatchMW2_ProtocolSix();
void PatchMW2_Materialism();
void PatchMW2_Branding();
void PatchMW2_ScriptCompileDebug();
void PatchMW2_Stats();
void PatchMW2_DownloadClient();
void PatchMW2_DownloadServer();
void PatchMW2_InGameMenu();
void PatchMW2_Legacy();
void PatchMW2_FXAA();
void PatchMW2_UILoading();
void PatchMW2_Playlists();
void PatchMW2_StringTable();
void PatchMW2_DemoRecording();
void PatchMW2_ScoreboardInfo();
void PatchMW2_ServerList();
void PatchMW2_WeaponCamos();
void PatchMW2_MatchRecord();
void PatchMW2_Script();
void PatchMW2_SuicideMessages();
void PatchMW2_NewsTicker();
void PatchMW2_Dvars();
void PatchMW2_ClientCommands();
void PatchMW2_Loadscreens();
void PatchMW2_UIScripts();
void PatchMW2_ModList();
void PatchMW2_Extrasensory();
void PatchMW2_SayCommands();
void PatchMW2_XPBar();
void PatchMW2_ClientDvar();
void PatchMW2_Friends();
void PatchMW2_VA();
void PatchMW2_MusicalTalent();
void PatchMW2_PreAuthenticate();
void PatchMW2_LocalizedStrings();
void PatchMW2_NUI();
void PatchMW2_OneThread();
void PatchMW2_StringList();
void PatchMW2_RecoverDevice();
void PatchMW2_CModels();
void PatchMW2_PartyBypass();
void PatchMW2_Maxclients();
void PatchMW2_RecordingSV();
void PatchMW2_FrameTime();
void PatchMW2_FrameRate();
void PatchMW2_UserAvatars();
void PatchMW2_PartialWeaponry();
auto PatchMW2_ArenaLength() -> void;

int ahfunc(int nAllocType, void* pvData, size_t nSize, int nBlockUse, long lRequest, const unsigned char* szFileName, int nLine)
{
	if ( nBlockUse == _CRT_BLOCK )
		return( TRUE );

	if (nAllocType == _HOOK_ALLOC || nAllocType == _HOOK_REALLOC)
	{
		if (nSize > 16384)
		{
			OutputDebugString(va("allocating %i bytes from %s:%i\n", nSize, szFileName, nLine));
		}
	}

	return TRUE;
}

void PatchMW2_159()
{
	//_CrtSetAllocHook(ahfunc);

	// protocol version (workaround for hacks)
	*(int*)0x4FB501 = PROTOCOL; // was 8E

	// protocol command
	*(int*)0x4D36A9 = PROTOCOL; // was 8E
	*(int*)0x4D36AE = PROTOCOL; // was 8E
	*(int*)0x4D36B3 = PROTOCOL; // was 8E

	// remove system pre-init stuff (improper quit, disk full)
	*(BYTE*)0x411350 = 0xC3;

	// remove STEAMSTART checking for DRM IPC
	memset((void*)0x451145, 0x90, 5);
	*(BYTE*)0x45114C = 0xEB;

	// master server
	strcpy((char*)0x6D9CBC, "iw4.prod.fourdeltaone.net");

	// internal version is 99, most servers should accept it
	*(int*)0x463C61 = 99;

	// patch web1 server
	const char* webName = "http://web1.pc.iw4.iwnet.infinityward.com:13000/pc/";

	*(DWORD*)0x4D4800 = (DWORD)webName;
	*(DWORD*)0x4D481F = (DWORD)webName;

	// winmain
	winMainInitHook.initialize(winMainInitHookLoc, WinMainInitHookStub);
	winMainInitHook.installHook();

	// Sys_Init
	sysInitHook.initialize(sysInitHookLoc, SysInitHookStub);
	sysInitHook.installHook();

	// always enable system console, not just if generating reflection probes
	memset((void*)0x60BB58, 0x90, 11);

	// disable the IWNet IP detection (default 'got ipdetect' flag to 1)
	*(BYTE*)0x649D6F0 = 1;

	// LSP disabled
	*(BYTE*)0x435950 = 0xC3; // LSP HELLO
	*(BYTE*)0x49C220 = 0xC3; // We wanted to send a logging packet, but we haven't connected to LSP!
	*(BYTE*)0x4BD900 = 0xC3; // main LSP response func
	*(BYTE*)0x682170 = 0xC3; // Telling LSP that we're playing a private match

	*(const char**)0x51F5A6 = "Could not load image '%s'. If this image name starts with 'preview_mp_', you might be missing iw_22.iwd/iw_23.iwd. Read the relevant FAQ entry on http://fourdeltaone.net/#/support :)";

	// always call _fpmath callback
	*(WORD*)0x6B8B9F = 0x9090;

	// more detailed patches
	PatchMW2_FFHash();
	PatchMW2_Modding();
	PatchMW2_Prefix();
	PatchMW2_Status();
	PatchMW2_OOB();
	PatchMW2_LogInitGame();
	PatchMW2_Redirect();
	PatchMW2_Servers();
	PatchMW2_Load();
	PatchMW2_Hello();
	PatchMW2_Experimental();
	PatchMW2_AssetRestrict();
	PatchMW2_SPMaps();
	PatchMW2_Materialism();
	PatchMW2_Branding();
	PatchMW2_ScriptCompileDebug();
	PatchMW2_Stats();
	PatchMW2_DownloadClient();
	PatchMW2_DownloadServer();
	PatchMW2_InGameMenu();
	PatchMW2_FXAA();
	PatchMW2_UILoading();
	PatchMW2_Playlists();
	PatchMW2_StringTable();
	PatchMW2_DemoRecording();
	PatchMW2_ScoreboardInfo();
	PatchMW2_UIScripts();
	PatchMW2_ServerList();
	PatchMW2_WeaponCamos();
	PatchMW2_Script();
	PatchMW2_MatchRecord();
	//PatchMW2_FifthInfinity();
	PatchMW2_CModels();
	PatchMW2_SuicideMessages();
	PatchMW2_NewsTicker();
	PatchMW2_Dvars();
	PatchMW2_ClientCommands();
	PatchMW2_ModList();
	PatchMW2_SayCommands();
	//PatchMW2_Extrasensory();
	PatchMW2_Legacy();
	PatchMW2_ProtocolSix();
	PatchMW2_XPBar();
	PatchMW2_ClientDvar();
	PatchMW2_VA();
	PatchMW2_PreAuthenticate();
	PatchMW2_LocalizedStrings();
	PatchMW2_StringList();
	PatchMW2_PartyBypass();
#ifdef ENABLE_MAX_CLIENTS_PATCH
	PatchMW2_Maxclients();
#endif
	PatchMW2_RecordingSV();
	PatchMW2_FrameTime();
	PatchMW2_UserAvatars();
	PatchMW2_PartialWeaponry();
	PatchMW2_ArenaLength();
	//PatchMW2_FrameRate();

	bool nativeConsole = GAME_FLAG(GAME_FLAG_CONSOLE);

	if (GAME_FLAG(GAME_FLAG_DEDICATED))
	{
		PatchMW2_Dedicated();
		nativeConsole = !nativeConsole;
	}
	else
	{
		PatchMW2_Client();
		PatchMW2_ClientConsole();
		PatchMW2_NoBorder();
		PatchMW2_RemoteConsoleClient();
		PatchMW2_Loadscreens();
		PatchMW2_Friends();
		PatchMW2_MusicalTalent();
		PatchMW2_OneThread();
		PatchMW2_RecoverDevice();
	}

	if (nativeConsole)
	{
		PatchMW2_Console();
	}
	else
	{
		FreeConsole();
	}
}

// smaller patches go below

// patches fastfile integrity checking
void PatchMW2_FFHash()
{
	// basic checks (hash jumps, both normal and playlist)
	*(WORD*)0x5B97A3 = 0x9090;
	*(WORD*)0x5BA493 = 0x9090;

	*(WORD*)0x5B991C = 0x9090;
	*(WORD*)0x5BA60C = 0x9090;

	*(WORD*)0x5B97B4 = 0x9090;
	*(WORD*)0x5BA4A4 = 0x9090;

	// some other, unknown, check
	*(BYTE*)0x5B9912 = 0xB8;
	*(DWORD*)0x5B9913 = 1;

	*(BYTE*)0x5BA602 = 0xB8;
	*(DWORD*)0x5BA603 = 1;

	if (IsDebuggerPresent())
	{
		// dirty disk breakpoint
		*(BYTE*)0x4CF7F0 = 0xCC;
	}
}

CallHook execIsFSHook;
DWORD execIsFSHookLoc = 0x6098FD;

// note this function has to return 'int', as otherwise only the bottom byte will get cleared
int ExecIsFSHookFunc(const char* execFilename, const char* dummyMatch) { // dummyMatch isn't used by us
	// check if the file exists in our FS_* path
	if (FS_ReadFile(execFilename, NULL) >= 0)
	{
		return false;
	}

	return true;
}

#pragma optimize("", off)
void __declspec(naked) ExecIsFSHookStub() {
	__asm jmp ExecIsFSHookFunc
}
#pragma optimize("", on)

DWORD pureShouldBeZero = 0;

// patches fs_game/IWD script support
void PatchMW2_Modding()
{
	// remove limit on IWD file loading
	//memset((void*)0x643B94, 0x90, 6);
	*(BYTE*)0x642BF3 = 0xEB;

	// remove convar write protection (why?)
	//*(BYTE*)0x647DD4 = 0xEB;


	// kill most of pure (unneeded in 159, 180+ messed it up)
	//memset((void*)0x45513D, 0x90, 5);
	//memset((void*)0x45515B, 0x90, 5);
	//memset((void*)0x45516C, 0x90, 5);

	//memset((void*)0x45518E, 0x90, 5);
	//memset((void*)0x45519F, 0x90, 5);

	//*(BYTE*)0x449089 = 0xEB;

	// other IWD things (pure?)
	//*(BYTE*)0x4C5E7B = 0xEB;
	//*(BYTE*)0x465107 = 0xEB;

	// default sv_pure to 0
	// TODO: implement client-side downloading/default to 1 for no-mods
	*(BYTE*)0x4D3A74 = 0;

	// disable sv_pure for .iwi/.wav/.mp3-only IWDs (for now we only want to get rid of scripts)
	/*strcpy((char*)0x6F1DBC, ".iwi");
	strcpy((char*)0x6E1B94, ".wav");
	strcpy((char*)0x71325C, ".mp3");*/

	// disable configstring checksum matching (it's unreliable at most)
	//*(BYTE*)0x4A75A7 = 0xEB; // SV_SpawnServer
	//*(BYTE*)0x5AC2CF = 0xEB; // CL_ParseGamestate
	*(BYTE*)0x5AC2C3 = 0xEB; // CL_ParseGamestate

	// steam stuff (steam authentication)
	*(DWORD*)0x414ACC = 0x90909090;
	*(WORD*)0x414AD0 = 0x9090;

	// hopefully allow alt-tab during game, used at least in alt-enter handling
	*(BYTE*)0x45ACE0 = 0xB0;
	*(BYTE*)0x45ACE1 = 0x01;
	*(BYTE*)0x45ACE2 = 0xC3;

	// set cg_scoreboardPingText to 1
	*(BYTE*)0x45888E = 1;

	// disable migration_dvarErrors
	*(BYTE*)0x60BDA7 = 0;

	// allow vid_restart even if 'connected'
	memset((void*)0x4CA1FA, 0x90, 6);

	// remove 'impure stats' checking
	*(BYTE*)0x4BB250 = 0x33;
	*(BYTE*)0x4BB251 = 0xC0;
	*(DWORD*)0x4BB252 = 0xC3909090;

	// remove fs_game profiles
	*(WORD*)0x4A5D74 = 0x9090;

	// fs_game crash fix removing some calls
	// (NOTE: CoD4 comparison shows this is related to LoadObj weaponDefs, might fix the crash we're having)
	*(BYTE*)0x452C1D = 0xEB;

	// remove fs_game check for moddable rawfiles - allows non-fs_game to modify rawfiles
	*(WORD*)0x61AB76 = 0x9090;

	// kill filesystem init default_mp.cfg check -- IW made it useless while moving .cfg files to fastfiles
	// and it makes fs_game crash

	// not nopping everything at once, there's cdecl stack cleanup in there
	memset((void*)0x461A9E, 0x90, 5);
	memset((void*)0x461AAA, 0x90, 5);
	memset((void*)0x461AB2, 0x90, 0xB1);

	// for some reason fs_game != '' makes the game load mp_defaultweapon, which does not exist in MW2 anymore as a real asset
	// kill the call and make it act like fs_game == ''
	// UPDATE 2010-09-12: this is why CoD4 had text weapon files, those are used with fs_game.
	// CLARIFY 2010-09-27: we don't have textual weapon files, as such we should load them from fastfile as usual
	// TODO: change this into a delay-loading hook for fastfile/loadobj (2011-05-20)
	*(BYTE*)0x4081FD = 0xEB;

	// exec fixing
	execIsFSHook.initialize(execIsFSHookLoc, ExecIsFSHookStub);
	execIsFSHook.installHook();

	// exec whitelist removal (YAYFINITY WARD)
	memset((void*)0x609685, 0x90, 5);
	*(WORD*)0x60968C = 0x9090;

	// allow joining 'developer 1' servers
	*(BYTE*)0x478BA2 = 0xEB;
}

void DoWinMainInit();

LONG WINAPI CustomUnhandledExceptionFilter(LPEXCEPTION_POINTERS ExceptionInfo)
{
	// step 1: write minidump
	static LPEXCEPTION_POINTERS exceptionData;

	exceptionData = ExceptionInfo;

	// create a temporary stack for these calls
	DWORD* tempStack = new DWORD[16000];
	static DWORD* origStack;

	__asm
	{
		mov origStack, esp
		mov esp, tempStack
		add esp, 0FA00h
		sub esp, 1000h // local stack space over here, sort of
	}

	char error[1024];
	char filename[MAX_PATH];
	__time64_t time;
	tm* ltime;

	_time64(&time);
	ltime = _localtime64(&time);
	strftime(filename, sizeof(filename) - 1, "iw4m-" VERSION "-%Y%m%d%H%M%S.dmp", ltime);
	_snprintf(error, sizeof(error) - 1, "A minidump has been written to %s.", filename);

	HANDLE hFile = CreateFile(filename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		MINIDUMP_EXCEPTION_INFORMATION ex;
		memset(&ex, 0, sizeof(ex));
		ex.ThreadId = GetCurrentThreadId();
		ex.ExceptionPointers = exceptionData;
		ex.ClientPointers = FALSE;

		MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ex, NULL, NULL);		

		CloseHandle(hFile);
	}
	else
	{
		_snprintf(error, sizeof(error) - 1, "An error (0x%x) occurred during creating %s.", GetLastError(), filename);
	}

	// step 2: exit the application
	/*if (exceptionData->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW)
	{
		Com_Printf(0, "Termination because of a stack overflow.\n");

		TerminateProcess(GetCurrentProcess(), EXCEPTION_STACK_OVERFLOW);
	}
	else
	{
		Com_Error(0, "Fatal error (0x%08x) at 0x%08x.\n%s", ExceptionInfo->ExceptionRecord->ExceptionCode, ExceptionInfo->ExceptionRecord->ExceptionAddress, error);	
	}*/

	__asm
	{
		mov esp, origStack
	}

	delete[] tempStack;

	return 0;
}

static void CrashStack()
{
	CrashStack();
}

void InstallCrashHandler() {
	DoWinMainInit();

	SetUnhandledExceptionFilter(&CustomUnhandledExceptionFilter);
	//CrashStack();
}

void ClientConsole_SetAutoComplete();

void DoSysInit()
{
	ClientConsole_SetAutoComplete();
}

// gethostbyname hook
dvar_t** masterServerName = (dvar_t**)0x1AD8F48;

char* serverName = NULL;
char* webName = NULL;

unsigned int oneAtATimeHash(char* inpStr)
{
	unsigned int value = 0,temp = 0;
	for(size_t i=0;i<strlen(inpStr);i++)
	{
		char ctext = tolower(inpStr[i]);
		temp = ctext;
		temp += value;
		value = temp << 10;
		temp += value;
		value = temp >> 6;
		value ^= temp;
	}
	temp = value << 3;
	temp += value;
	unsigned int temp2 = temp >> 11;
	temp = temp2 ^ temp;
	temp2 = temp << 15;
	value = temp2 + temp;
	if(value < 2) value += 2;
	return value;
}

hostent* WINAPI custom_gethostbyname(const char* name) {
	// if the name is IWNet's stuff...
	unsigned int ip1 = oneAtATimeHash("ip1.pc.iw4.iwnet.infinityward.com");
	unsigned int log1 = oneAtATimeHash("log1.pc.iw4.iwnet.infinityward.com");
	unsigned int match1 = oneAtATimeHash("match1.pc.iw4.iwnet.infinityward.com");
	unsigned int web1 = oneAtATimeHash("web1.pc.iw4.iwnet.infinityward.com");
	unsigned int blob1 = oneAtATimeHash("blob1.pc.iw4.iwnet.infinityward.com");

	unsigned int current = oneAtATimeHash((char*)name);
	char* hostname = (char*)name;

	if (current == log1 || current == match1 || current == blob1 || current == ip1 || current == web1) {
		hostname = (*masterServerName)->current.string;
	}

	return gethostbyname(hostname);
}

void PatchMW2_Servers()
{
	// gethostbyname
	*(DWORD*)0x6D7458 = (DWORD)custom_gethostbyname;
}

// HELLO WORLD function
CallHook helloHook;
DWORD helloHookLoc = 0x60BB01;

CallHook helloHook2;
DWORD helloHook2Loc = 0x60A946;

void HelloIW(int type)
{
	Com_Printf(type, "%s built on %s %s\n", VERSIONSTRING, __DATE__, __TIME__);
	Com_Printf(type, "http://fourdeltaone.net/iw4m\n");
	Com_Printf(type, "\"Variety is the next big thing for us. We're going in deep, and we're going in hard.\"\n");
	Com_Printf(type, "  -- Robert Bowling, former Creative Strategist at Infinity Ward (during IW3 credits)\n");
	//Com_Printf(type, "\"Lead best team in the world in the creation of new gameplay experiences (with dedis ;))\"\n");
	//Com_Printf(type, "-- Jason West, President of Respawn Entertainment (on the initial biography page, showing his dedication to the community)");
}

#pragma optimize("", off)
void __declspec(naked) HelloHookStub()
{
	HelloIW(0);
	__asm retn
}

void __declspec(naked) HelloHook2Stub()
{
	HelloIW(16);
	__asm jmp helloHook2.pOriginal
}
#pragma optimize("", on)

void PatchMW2_Hello()
{
	helloHook.initialize(helloHookLoc, HelloHookStub);
	helloHook.installHook();

	helloHook2.initialize(helloHook2Loc, HelloHook2Stub);
	helloHook2.installHook();
}

// connect command is here now, not supposed to be here, please move to client patches
typedef void (__cdecl * Steam_JoinLobby_t)(CSteamID, char);
Steam_JoinLobby_t Steam_JoinLobby = (Steam_JoinLobby_t)0x49CF70;

static netadr_t currentLobbyTarget;

const char* IWClient_HandleLobbyData(const char* key)
{
	netadr_t address = currentLobbyTarget;

	if (!strcmp(key, "addr"))
	{
		return va("%d", address.ip[0] | (address.ip[1] << 8) | (address.ip[2] << 16) | (address.ip[3] << 24));
	}
	else if (!strcmp(key, "port"))
	{
		return va("%d", htons(currentLobbyTarget.port));
	}
	
	return "212";
}

void ConnectToAddress(netadr_t address)
{
	/*steamClient->currentLobby = CSteamID( 1337132, 0x40000, k_EUniversePublic, k_EAccountTypeChat );
	steamClient->resetThis = 0;
	steamClient->addr = address.ip[0] | (address.ip[1] << 8) | (address.ip[2] << 16) | (address.ip[3] << 24);
	steamClient->port = address.port;
	steamClient->addrLoc = steamClient->addr;
	steamClient->portLoc = steamClient->port;

	*(BYTE*)0x6726059 = 1;
	*(BYTE*)0x672605B = 1;
	*(DWORD*)0x64FD9B8 = 2;*/

	// ^ that doesn't work, and I'm lazy right now
	currentLobbyTarget = address;

	CSteamID steamIDLobby = CSteamID(1337132, 0x40000, k_EUniversePublic, k_EAccountTypeChat );
	Steam_JoinLobby(steamIDLobby, 0);
}

void Auth_OpenDLCStore();

void CL_OpenStore_f()
{
	//ShellExecute(NULL, "open", "http://pastehtml.com/view/cs8q6n2pe.html", 0, 0, SW_SHOWNORMAL);
	Auth_OpenDLCStore();
}

void PatchMW2_Client()
{
	// disable some checks in connection status display (testing only?)
	*(WORD*)0x49FA32 = 0x9090;

	// cause 'does current Steam lobby match' calls in Steam_JoinLobby to be ignored (issue #8)
	*(BYTE*)0x49D007 = 0xEB;

	// disable bind protection
	*(BYTE*)0x4DACA2 = 0xEB;

	// increase maximum LSP packet size
	*(DWORD*)0x682614 = 3500;
	*(DWORD*)0x68262A = 3500;

	// party host timeout, oilrig takes a little long to load
	*(DWORD*)0x4974C2 = 60000;

	// require Windows 5
	*(BYTE*)0x467ADF = 5;
	*(BYTE*)0x6DF5D6 = '5';

	//memset((void*)0x4CAA5D, 0x90, 5); // function checking party heartbeat timeouts, causes random issues

	// remove activeAction execution (exploit in mods)
	*(BYTE*)0x5A1D43 = 0xEB;

	//make Com_Error and similar go back to main_text instead of menu_xboxlive.
	strcpy((char*)0x6FC790, "main_text");

#ifdef NO_UPNP
	memset((void*)0x60BE24, 0x90, 5); //NO UPNP
#elif DEBUG
	memset((void*)0x60BE24, 0x90, 5); //NO UPNP
#endif

	static cmd_function_t openStoreCmd;
	Cmd_AddCommand("openStore", CL_OpenStore_f, &openStoreCmd, 0);
}

StompHook vaHook;
DWORD vaHookLoc = 0x4785B0;

void PatchMW2_Miley();

void PatchMW2_VA()
{
	vaHook.initialize(vaHookLoc, va);
	vaHook.installHook();

	PatchMW2_Miley();
}

void __stdcall AIL_init_sample_hook(int a1, int a2, int a3)
{
	static void (__stdcall * AIL_init_sample)(int, int);

	if (AIL_init_sample == 0)
	{
		HMODULE mss32 = GetModuleHandleA("mss32.dll");

		AIL_init_sample = (void (__stdcall *)(int, int))GetProcAddress(mss32, "_AIL_init_sample@8");
	}

	AIL_init_sample(a1, a2);
}

void __stdcall AIL_set_sample_channel_levels_hook(int a1, int a2, int a3)
{
	static void (__stdcall * func)(int, int, int, int);

	if (func == 0)
	{
		HMODULE mss32 = GetModuleHandleA("mss32.dll");

		func = (void (__stdcall *)(int, int, int, int))GetProcAddress(mss32, "_AIL_set_sample_speaker_scale_factors@16");
	}

	char outBuf[48];

	func(a1, (int)outBuf, a2, a3);
}

void __stdcall AIL_sample_channel_levels_hook(int a1, int a2)
{
	*(int*)a2 = 5;
}

void __stdcall AIL_sample_stage_property_hook(int a1, int a2, int a3, int a4, int a5, int a6)
{
	static void (__stdcall * func)(int, int, int, int, int, int, int);

	if (func == 0)
	{
		HMODULE mss32 = GetModuleHandleA("mss32.dll");

		func = (void (__stdcall *)(int, int, int, int, int, int, int))GetProcAddress(mss32, "_AIL_set_sample_speaker_scale_factors@28");
	}

	func(a1, a2, a3, -1, a4, a5, a6);
}

void PatchMW2_Miley()
{
	return;

	call(0x689E9E, AIL_init_sample_hook, PATCH_JUMP);
	call(0x689F0A, AIL_set_sample_channel_levels_hook, PATCH_JUMP);
	call(0x689F10, AIL_sample_channel_levels_hook, PATCH_JUMP);
	call(0x689EB6, AIL_sample_stage_property_hook, PATCH_JUMP);

	// AIL_set_DirectSound_HWND
	*(BYTE*)0x64B600 = 0xC3;
}