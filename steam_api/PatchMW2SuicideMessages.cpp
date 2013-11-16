// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Adds a T5-style string next to obituaries of
//          players committing suicide.
//
// Initial author: NTAuthority
// Started: 2012-04-10
// ==========================================================

#include "StdInc.h"
#include <time.h>

dvar_t* iw4m_suicideMsg;
#define NUM_SUICIDE_MESSAGES 13

static const char* suicideMessages[] = 
{
	"Mistakes were made.",
	"made a fatal mistake.",
	"killed himself.",
	"couldn't take it anymore.",
	"bought the farm.",
	"ate xetal's socks.",
	"gave Pigophone a pillow.",
	"is KIA.",
	"committed suicide.",
	"couldn't be saved.",
	"was too depressed.",
	"SUICIDED!",
	"bid farewell, cruel world!"
};

CallHook printObituaryHook;
DWORD printObituaryHookLoc = 0x401453;

void ModifyObituaryMessage(char* attackerName, char* buffer)
{
	if (!attackerName[0] && (!iw4m_suicideMsg || iw4m_suicideMsg->current.boolean))
	{
		char suicideMessage[128] = { 0 };
		strcat(suicideMessage, " ");
		strcat(suicideMessage, suicideMessages[rand() % NUM_SUICIDE_MESSAGES]);
		strcat(suicideMessage, "\n");

		char* end = &buffer[strlen(buffer) - 1];
		strcpy(end, suicideMessage);
	}
}

void __declspec(naked) PrintObituaryHookStub()
{
	__asm
	{
		push ecx
		push eax				// message buffer

		mov ecx, [esp + 438h]	// function argument 2, 'attacker name'

		push ecx
		call ModifyObituaryMessage
		add esp, 4h

		pop eax
		pop ecx
		jmp printObituaryHook.pOriginal
	}
}

void PatchMW2_SuicideMessages()
{
	srand((unsigned int)time(NULL));

	printObituaryHook.initialize(printObituaryHookLoc, PrintObituaryHookStub);
	printObituaryHook.installHook();

	iw4m_suicideMsg = (dvar_t*)Dvar_RegisterBool("iw4m_suicideMsg", 1, DVAR_FLAG_SAVED, "Enables custom suicide messages");
}