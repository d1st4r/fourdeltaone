// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: extdll
// Purpose: Server-side demo recording: server logic.
//
// Initial author: NTAuthority
// Started: 2013-02-24
// ==========================================================

#include "StdInc.h"
#include "Script.h"
#include "Recording.h"
#include <zlib.h>

static struct sv_demo_s
{
	bool demoRecording;
	int demoClientNum;

	char* demoBuffer;
	size_t demoBufferOffset;
	size_t demoBufferSize;
} sv_demo;

void SV_Demo_StopRecording();

void SV_Demo_WriteDemoMessage(DemoMessageType type, void* buffer, size_t length)
{
	DemoMessageHeader header;
	header.type = type;
	header.length = length;
	
	static char compressedPacket[131072];
	int err = compress2((Bytef*)compressedPacket, (uLongf*)&header.packedLength, (Bytef*)buffer, length, Z_BEST_COMPRESSION);

	if (err < 0)
	{
		SV_Demo_StopRecording();
		return;
	}

	size_t newLength = sizeof(header) + header.packedLength;

	if ((sv_demo.demoBufferOffset + newLength) >= sv_demo.demoBufferSize)
	{
		sv_demo.demoBuffer = (char*)realloc(sv_demo.demoBuffer, sv_demo.demoBufferSize * 2);
		sv_demo.demoBufferSize *= 2;
	}

	memcpy(&sv_demo.demoBuffer[sv_demo.demoBufferOffset], &header, sizeof(header));
	sv_demo.demoBufferOffset += sizeof(header);

	memcpy(&sv_demo.demoBuffer[sv_demo.demoBufferOffset], compressedPacket, header.packedLength);
	sv_demo.demoBufferOffset += header.packedLength;
}

void SV_Demo_StartRecording()
{
	sv_demo.demoBuffer = (char*)malloc(65535);
	sv_demo.demoBufferOffset = 0;
	sv_demo.demoBufferSize = 65535;
}

void SV_Demo_StopRecording()
{
	// TODO: write a 'demo end' message

	FS_WriteFile("demo.demo", "players", sv_demo.demoBuffer, sv_demo.demoBufferOffset);

	free(sv_demo.demoBuffer);
}

// script handlers
bool SV_AddDemoClient();
void SV_RemoveDemoClient();

void Scr_IsDemoRecording(scr_entref_t entref)
{
	Scr_AddInt(sv_demo.demoRecording);
}

void Scr_StartDemoRecording(scr_entref_t entref)
{
	if (sv_demo.demoRecording)
	{
		return;
	}

	sv_demo.demoRecording = true;

	SV_Demo_StartRecording();

	if (!SV_AddDemoClient())
	{
		sv_demo.demoRecording = false;
	}
}

void Scr_StopDemoRecording(scr_entref_t entref)
{
	if (!sv_demo.demoRecording)
	{
		return;
	}

	SV_RemoveDemoClient();
	sv_demo.demoRecording = false;

	SV_Demo_StopRecording();
}

void Scr_AddDemoBookmark(scr_entref_t entref)
{
	// special price cabbage
}

// democlient stuff
typedef int (__cdecl * func_int_t)();
static func_int_t _getProtocol = (func_int_t)0x4FB500;
static func_int_t _getStatMajor = (func_int_t)0x4C1850;
static func_int_t _getStatMinor = (func_int_t)0x44CBE0;
static func_int_t _getDataChecksum = (func_int_t)0x433CB0;

// SV_Cmd functions
typedef void (__cdecl * SV_Cmd_TokenizeString_t)(const char* string);
typedef void (__cdecl * SV_Cmd_EndTokenizedString_t)();

SV_Cmd_TokenizeString_t SV_Cmd_TokenizeString = (SV_Cmd_TokenizeString_t)0x4B5780;
SV_Cmd_EndTokenizedString_t SV_Cmd_EndTokenizedString = (SV_Cmd_EndTokenizedString_t)0x464750;

// connection handling functions
typedef void (__cdecl * SV_DirectConnect_t)(netadr_t adr);
SV_DirectConnect_t SV_DirectConnect = (SV_DirectConnect_t)0x460480;

typedef void (__cdecl * SV_ClientEnterWorld_t)(void* client, void* usercmd);
SV_ClientEnterWorld_t SV_ClientEnterWorld = (SV_ClientEnterWorld_t)0x468E70;

bool SV_AddDemoClient()
{
	// initialize some local relevant variables
	int maxClients = *svs_numclients;
	//char* svs_clients = (char*)0x49EB690;
	size_t clientSize = 681872;

	// check if there are any free slots
	int i = 0;

	for (char* client = (char*)svs_clients; i < maxClients; client += clientSize, i++)
	{
		if (!*(int*)client)
		{
			break;
		}
	}

	if (i == maxClients)
	{
		return false;
	}

	// prepare a connection string
	char connectString[1024];
	static int botport = 0;

	_snprintf(connectString, sizeof(connectString), "connect demo%d \"\\cg_predictItems\\1\\cl_anonymous\\0\\color\\4\\head\\default\\model\\multi\\snaps\\20\\rate\\5000\\name\\[4D1]democlient\\protocol\\%d\\checksum\\%d\\statver\\%d %u\\qport\\%d\"",
		botport,
		_getProtocol(),
		_getDataChecksum(),
		_getStatMajor(),
		_getStatMinor(),
		botport + 1);

	// handle client connection
	netadr_t botadr;
	memset(&botadr, 0, sizeof(botadr));
	botadr.port = botport;

	SV_Cmd_TokenizeString(connectString);
	SV_DirectConnect(botadr);
	SV_Cmd_EndTokenizedString();

	botport++;

	// get the bot's client_t* and clientnum; also return if the bot didn't connect
	char* botclient = 0;
	i = 0;

	for (botclient = (char*)svs_clients; i < maxClients; botclient += clientSize, i++)
	{
		if (*(int*)botclient)
		{
			netadr_t clientadr = ((client_t*)botclient)->adr;

			if (clientadr.type == botadr.type && clientadr.port == botadr.port)
			{
				// found him. or her. or it. or whatever.
				break;
			}
		}
	}

	if (i == maxClients)
	{
		return false;
	}

	// set the 'bot' flag (and set it to 2 as well to indicate we're a democlient)
	*(int*)(botclient + 269040) = 2;

	// SV_SendClientGamestate, a usercall
	__asm
	{
		mov eax, 625570h
			push esi
			mov esi, botclient
			call eax
			pop esi
	}

	// SV_ClientEnterWorld
	char usercmd[44];
	memset(usercmd, 0, sizeof(usercmd));

	SV_ClientEnterWorld(botclient, usercmd);

	// and return the clientnum (which is also an entref, even, yay)
	sv_demo.demoClientNum = i;

	return true;
}

void SV_KickClient(void* client, const char* reason);

/*void SV_KickClient(void* client, const char* reason)
{
	DWORD func = 0x6249A0;
	__asm
	{
		push edi
		push esi
		mov edi, 0
		mov esi, client
		push reason
		push 0
		push 0
		call func
		add esp, 0Ch
		pop esi
		pop edi
	}
}*/

void SV_RemoveDemoClient()
{
	SV_KickClient(&svs_clients[sv_demo.demoClientNum], "EXE_DISCONNECTED");
}

void SV_SendMessageToClientHookFunc(msg_t* msg, client_t* client)
{
	if (client->isBot == 2) // demo client
	{
		SV_Demo_WriteDemoMessage(DMT_SNAPSHOT, msg->data, msg->cursize);
	}
}

void __declspec(naked) SV_SendMessageToClientHookStub()
{
	__asm
	{
		mov eax, [esp + 4h]
		mov ecx, [esp + 8h]

		push ecx
		push eax
		call SV_SendMessageToClientHookFunc
		// add esp, 8
		
		// prepare for return (sub esp, 8 was done above by pushing)
		// sub esp, 8

		push ebx
		push ebp

		// return to original function
		mov eax, 48FE95h
		jmp eax
	}
}

void PatchMW2_RecordingSV()
{
	Scr_DeclareFunction("startdemorecording", Scr_StartDemoRecording, false);
	Scr_DeclareFunction("stopdemorecording", Scr_StopDemoRecording, false);
	Scr_DeclareFunction("isdemorecording", Scr_IsDemoRecording, false);
	Scr_DeclareFunction("adddemobookmark", Scr_AddDemoBookmark, false);

	// SV_SendMessageToClient hook - captures messages for the demo client
	call(0x48FE90, SV_SendMessageToClientHookStub, PATCH_JUMP);
}