// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Downloading support, client-side component.
//
// Initial author: NTAuthority
// Started: 2011-06-20
// ==========================================================

#include "StdInc.h"
#include <queue>
#include <tinyxml.h>

#define CURL_STATICLIB
#include <curl/curl.h>
#include <curl/easy.h>
#include <curl/multi.h>

// function definitions
typedef void (__cdecl * CL_DownloadsComplete_t)(int controller);
CL_DownloadsComplete_t CL_DownloadsComplete = (CL_DownloadsComplete_t)0x42CE90;

typedef void (__cdecl * FS_BuildOSPath_t)(const char* path, const char* qpath, const char* a3, char* outbuf);
FS_BuildOSPath_t FS_BuildOSPath = (FS_BuildOSPath_t)0x4702C0;

// variable definitions
dvar_t** com_sv_running = (dvar_t**)0x1AD7934;
dvar_t** fs_homepath = (dvar_t**)0x63D4FD8;
int* fs_numServerReferencedIwds = (int*)0x63D0BAC;
int* fs_serverReferencedIwds = (int*)0x63D2ED8;
char** fs_serverReferencedIwdNames = (char**)0x63D0CD8;

// code implementations
bool FS_SV_FileExists( const char *file )
{
	FILE *f;
	char testpath[256];

	FS_BuildOSPath( (*fs_homepath)->current.string, file, "", testpath);
	testpath[strlen(testpath)-1] = '\0';

	f = fopen( testpath, "rb" );
	if (f) {
		fclose( f );
		return true;
	}
	return false;
}

int FS_FilenameCompare( const char *s1, const char *s2 ) {
	int		c1, c2;

	do {
		c1 = *s1++;
		c2 = *s2++;

		if (c1 >= 'a' && c1 <= 'z') {
			c1 -= ('a' - 'A');
		}
		if (c2 >= 'a' && c2 <= 'z') {
			c2 -= ('a' - 'A');
		}

		if ( c1 == '\\' || c1 == ':' ) {
			c1 = '/';
		}
		if ( c2 == '\\' || c2 == ':' ) {
			c2 = '/';
		}

		if (c1 != c2) {
			return -1;		// strings not equal
		}
	} while (c1);

	return 0;		// strings are equal
}

#define NUM_IW_IWDS 25

bool FS_iwIwd( char *pak, char *base ) {
	int i;

	for (i = 0; i < NUM_IW_IWDS; i++) {
		if ( !FS_FilenameCompare(pak, va("%s/iw_%02d", base, i)) ) {
			break;
		}
	}
	if (i < NUM_IW_IWDS) {
		// TODO: fix this up
		//if ( FS_FilenameCompare(pak, va("%s/localized_", base)) ) {
			return true;
		//}
	}
	return false;
}

int FS_CompareWithServerFiles(char* needediwds, int len, bool dlstring)
{
	searchpath_t *sp;
	bool haveiwd, badchecksum;
	int i;

	if ( !*fs_numServerReferencedIwds ) {
		return false; // Server didn't send any pack information along
	}

	*needediwds = 0;

	for ( i = 0 ; i < *fs_numServerReferencedIwds ; i++ ) {
		// Ok, see if we have this iwd file
		badchecksum = false;
		haveiwd = false;

		// never autodownload any of the iw iwds
		if ( FS_iwIwd(fs_serverReferencedIwdNames[i], "main") || FS_iwIwd(fs_serverReferencedIwdNames[i], "m2demo") ) {
			continue;
		}

		if (strstr(fs_serverReferencedIwdNames[i], "mods") == NULL)
		{
			continue;
		}

		if (strstr(fs_serverReferencedIwdNames[i], "_svr_") != NULL)
		{
			continue;
		}

		for ( sp = fs_searchpaths->next ; sp ; sp = sp->next ) {
			if ( sp->pack && sp->pack->checksum == fs_serverReferencedIwds[i] ) {
				haveiwd = true; // This is it!
				break;
			}
		}

		if ( !haveiwd && fs_serverReferencedIwdNames[i] && *fs_serverReferencedIwdNames[i] ) { 
			// Don't got it

			if (dlstring)
			{
				// Do we have one with the same name?
				if ( FS_SV_FileExists( va( "%s.iwd", fs_serverReferencedIwdNames[i] ) ) )
				{
					//char st[256];
					// We already have one called this, we need to download it to another name
					// Make something up with the checksum in it
					//_snprintf( st, sizeof( st ), "%s.iwd", fs_serverReferencedIwdNames[i] );
					//strncat( needediwds, st , len);

					//return 2;
				} else
				{
					// Remote name
					strncat( needediwds, "@", len);
					strncat( needediwds, fs_serverReferencedIwdNames[i] , len);
					strncat( needediwds, ".iwd" , len);

					// Local name
					strncat( needediwds, "@", len);

					strncat( needediwds, fs_serverReferencedIwdNames[i] , len);
					strncat( needediwds, ".iwd" , len);
				}
			}
			else
			{
				strncat( needediwds, fs_serverReferencedIwdNames[i] , len);
				strncat( needediwds, ".iwd" , len);
				// Do we have one with the same name?
				if ( FS_SV_FileExists( va( "%s.iwd", fs_serverReferencedIwdNames[i] ) ) )
				{
					strncat( needediwds, " (local file exists with wrong checksum)", len);
				}
				strncat( needediwds, "\n", len);
			}
		}
	}

	if ( *needediwds ) {
		return 1;
	}

	return 0; // We have them all
}

typedef struct download_s
{
	char url[512];
	char file[512];
	int size;
	CURL* curlHandle;
	FILE* fp;
} download_t;

struct dlState
{
	char downloadList[8192];
	int numDownloads;
	int completedDownloads;
	int totalSize;
	int completedSize;

	int lastTime;
	int lastBytes;
	int bytesPerSecond;

	bool downloadInitialized;
	CURLM* curl;

	bool isDownloading;
	download_t* currentDownload;
	std::queue<download_t> downloadQueue;
} dls;

const char* DL_ProgressString()
{
	static char buffer[1024];
	float progress = ((float)dls.completedSize / (float)dls.totalSize) * 100.0f;
	sprintf_s(buffer, sizeof(buffer), "Downloading %d of %d (%d%%, %d kB/s)", dls.completedDownloads + 1, dls.numDownloads, (int)progress, dls.bytesPerSecond / 1024);
	return buffer;
}

void CL_StartDownloads(int controller)
{
	CL_AddReliableCommand(controller, va("download %s", dls.downloadList));
}

void CL_InitDownloads(int controller)
{
	if ((*com_sv_running)->current.boolean)
	{
		return CL_DownloadsComplete(controller);
	}

	if (false) // cl_allowDownload == false
	{

	}
	else
	{
		int result = FS_CompareWithServerFiles(dls.downloadList, sizeof(dls.downloadList), true);

		if (result > 0)
		{
			if (result == 2)
			{
				Com_Error(1, "%s is different from the server", dls.downloadList);
			}
			else
			{
				Com_Printf(0, "Need files: %s\n", dls.downloadList);

				if (dls.downloadList[0] != '\0')
				{
					CL_StartDownloads(controller);
					return;
				}
			}
		}
	}

	return CL_DownloadsComplete(controller);
}

void FS_SearchPaths_f()
{
	searchpath_t* sp;

	for ( sp = fs_searchpaths->next ; sp ; sp = sp->next ) {
		if (sp->pack)
		{
			Com_Printf(0, "%s (0x%x)\n", sp->pack->pakFilename, sp->pack->checksum);
		}
		else if (sp->dir)
		{
			Com_Printf(0, "%s/%s\n", sp->dir->path, sp->dir->gamedir);
		}
	}
}

void CL_InitDownloadQueue()
{
	dls.completedSize = 0;
	dls.totalSize = 0;
	dls.completedDownloads = 0;
	dls.numDownloads = 0;
	dls.currentDownload = NULL;
	dls.isDownloading = false;
	dls.lastTime = 0;
	dls.lastBytes = 0;
	dls.bytesPerSecond = 0;
	
	std::queue<download_t> empty;
	std::swap(dls.downloadQueue, empty);
}

void CL_QueueDownload(const char* url, const char* file, int size)
{
	download_t download;
	sprintf_s(download.url, sizeof(download.url), url, CL_GetServerIPAddress());
	strcpy_s(download.file, sizeof(download.file), file);
	download.size = size;
	download.curlHandle = NULL;

	dls.downloadQueue.push(download);

	dls.numDownloads++;
	dls.totalSize += size;
}

void DL_Initialize()
{
	dls.curl = curl_multi_init();
	dls.downloadInitialized = true;

	Com_Printf(0, "Client download subsystem initialized\n");
}

void DL_Shutdown()
{
	curl_multi_cleanup(dls.curl);
	dls.downloadInitialized = false;
}

static download_t thisDownload;

void DL_DequeueDownload()
{
	download_t download = dls.downloadQueue.front();

	memcpy(&thisDownload, &download, sizeof(download_t));
	dls.currentDownload = &thisDownload;

	dls.downloadQueue.pop();
}

size_t DL_WriteToFile(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	size_t written;
	written = fwrite(ptr, size, nmemb, stream);

	// do size calculations
	dls.completedSize += (size * nmemb);
	if ((dls.lastTime + 1000) < Com_Milliseconds())
	{
		dls.bytesPerSecond = (dls.completedSize - dls.lastBytes);
		dls.lastTime = Com_Milliseconds();
		dls.lastBytes = dls.completedSize;
	}

	return written;
}

void DL_ProcessDownload()
{
	if (!dls.currentDownload)
	{
		return;
	}

	// build path stuff, TODO: should be saved somewhere intermediate
	char opath[256];
	char tmpPath[256];
	char tmpDir[256];
	//FS_BuildOSPath((*fs_homepath)->current.string, "", dls.currentDownload->file, opath);
	opath[0] = '\0';
	strcat_s(opath, sizeof(opath), (*fs_homepath)->current.string);
	strcat_s(opath, sizeof(opath), "/");
	strcat_s(opath, sizeof(opath), dls.currentDownload->file);
	strcpy_s(tmpPath, sizeof(tmpPath), opath);
	strcat_s(tmpPath, sizeof(tmpPath), ".tmp");

	// create a new handle if we don't have one yet
	if (!dls.currentDownload->curlHandle)
	{
		// create the parent directory too
		strncpy(tmpDir, tmpPath, sizeof(tmpDir));
		strrchr(tmpDir, '/')[0] = '\0';
		CreateDirectoryAnyDepth(tmpDir);

		FILE* fp = fopen(tmpPath, "wb");
		if (!fp)
		{
			dls.isDownloading = false;
			Com_Error(1, "Unable to open %s for writing.", opath);
		}

		dls.currentDownload->fp = fp;
		dls.currentDownload->curlHandle = curl_easy_init();
		curl_easy_setopt(dls.currentDownload->curlHandle, CURLOPT_URL, dls.currentDownload->url);
		curl_easy_setopt(dls.currentDownload->curlHandle, CURLOPT_WRITEDATA, fp);
		curl_easy_setopt(dls.currentDownload->curlHandle, CURLOPT_WRITEFUNCTION, DL_WriteToFile);

		curl_multi_add_handle(dls.curl, dls.currentDownload->curlHandle);
	}

	// perform the downloading loop bit
	int stillRunning;
	curl_multi_perform(dls.curl, &stillRunning);

	if (stillRunning == 0)
	{
		if (dls.currentDownload->fp)
		{
			fclose(dls.currentDownload->fp);
		}

		// check for success
		CURLMsg* info = NULL;

		do 
		{
			info = curl_multi_info_read(dls.curl, &stillRunning);

			if (info != NULL)
			{
				if (info->msg == CURLMSG_DONE)
				{
					CURLcode code = info->data.result;

					curl_multi_remove_handle(dls.curl, dls.currentDownload->curlHandle);
					curl_easy_cleanup(dls.currentDownload->curlHandle);

					if (code == CURLE_OK)
					{
						_unlink(opath);
						rename(tmpPath, opath);
						dls.completedDownloads++;
					}
					else
					{
						_unlink(tmpPath);
						Com_Error(1, "Downloading of %s failed with CURLcode 0x%08x", dls.currentDownload->url, code);
					}

					dls.currentDownload = NULL;
				}
			}
		} while (info != NULL);
	}
}

dvar_t* cl_modVidRestart;

void DL_Process()
{

	if (!dls.isDownloading)
	{
		if (dls.downloadInitialized)
		{
			DL_Shutdown();
		}

		return;
	}

	if (!dls.downloadInitialized)
	{
		DL_Initialize();
	}

	if (dls.currentDownload)
	{
		DL_ProcessDownload();
	}
	else
	{
		if (dls.downloadQueue.size() > 0)
		{
			DL_DequeueDownload();
		}
		else
		{
			if (cl_modVidRestart->current.boolean)
			{
				__asm
				{
					mov eax, 4CA1F0h // CL_Vid_Restart_f
					call eax
				}
			}

			dls.isDownloading = false;
			CL_DownloadsComplete(0);

			Cmd_ExecuteSingleCommand(0, 0, "reconnect");
		}
	}
}

void CL_ParseDownload(int controller, msg_t* msg)
{
	// read the message data
	int size = MSG_ReadLong(msg);
	char* buffer = (char*)malloc(size);
	memset(buffer, 0, size);
	MSG_ReadData(msg, buffer, size);

	// create an XML document
	TiXmlDocument document;
	document.Parse(buffer);

	// free the buffer
	free(buffer);

	// reset the download queue
	CL_InitDownloadQueue();

	dls.isDownloading = true;

	// iterate through the list of downloads and queue them
	TiXmlHandle hDocument(&document);
	TiXmlElement* root;

	root = hDocument.FirstChildElement().ToElement();
	if (!root)
	{
		Com_Printf(0, "CL_ParseDownload: no root element\n");
		return;
	}

	// actually iterate
	for (TiXmlElement* download = root->FirstChildElement("file"); download != NULL; download = download->NextSiblingElement())
	{
		// read the attributes
		int size;
		const char* url = download->Attribute("url");
		const char* file = download->Attribute("filename");
		download->Attribute("size", &size);

		// and enqueue the download
		CL_QueueDownload(url, file, size);

		// also output it
		Com_Printf(0, "download url %s file %s size %d\n", url, file, size);
	}
}

void __declspec(naked) CL_ParseServerMessageHookStub()
{
	__asm
	{
		cmp esi, 7
		jne returnToOrigin

		lea ecx, [esp + 18h]
		push ecx
		push ebx
		call CL_ParseDownload
		add esp, 8

		pop edi
		pop ebx
		pop esi
		pop ebp
		add esp, 30h
		retn

returnToOrigin:
		mov eax, 4AA05Ch
		jmp eax
	}
}

void GetFS_Game()
{
   char* msg = (char *)0x1CB7EB9;//it actually starts at 0x1CB7EB8

   Dvar_SetCommand("fs_game", Info_ValueForKey(msg, "fs_game"));
}

#pragma optimize("", off)
void __declspec(naked) clParseGamestateHookStub()
{
   __asm
   {
      mov ebx, [esp]
      mov [esp + 4], ebx
      //code we've overwritten
      add esp, 4
      mov ebx, eax

      test edi, edi //some kind of ID
      jne back
      pushad
      call GetFS_Game
      popad

back:
      ret
   }
}
#pragma optimize("", on)

// hook definitions
CallHook clInitDownloadsHook; // from CL_ParseGamestate, currently calls CL_DownloadsComplete
DWORD clInitDownloadsHookLoc = 0x5AC6E9;

CallHook clFrameHook;
DWORD clFrameHookLoc = 0x4B0F81;

CallHook connStatusDisplayHook;
DWORD connStatusDisplayHookLoc = 0x49FA39;

DWORD clParseGamestateHookLoc = 0x5AC4B0;


#include "ExtDLL.h"
extern IExtDLL* g_extDLL;

void UI_DoServerRefresh();
void CL_ConnectFrame();
void NUI_Frame()
{
	g_extDLL->NUIFrame();
}

void __declspec(naked) CL_FrameHookStub()
{
	__asm
	{
		call clFrameHook.pOriginal
		call UI_DoServerRefresh
		call NUI_Frame
		call CL_ConnectFrame
		jmp DL_Process
	}
}

void __declspec(naked) DisplayConnectionStatusHookStub()
{
	if (dls.isDownloading)
	{
		__asm
		{
			jmp DL_ProgressString
		}
	}

	__asm
	{
		jmp connStatusDisplayHook.pOriginal
	}
}

// entry point
cmd_function_t fsspCmd;

void PatchMW2_DownloadClient()
{
	// CL_InitDownloads hook
	clInitDownloadsHook.initialize(clInitDownloadsHookLoc, CL_InitDownloads);
	clInitDownloadsHook.installHook();

	clFrameHook.initialize(clFrameHookLoc, CL_FrameHookStub);
	clFrameHook.installHook();

	connStatusDisplayHook.initialize(connStatusDisplayHookLoc, DisplayConnectionStatusHookStub);
	connStatusDisplayHook.installHook();

	if (!GAME_FLAG(GAME_FLAG_DEDICATED))
	{
		call(clParseGamestateHookLoc, clParseGamestateHookStub, PATCH_CALL);
	}

	// switch table in CL_ParseServerMessage
	*(DWORD*)0x4A9FD8 = (DWORD)CL_ParseServerMessageHookStub - (0x4A9FD6 + 6);

	Cmd_AddCommand("fs_searchPaths", FS_SearchPaths_f, &fsspCmd, 0);

	cl_modVidRestart = Dvar_RegisterBool("cl_modVidRestart", true, DVAR_FLAG_SAVED, "Perform a vid_restart when loading a mod.");
}