// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: line/file display for script compilation errors
//
// Initial author: NTAuthority
// Started: 2011-06-11
// ==========================================================

#include "StdInc.h"
#include "Script.h"

const char* currentScriptFilename;

// Scr_LoadScriptInternal
StompHook scriptSetFileNameHook;
DWORD scriptSetFileNameHookLoc = 0x427DBC;
DWORD scriptSetFileNameHookRet = 0x427DC3;

StompHook scriptRestoreFileNameHook;
DWORD scriptRestoreFileNameHookLoc = 0x427E71;
DWORD scriptRestoreFileNameHookRet = 0x427E78;

void __declspec(naked) ScriptRestoreFileNameHookStub()
{
	__asm
	{
		mov currentScriptFilename, ebx
		pop ebx
		mov ds:1CDEAA8h, ebp
		pop ebp
		jmp scriptRestoreFileNameHookRet
	}
}

void __declspec(naked) ScriptSetFileNameHookStub()
{
	__asm
	{
		push ebp
		mov ebp, ds:1CDEAA8h
		push ebx
		mov ebx, currentScriptFilename
		mov eax, [esp - 8h]
		mov currentScriptFilename, eax
		jmp scriptSetFileNameHookRet
	}
}

// _CompileError
StompHook compileErrorHook;
DWORD compileErrorHookLoc = 0x434260;

void Scr_PrintSourcePos(const char* filename, int offset)
{
	int line = 0;
	int inLineOffset = 0;
	char* scriptFile = 0;
	char* currentLine = 0;
	bool freeScript = false;
	
	if (FS_ReadFile(filename, (char**)&scriptFile) > -1)
	{
		int globalOffset = 0;

		freeScript = true;

		for (char* c = scriptFile; *c != '\0'; c++)
		{
			if (!currentLine || *c == '\n')
			{
				line++;
				inLineOffset = 0;
				currentLine = c;
			}

			if (globalOffset == offset)
			{
				while (*c != '\r' && *c != '\n' && c != '\0')
				{
					c++;
				}

				*c = '\0';
				break;
			}

			if (*c == '\t')
			{
				*c = ' ';
			}

			globalOffset++;
			inLineOffset++;
		}
	}

	Com_Printf(23, "at file %s, line %d:", filename, line);

	if (currentLine)
	{
		Com_Printf(23, "%s\n", currentLine);

		for (int i = 0; i < (inLineOffset - 1); i++)
		{
			Com_Printf(23, " ");
		}

		Com_Printf(23, "*\n");
	}
	else
	{
		Com_Printf(23, "\n");
	}

	if (freeScript)
	{
		FS_FreeFile(scriptFile);
	}
}

void CompileError(int offset, const char* message, ...)
{
	char msgbuf[1024];
	va_list v;
	va_start(v, message);
	_vsnprintf(msgbuf, sizeof(msgbuf), message, v);
	va_end(v);

	char scriptFilename[512];

	if (strstr(currentScriptFilename, ".gsc") == 0)
	{
		_snprintf(scriptFilename, sizeof(scriptFilename), "%s.gsc", currentScriptFilename);
	}
	else
	{
		strcpy(scriptFilename, currentScriptFilename);
	}

	// TODO: possibly implement support for 0x201A40F ('just eat errors' flag)?
	((void (*)())0x441650)(); // Scr_ShutdownAllocNode

	Com_Printf(23, "\n");
	Com_Printf(23, "******* script compile error *******\n");
	Com_Printf(23, "Howdy there, fella' - it seems you made a mistake!\n");
	Com_Printf(23, "Error: %s ", msgbuf);
	Scr_PrintSourcePos(scriptFilename, offset);
	Com_Printf(23, "************************************\n");

	Com_Error(5, "script compile error\n%s\n%s\n(see console for actual details)\n", msgbuf, scriptFilename);
}

DWORD FunctionLookupStartRetn = (DWORD)0x612DB6;
DWORD FunctionLookupUnkStubRetn = (DWORD)0x612E92;
DWORD FunctionLookupUnkStub2Retn = (DWORD)0x612EA7;

int functionRef;
char* functionStr;

void FunctionLookupUnkFunc()
{
	functionStr = SL_ConvertToString((unsigned short)functionRef);

	char scriptFilename[512];

	if (strstr(currentScriptFilename, ".gsc") == 0)
	{
		_snprintf(scriptFilename, sizeof(scriptFilename), "%s.gsc", currentScriptFilename);
	}
	else
	{
		strcpy(scriptFilename, currentScriptFilename);
	}

	Com_Printf(23, "\n");
	Com_Printf(23, "******* script compile error *******\n");
	Com_Printf(23, "Howdy there, fella' - it seems you made a mistake!\n");
	Com_Printf(23, "Error: unknown function %s in %s\n", functionStr, scriptFilename);
	Com_Printf(23, "************************************\n");

	Com_Error(5, "script compile error\nunknown function %s\n%s\n", functionStr, scriptFilename);
}

void __declspec(naked) FunctionLookupUnkStub()
{
	__asm
	{
		call FunctionLookupUnkFunc
		mov edx, [ebx]
		jmp FunctionLookupUnkStubRetn
	}
}

void __declspec(naked) FunctionLookupUnkStub2()
{
	__asm
	{
		call FunctionLookupUnkFunc
		jmp FunctionLookupUnkStub2Retn
	}
}

void __declspec(naked) FunctionLookupStart()
{
	__asm
	{
		mov     eax, [esp-8]
		mov functionRef, eax
		sub     esp, 0Ch
		push    0
		push    edi
		jmp FunctionLookupStartRetn
	}
}
void PatchMW2_ScriptCompileDebug()
{
	scriptSetFileNameHook.initialize(scriptSetFileNameHookLoc, ScriptSetFileNameHookStub, 7);
	scriptSetFileNameHook.installHook();

	scriptRestoreFileNameHook.initialize(scriptRestoreFileNameHookLoc, ScriptRestoreFileNameHookStub, 7);
	scriptRestoreFileNameHook.installHook();

	compileErrorHook.initialize(compileErrorHookLoc, CompileError);
	compileErrorHook.installHook();

	// unknown function reporting
	call(0x612DB0, FunctionLookupStart, PATCH_JUMP); // to get the function name
	call(0x612E85, FunctionLookupUnkStub, PATCH_JUMP); // first occurence
	call(0x612E9C, FunctionLookupUnkStub2, PATCH_JUMP); // second occurence

	// increase various stack bits as we push/pop ebx as well now
	#define ADD_STACK_BYTE(address) *(BYTE*)address += 4;
	#define ADD_STACK_DWORD(address) *(DWORD*)address += 4;

	ADD_STACK_BYTE(0x427DDF)
	ADD_STACK_DWORD(0x427E41)
	ADD_STACK_DWORD(0x427E48)
	ADD_STACK_BYTE(0x427E50)
	ADD_STACK_BYTE(0x427E55)

	#undef ADD_STACK_BYTE
	#undef ADD_STACK_DWORD
}