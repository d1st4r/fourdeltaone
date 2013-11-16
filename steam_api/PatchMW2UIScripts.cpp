// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: UI Script modifications, providing a simple
//          interface to UI features
//
// Initial author: Pigophone / NTAuthority 
// (some code copied from PatchMW2ServerList.cpp)
// Started: 2012-04-29
// ==========================================================

#include "StdInc.h"
#include "PatchMW2UIScripts.h"

std::vector<UIFeeder_t> UIFeeders;
std::vector<UIFeeder_t>::iterator it_feed;
std::vector<UIScript_t> UIScripts;
std::vector<UIScript_t>::iterator it_script;
std::vector<UIClickHandler_t> UIClickHandlers;
std::vector<UIClickHandler_t>::iterator it_clck;

StompHook uiFeederCountHook;
DWORD uiFeederCountHookLoc = 0x41A0D0;
DWORD uiFeederCountHookRet = 0x41A0D7;

StompHook uiFeederItemTextHook;
DWORD uiFeederItemTextHookLoc = 0x4CE9E0;
DWORD uiFeederItemTextHookRet = 0x4CE9E7;

float currentFeeder;
bool activeFeeder;

float fl737C28 = 4.0f;

char* uiS_name;
char* uiS_args;
DWORD uiS_continue = 0x45ED00;

bool Int_Parse(char* p, int* i)
{
	char* token;
	token = Com_ParseExt((char**)p);

	if (token && token[0] != 0)
	{
		*i = atoi(token);
		return true;
	}

	return false;
}

int UIFeederCountHookFunc()
{
	for (it_feed = UIFeeders.begin(); it_feed < UIFeeders.end(); it_feed++)
	{
		if (it_feed->feeder == currentFeeder)
		{
			return it_feed->GetItemCount();
		}
	}

	return 0;
}

void __declspec(naked) UIFeederCountHookStub()
{
	__asm 
	{
		mov eax, [esp + 8h]
		mov currentFeeder, eax
		call UIFeederCountHookFunc
		test eax, eax
		jz continueRunning
		retn

continueRunning:
		push ecx
		fld fl737C28
		jmp uiFeederCountHookRet;
	}

}

int index;
int column;

const char* UIFeederItemTextHookFunc()
{
	for (it_feed = UIFeeders.begin(); it_feed < UIFeeders.end(); it_feed++)
	{
		if (it_feed->feeder == currentFeeder)
		{
			return it_feed->GetItemText(index, column);
		}
	}

	return 0;
}

void __declspec(naked) UIFeederItemTextHookStub()
{
	__asm 
	{
		mov eax, [esp + 0Ch]
		mov currentFeeder, eax
		mov eax, [esp + 10h]
		mov index, eax
		mov eax, [esp + 14h]
		mov column, eax
		call UIFeederItemTextHookFunc
		test eax, eax
		jz continueRunning
		push ebx
		mov ebx, [esp + 4 + 28h]
		mov dword ptr [ebx], 0
		pop ebx
		retn

continueRunning:
		push ecx
		fld fl737C28
		jmp uiFeederItemTextHookRet
	}

}
StompHook uiFeederSelectionHook;
DWORD uiFeederSelectionHookLoc = 0x4C25D0;
DWORD uiFeederSelectionHookRet = 0x4C25D6;


bool UIFeederSelectionHookFunc()
{
	std::vector<UIFeeder_t>::iterator it;

	for (it = UIFeeders.begin(); it < UIFeeders.end(); it++)
	{
		if (it->feeder == currentFeeder)
		{
			it->Select(index);
			return true;
		}
	}
	return false;
}

void __declspec(naked) UIFeederSelectionHookStub()
{
	__asm 
	{
		mov eax, [esp + 08h]
		mov currentFeeder, eax
		mov eax, [esp + 0Ch]
		mov index, eax
		call UIFeederSelectionHookFunc
		test eax, eax
		jz continueRunning
		retn

continueRunning:
		fld fl737C28
		jmp uiFeederSelectionHookRet
	}

}

StompHook mouseEnterHook;
DWORD mouseEnterHookLoc = 0x639D6E;
DWORD mouseEnterHookRet = 0x639D75;
void MouseEnterHookFunc()
{
	activeFeeder = false;

	if (currentFeeder == 15.0f)
	{
		return;
	}

	for (it_feed = UIFeeders.begin(); it_feed < UIFeeders.end(); it_feed++)
	{
		if (it_feed->feeder == currentFeeder)
		{
			activeFeeder = true;
			break;
		}
	}
}
void __declspec(naked) MouseEnterHookStub()
{
	__asm
	{
		mov eax, [edi + 12Ch]
		mov currentFeeder, eax
	}

	MouseEnterHookFunc();
	

	if (!activeFeeder)
	{
		__asm
		{
			mov [edi + 130h], esi
		}
	}

	__asm
	{
		jmp mouseEnterHookRet	
	}
}

CallHook handleKeyHook;
DWORD handleKeyHookLoc = 0x63C5BC;

DWORD lastListBoxClickTime;
listBoxDef_t* listPtr;

static itemDef_t* item;

static void TestActiveFeeder()
{
	activeFeeder = false;
	for (it_feed = UIFeeders.begin(); it_feed < UIFeeders.end(); it_feed++)
	{
		if (it_feed->feeder == currentFeeder)
		{
			activeFeeder = true;
			break;
		}
	}
}

void __declspec(naked) HandleKeyHookStub()
{
	__asm
	{
		mov item, ebp
		mov eax, [ebp + 12Ch]
		mov currentFeeder, eax
	}

	TestActiveFeeder();
	if (!activeFeeder || currentFeeder == 15.0f)
	{
		__asm jmp handleKeyHook.pOriginal
	}

	if (Com_Milliseconds() < (int)lastListBoxClickTime)
	{
		__asm
		{
			jmp handleKeyHook.pOriginal
		}
	}

	lastListBoxClickTime = Com_Milliseconds() + 300;

	listPtr = (listBoxDef_t*)item->typeData;

	if (item->cursorPos != listPtr->cursorPos)
	{
		item->cursorPos = listPtr->cursorPos;
		index = item->cursorPos;

		UIFeederSelectionHookFunc();
	}

	__asm
	{
		retn
	}
}

CallHook selectOnMouseHook;
DWORD selectOnMouseHookLoc = 0x639D31;

void SelectOnMouseHookFunc()
{
	activeFeeder = false;

	if (currentFeeder == 15.0f)
	{
		return;
	}

	for (it_feed = UIFeeders.begin(); it_feed < UIFeeders.end(); it_feed++)
	{
		if (it_feed->feeder == currentFeeder)
		{
			activeFeeder = true;
			break;
		}
	}
}
void __declspec(naked) SelectOnMouseHookStub()
{
	__asm
	{
		mov eax, [esp + 08h]
		mov currentFeeder, eax
	}

	SelectOnMouseHookFunc();

	if (!activeFeeder)
	{
		__asm jmp selectOnMouseHook.pOriginal
	}

	__asm
	{
		retn
	}
}

CallHook playSoundOnMouseHook;
DWORD playSoundOnMouseHookLoc = 0x639D66;

void __declspec(naked) PlaySoundOnMouseHookStub()
{
	__asm
	{
		mov eax, [edi + 12Ch]
		mov currentFeeder, eax
	}

	SelectOnMouseHookFunc();

	if (!activeFeeder)
	{
		__asm jmp playSoundOnMouseHook.pOriginal
	}

	__asm
	{
		retn
	}
}

CallHook uiOwnerDrawHandleKeyHook;
DWORD uiOwnerDrawHandleKeyHookLoc = 0x63D233;

void UI_OwnerDrawHandleKey(int ownerDraw, int a2, int a3, int key)
{
	if (key == 200 || key == 201) //mouse buttons
	{
		for (it_clck = UIClickHandlers.begin(); it_clck < UIClickHandlers.end(); it_clck++)
		{
			if (it_clck->function && it_clck->owner == ownerDraw)
			{
				it_clck->function();
			}
		}
	}
}

void __declspec(naked) UI_OwnerDrawHandleKeyHookStub()
{
	__asm
	{
		push ebp
		push ecx
		push edx
		push eax
		call UI_OwnerDrawHandleKey
		add esp, 10h

		jmp uiOwnerDrawHandleKeyHook.pOriginal
	}
}

// returns true if handled, and false if not
bool UI_RunMenuScriptFunc()
{
	bool handled = false;
	for (it_script = UIScripts.begin(); it_script < UIScripts.end(); it_script++)
	{
		if (it_script->function && !_stricmp(it_script->script, uiS_name))
		{
			it_script->function(uiS_args);
			handled = true;
		}
	}
	return handled;
}

void __declspec(naked) UI_RunMenuScriptStub()
{
	__asm
	{
		mov eax, esp
		add eax, 8h
		mov uiS_name, eax
		mov eax, [esp + 0C10h]
		mov uiS_args, eax
		call UI_RunMenuScriptFunc
		test eax, eax
		jz continue_uis

		// if returned
		pop edi
		pop esi
		add esp, 0C00h
		retn

continue_uis:
		jmp uiS_continue
	}
}
void AddUIClickHandler(int owner, UIClickHandlerFunc_t function)
{
	UIClickHandler_t h;
	h.owner = owner;
	h.function = function;
	UIClickHandlers.push_back(h);
}
void AddUIScript(char* script, UIScriptFunc_t function)
{
	UIScript_t h;
	h.script = script;
	h.function = function;
	UIScripts.push_back(h);
}
void PatchMW2_UIScripts()
{
	uiFeederCountHook.initialize(uiFeederCountHookLoc, UIFeederCountHookStub, 7);
	uiFeederCountHook.installHook();

	uiFeederItemTextHook.initialize(uiFeederItemTextHookLoc, UIFeederItemTextHookStub, 7);
	uiFeederItemTextHook.installHook();

	uiFeederSelectionHook.initialize(uiFeederSelectionHookLoc, UIFeederSelectionHookStub, 6);
	uiFeederSelectionHook.installHook();

	mouseEnterHook.initialize(mouseEnterHookLoc, MouseEnterHookStub, 7);
	mouseEnterHook.installHook();

	handleKeyHook.initialize(handleKeyHookLoc, HandleKeyHookStub);
	handleKeyHook.installHook();

	selectOnMouseHook.initialize(selectOnMouseHookLoc, SelectOnMouseHookStub);
	selectOnMouseHook.installHook();

	playSoundOnMouseHook.initialize(playSoundOnMouseHookLoc, PlaySoundOnMouseHookStub);
	playSoundOnMouseHook.installHook();

	uiOwnerDrawHandleKeyHook.initialize(uiOwnerDrawHandleKeyHookLoc, UI_OwnerDrawHandleKeyHookStub);
	uiOwnerDrawHandleKeyHook.installHook();

	// some thing overwriting feeder 2's data
	*(BYTE*)0x4A06A9 = 0xEB;

	// ui script stuff
	*(int*)0x45EC5B = ((DWORD)UI_RunMenuScriptStub) - 0x45EC59 - 6;
}