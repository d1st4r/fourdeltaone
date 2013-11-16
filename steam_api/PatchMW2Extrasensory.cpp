// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Extrasensory features for an extrasensory
//			audience.
//
// Initial author: NTAuthority
// Started: 2012-05-07
// ==========================================================

#include "StdInc.h"

#if 0
#define MAX_ENTITIES 2048

typedef struct  
{
	bool playerWasVisible;
} entityStateCustom_t;

typedef struct  
{
	int entityNum;
	char pad[20];
	float trBase[3];
} playerEntityState_t;

typedef struct  
{
	char pad[948];
	float origin[3];
	char pad2[364];
} somePlayerState_t;

static somePlayerState_t* states = (somePlayerState_t*)0x1A40008;

CallHook entityWriteFieldsHook;
DWORD entityWriteFieldsHookLoc = 0x6111AC;

static entityStateCustom_t entityStates[MAX_CLIENTS][MAX_CLIENTS];

static int* a1;

void __declspec(naked) EntityWriteFields(int snapshotInfo, int a3, playerEntityState_t* oldState, playerEntityState_t* newState, int a6, int a7, int a8, int a9)
{
	__asm
	{
		mov eax, a1
		jmp entityWriteFieldsHook.pOriginal
	}
}

typedef struct  
{
	float fraction;
	char pad[96];
} trace_t;

typedef void (__cdecl * G_TraceCapsule_t)(trace_t* trace, float* source, float* target, float* bounds, int flags1, int flags2);
G_TraceCapsule_t G_TraceCapsule = (G_TraceCapsule_t)0x42CE50;

void EntityWriteFieldsHookFunc(int snapshotInfo, int a3, playerEntityState_t* oldState, playerEntityState_t* newState, int a6, int a7, int a8, int a9)
{
	int clientNum = *a1;
	int entityNum = newState->entityNum;

	if (entityNum >= MAX_CLIENTS)
	{
		EntityWriteFields(snapshotInfo, a3, oldState, newState, a6, a7, a8, a9);
		return;
	}

	playerEntityState_t oldStateBackup;
	playerEntityState_t newStateBackup;

	memcpy(&oldStateBackup, oldState, sizeof(oldStateBackup));
	memcpy(&newStateBackup, newState, sizeof(newStateBackup));

	trace_t trace;
	G_TraceCapsule(&trace, states[clientNum].origin, newState->trBase, (float*)0x739B78, 2047, 0x810011);

	bool visible = (trace.fraction > 0.0f);

	if (!visible)
	{
		newState->trBase[0] += ((Com_Milliseconds() / 10) % 300);
	}

	if (entityStates[clientNum][entityNum].playerWasVisible != visible)
	{
		oldState->trBase[0] = 0.0f;
		oldState->trBase[1] = 0.0f;
		oldState->trBase[2] = 0.0f;
	}

	EntityWriteFields(snapshotInfo, a3, oldState, newState, a6, a7, a8, a9);

	memcpy(oldState, &oldStateBackup, sizeof(oldStateBackup));
	memcpy(newState, &newStateBackup, sizeof(newStateBackup));
}

void __declspec(naked) EntityWriteFieldsHookStub()
{
	__asm
	{
		mov a1, eax
		jmp EntityWriteFieldsHookFunc
	}
}

void PatchMW2_Extrasensory()
{
	entityWriteFieldsHook.initialize(entityWriteFieldsHookLoc, EntityWriteFieldsHookStub);
	entityWriteFieldsHook.installHook();
}
#endif