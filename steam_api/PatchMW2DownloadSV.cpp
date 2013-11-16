// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Downloading support, server-side component.
//
// Initial author: NTAuthority
// Started: 2011-06-21
// ==========================================================

#include "StdInc.h"
#include <tinyxml.h>

// type definitions
typedef struct {
	char	*name;
	void	(*func)( void *cl );
	int		flag;
} ucmd_t;

typedef struct  
{
	char downloadNames[8192];
} cldl_t;

// variable definitions
static ucmd_t ucmds[32];
static cldl_t clientDL[MAX_CLIENTS];

// code implementations
void SV_SendDownloads_f(void* client)
{
	Com_Printf(0, "Obtained download string from client %i: %s\n", CLIENTNUM(client), Cmd_ArgvSV(1));

	strcpy_s(clientDL[CLIENTNUM(client)].downloadNames, 8191, Cmd_ArgvSV(1));
}

void SV_WriteDownloadToClient(void* client, msg_t* msg)
{
	char downloadNames[8192];
	int clientNum = CLIENTNUM(client);

	if (clientDL[clientNum].downloadNames[0] == '\0')
	{
		return;
	}

	Com_Printf(0, "Sending download data to client %i\n", clientNum);

	// copy locally to allow for strtok
	strcpy_s(downloadNames, sizeof(downloadNames) - 1, clientDL[clientNum].downloadNames);

	// create the XML document
	TiXmlDocument doc;
	TiXmlElement* rootElement = new TiXmlElement("downloadList");

	int i = 0;

	for (char* token = strtok(downloadNames, "@"); token; token = strtok(NULL, "@"))
	{
		if ((i % 2) == 0)
		{
			char downloadOrigin[512];
			char filename[256];

			const char* baseURL = "http://%%s/dl/%s";

			strcpy_s(filename, sizeof(filename) - 1, token);
			sprintf_s(downloadOrigin, sizeof(downloadOrigin) - 1, baseURL, filename);

			TiXmlElement* element = new TiXmlElement("file");
			element->SetAttribute("size", FileSize(filename));
			element->SetAttribute("filename", filename);
			element->SetAttribute("url", downloadOrigin);

			rootElement->LinkEndChild(element);
		}

		i++;
	}

	doc.LinkEndChild(rootElement);

	// print the document
	TiXmlPrinter printer;
	printer.SetStreamPrinting();
	doc.Accept(&printer);

	const char* data = printer.CStr();

	// write out XML string to the msg_t
	MSG_WriteByte(msg, 7); // our own svc_download
	MSG_WriteLong(msg, strlen(data));
	MSG_WriteData(msg, (char*)data, strlen(data));

	Com_Printf(0, "%s", data);

	clientDL[clientNum].downloadNames[0] = '\0';
}

bool FS_iwIwd( char *pak, char *base );

bool FS_GetFileForDownload(const char* path, char* buffer, size_t bufferSize)
{
	// split gamename/basename
	char qpath[1024];
	char qpath2[1024];
	strcpy_s(qpath, sizeof(qpath), path);
	strcpy_s(qpath2, sizeof(qpath2), path);

	char* pakName = strrchr(qpath, '/');

	if (!pakName)
	{
		return false;
	}

	pakName[0] = '\0';
	pakName++;

	// remove the '.' from the IWD name
	char* dot = strrchr(pakName, '.');

	if (dot)
	{
		dot[0] = '\0';
	}
	
	dot = strrchr(qpath2, '.');
	
	if (dot)
	{
		dot[0] = '\0';
	}

	// never, ever return IW IWDs
	if (FS_iwIwd(qpath2, qpath))
	{
		return false;
	}

	if (strstr(path, "_svr_") != NULL)
	{
		return false;
	}

	// get the OS path for the IWD
	for (searchpath_t* sp = fs_searchpaths->next; sp; sp = sp->next)
	{
		if (sp->pack)
		{
			if (strcmp(sp->pack->pakGamename, qpath) == 0)
			{
				if (strcmp(sp->pack->pakBasename, pakName) == 0)
				{
					strcpy_s(buffer, bufferSize, sp->pack->pakFilename);
					return true;
				}
			}
		}
	}

	return false;
}

// hook definitions
CallHook endClientSnapshotHook;
DWORD endClientSnapshotHookLoc = 0x4F5315;

void __declspec(naked) EndClientSnapshotHookStub()
{
	__asm
	{
		push esi
		push edi
		call SV_WriteDownloadToClient
		add esp, 8h
		jmp endClientSnapshotHook.pOriginal
	}
}

// entry point
void PatchMW2_DownloadServer()
{
	// move ucmds
	memcpy(&ucmds, (void*)0x79B9E8, sizeof(ucmd_t) * 7);
	*(DWORD*)0x625962 = (DWORD)ucmds;
	*(DWORD*)0x62596C = (DWORD)ucmds;

	// add our call to ucmds
	ucmds[6].name = "download";
	ucmds[6].func = SV_SendDownloads_f;
	ucmds[6].flag = 0;

	// add 'svc_download' to svc name list (for debugging)
	*(DWORD*)0x798C04 = (DWORD)"svc_download";

	// hooks
	endClientSnapshotHook.initialize(endClientSnapshotHookLoc, EndClientSnapshotHookStub);
	endClientSnapshotHook.installHook();
}