// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Demo recording/playing fixes 
//
// Initial author: NTAuthority
// Started: 2011-11-22
// ==========================================================

#include "StdInc.h"

CallHook gamestateWriteHook;
DWORD gamestateWriteHookLoc = 0x5A8370;

void GamestateWriteHookFunc(msg_t* msg, char byte) {
	MSG_WriteLong(msg, 0);
	MSG_WriteByte(msg, byte);
}

void __declspec(naked) GamestateWriteHookStub() {
	__asm jmp GamestateWriteHookFunc
}

StompHook baselineStoreHook;
DWORD baselineStoreHookLoc = 0x5ABE36;
DWORD baselineStoreHookRet = 0x5ABEF5;

char baselineSnap[131072]; // might be a bit, um, large?
PBYTE bssMsg = 0;

int bssMsgOff = 0;
int bssMsgLen = 0;

void __declspec(naked) BaselineStoreHookFunc() {
	__asm {
		mov bssMsg, edi
	}

	bssMsgLen = *(int*)(bssMsg + 20);
	bssMsgOff = *(int*)(bssMsg + 28) - 7;
	memcpy(baselineSnap, (void*)*(DWORD*)(bssMsg + 8), *(DWORD*)(bssMsg + 20));

	__asm
	{
		jmp baselineStoreHookRet
	}
}

typedef size_t (__cdecl * CompressPacket_t)(char const0, char* a2, int a3, int a4);
CompressPacket_t CompressPacket = (CompressPacket_t)0x4319D0;

int* demoFile = (int*)0xA5EA1C;
int* serverMessageSequence = (int*)0xA3E9B4;

char byte0 = 0;

DWORD baselineToFileLoc = 0x5A8630;
DWORD baselineToFileRet = 0x5A863A;
StompHook baselineToFile;

void WriteBaseline() {
	msg_t buf;
	static char bufData[131072];
	static char cmpData[65535];
	int byte8 = 8;

	MSG_Init(&buf, bufData, 131072);
	MSG_WriteData(&buf, &baselineSnap[bssMsgOff], bssMsgLen - bssMsgOff);
	MSG_WriteByte(&buf, 6);

	buf.maxsize = (int)&cmpData;
	//*(int*)buf[4] = *(int*)buf[2];

	int compressedSize = CompressPacket(0, buf.data, buf.maxsize, buf.cursize);
	int fileCompressedSize = compressedSize + 4;

	FS_Write(&byte0, 1, *demoFile);
	FS_Write(serverMessageSequence, 4, *demoFile);
	//FS_Write(&buf[5], 4, *demoFile);
	FS_Write(&fileCompressedSize, 4, *demoFile);
	FS_Write(&byte8, 4, *demoFile);
	//FS_Write((void*)buf[2], buf[5], *demoFile);
	
	int pt1 = compressedSize;
	for ( int i = 0; i < compressedSize; i += 1024 )
	{
		int blk = pt1 - i;
		if ( blk > 1024 )
			blk = 1024;

		FS_Write((void *)(i + buf.maxsize), blk, *demoFile);
		pt1 = compressedSize;
	}
}

void __declspec(naked) BaselineToFileFunc() {
	WriteBaseline();

	*(int*)0xA5E9C4 = 0;
	
	__asm jmp baselineToFileRet
}


DWORD recordGamestateLoc = 0x5A85D2;
CallHook recordGamestate;

int tmpSeq;

#pragma optimize("", off)
void __declspec(naked) RecordGamestateFunc() {
	tmpSeq = *serverMessageSequence;
	tmpSeq--;

	FS_Write(&tmpSeq, 4, *demoFile);

	__asm retn
}
#pragma optimize("", on)

// using any buttons quits the game when viewing a demo, it seems.
StompHook uiSetActiveMenuHook;
DWORD uiSetActiveMenuHookLoc = 0x4CB3EF;
DWORD uiSetActiveMenuHookRet = 0x4CB3F6;
DWORD uiSetActiveMenuHookRetn = 0x4CB49C;
extern int* demoPlaying;

void __declspec(naked) UISetActiveMenuHookStub()
{
	if (*demoPlaying == 1)
	{
		__asm jmp uiSetActiveMenuHookRetn
	}
	__asm
	{
		mov     ecx, [esp+10h]
		push    10h
		push    ecx
		jmp uiSetActiveMenuHookRet
	}
}

// --- demo playback timing ---
CallHook clAdjustTimeDeltaHook;
DWORD clAdjustTimeDeltaHookLoc = 0x50320E;

void __declspec(naked) CL_AdjustTimeDeltaHookStub()
{
	__asm
	{
		mov eax, 0A5EA0Ch // demoPlaying
		mov eax, [eax]
		test al, al
		jz returnStuff

		// delta doesn't drift for demos
		retn

returnStuff:
		jmp clAdjustTimeDeltaHook.pOriginal
	}
}
// --- error: Server Connection Timed Out
// quick fix patching it. A better fix would be to patch the point of which it waits for net messages.
StompHook serverTimedOutHook;
DWORD serverTimedOutHookLoc = 0x5A8E03;
DWORD serverTimedOutHookRetn = 0x5A8E70;
DWORD serverTimedOutHookContinue = 0x5A8E08;
void __declspec(naked) ServerTimedOutHookStub()
{
	__asm
	{
		mov eax, 0A5EA0Ch // demoPlaying
		mov eax, [eax]
		test al, al
		jz returnStuff
		jmp serverTimedOutHookRetn

returnStuff:
		mov eax, 0B2BB90h
		jmp serverTimedOutHookContinue
	}
}
void PatchMW2_DemoRecording()
{
	gamestateWriteHook.initialize(gamestateWriteHookLoc, GamestateWriteHookStub);
	gamestateWriteHook.installHook();

	recordGamestate.initialize(recordGamestateLoc, RecordGamestateFunc);
	recordGamestate.installHook();

	baselineStoreHook.initialize(baselineStoreHookLoc, BaselineStoreHookFunc);
	baselineStoreHook.installHook();

	baselineToFile.initialize(baselineToFileLoc, BaselineToFileFunc);
	baselineToFile.installHook();
	
	uiSetActiveMenuHook.initialize(uiSetActiveMenuHookLoc, UISetActiveMenuHookStub);
	uiSetActiveMenuHook.installHook();

	clAdjustTimeDeltaHook.initialize(clAdjustTimeDeltaHookLoc, CL_AdjustTimeDeltaHookStub);
	clAdjustTimeDeltaHook.installHook();

	serverTimedOutHook.initialize(serverTimedOutHookLoc, ServerTimedOutHookStub);
	serverTimedOutHook.installHook();

	if (!GAME_FLAG(GAME_FLAG_DEDICATED))
	{
		// set the configstrings stuff to load the default (empty) string table; this should allow demo recording on all gametypes/maps
		*(DWORD*)0x47440B = (DWORD)"mp/defaultStringTable.csv";
	}

	// demo 'recording [name], #k' scale parameters
	*(BYTE*)0x5AC854 = 4;
	*(BYTE*)0x5AC85A = 4;
}