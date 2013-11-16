// ==========================================================
// alterIWnet project
// 
// Component: aiw_client
// Sub-component: steam_api
// Purpose: Functionality to interact with the GameScript 
//          runtime.
//
// Initial author: NTAuthority
// Started: 2011-12-19
// ==========================================================

#include "StdInc.h"
#include "Script.h"

// script calls
Scr_GetNumParam_t Scr_GetNumParam = (Scr_GetNumParam_t)0x4B0E90;
Scr_GetString_t Scr_GetString = (Scr_GetString_t)0x425900;
Scr_GetFloat_t Scr_GetFloat = (Scr_GetFloat_t)0x443140;
Scr_GetInt_t Scr_GetInt = (Scr_GetInt_t)0x4F31D0;

Scr_AddString_t Scr_AddString = (Scr_AddString_t)0x412310;
Scr_AddInt_t Scr_AddInt = (Scr_AddInt_t)0x41D7D0;
Scr_AddFloat_t Scr_AddFloat = (Scr_AddFloat_t)0x61E860;

Scr_LoadScript_t Scr_LoadScript = (Scr_LoadScript_t)0x45D940;
Scr_GetFunctionHandle_t Scr_GetFunctionHandle = (Scr_GetFunctionHandle_t)0x4234F0;

Scr_ExecThread_t Scr_ExecThread = (Scr_ExecThread_t)0x4AD0B0;
Scr_ExecEntThread_t Scr_ExecEntThread = (Scr_ExecEntThread_t)0x48F640;
Scr_FreeThread_t Scr_FreeThread = (Scr_FreeThread_t)0x4BD320;

Scr_LoadScript_t Scr_Error = (Scr_LoadScript_t)0x61E8B0;
Scr_NotifyNum_t Scr_NotifyNum = (Scr_NotifyNum_t)0x48F980;
Scr_NotifyLevel_t Scr_NotifyLevel = (Scr_NotifyLevel_t)0x4D9C30;

SL_ConvertToString_t SL_ConvertToString = (SL_ConvertToString_t)0x4EC1D0;
SL_GetString_t SL_GetString = (SL_GetString_t)0x4CDC10;

#ifndef BUILDING_EXTDLL
// custom functions
typedef struct  
{
	const char* functionName;
	scr_function_t functionCall;
	int developerOnly;
} scr_funcdef_t;

static std::map<std::string, scr_funcdef_t> scriptFunctions;

void __declspec(naked) Scr_DeclareFunctionTableEntry(scr_function_t func)
{
	__asm
	{
		mov eax, 492D50h
		jmp eax
	}
}

scr_function_t Scr_GetCustomFunction(const char** name, int* isDeveloper)
{
	if (name)
	{
		if(scriptFunctions.find(std::string(*name)) != scriptFunctions.end()){
			scr_funcdef_t func = scriptFunctions[*name];

			if (func.functionName)
			{
				*name = func.functionName;
				*isDeveloper = func.developerOnly;

				return func.functionCall;
			}
		}
	}
	else
	{
		std::map<std::string, scr_funcdef_t>::iterator iter;

		for (iter = scriptFunctions.begin(); iter != scriptFunctions.end(); iter++)
		{

			scr_funcdef_t func = (*iter).second;

			Scr_DeclareFunctionTableEntry(func.functionCall);
		}
	}

	return NULL;
}


void Scr_DeclareFunction(const char* name, scr_function_t func, bool developerOnly = false)
{
	scr_funcdef_t funcDef;
	funcDef.functionName = name;
	funcDef.functionCall = func;
	funcDef.developerOnly = (developerOnly) ? 1 : 0;

	scriptFunctions[name] = funcDef;
}

int GScr_LoadScriptAndLabel(char* script, char* function)
{
	Com_Printf(0, "Loading script %s.gsc...\n", script);
	if (!Scr_LoadScript(script))
	{
		Com_Printf(0, "Script %s encountered an error while loading. (doesn't exist?)", script);
		Com_Error(1, (char*)0x70B810, script);
	}
	else
	{
		Com_Printf(0, "Script %s.gsc loaded successfully.\n", script);
	}
	Com_Printf(0, "Finding script handle %s::%s...\n", script, function);
	int handle = Scr_GetFunctionHandle(script, function);
	if (handle)
	{
		Com_Printf(0, "Script handle %s::%s loaded successfully.\n", script, function);
		return handle;
	}
	Com_Printf(0, "Script handle %s::%s couldn't be loaded. (file with no entry point?)\n", script, function);
	return handle;
}
// TODO: move this into a PatchMW2 file?
CallHook scrGetFunctionHook;
DWORD scrGetFunctionHookLoc = 0x44E72E;

CallHook gscrLoadGameTypeScriptHook;
DWORD gscrLoadGameTypeScriptHookLoc = 0x45D44A;

CallHook scrLoadGameTypeHook;
DWORD scrLoadGameTypeHookLoc = 0x48EFFE;

void __declspec(naked) Scr_GetFunctionHookStub()
{
	__asm
	{
		push esi
		push edi
		call scrGetFunctionHook.pOriginal

		test eax, eax
		jnz returnToSender

		// try our own function
		call Scr_GetCustomFunction

returnToSender:
		pop edi
		pop esi

		retn
	}
}

int scrhandle = 0;
int threadhandle = 0;
int amount = 0;
int i = 0;
char** list = 0;
std::vector<int> handles;
void __declspec(naked) GScr_LoadGameTypeScriptHookStub()
{
	list = FS_ListFiles("scripts/", "gsc", 0, &amount);
	if (handles.size() > 0) handles.clear();
	for (i = 0; i < amount; i++)
	{
		if (strlen(list[i]) < 5 || list[i][strlen(list[i])-4] != '.')
		{
				continue;
		}
		else
		{
			list[i][strlen(list[i])-4] = 0;
		}
		static char scriptName[255];
		sprintf_s(scriptName, sizeof(scriptName), "scripts/%s", list[i]);
		scrhandle = GScr_LoadScriptAndLabel(scriptName, "init");
		if (scrhandle)
		{
			handles.push_back(scrhandle);
		}
	}
	FS_FreeFileList(list);
	__asm jmp gscrLoadGameTypeScriptHook.pOriginal
}
void __declspec(naked) Scr_LoadGameTypeHookStub()
{
	if (handles.size() > 0) {
		for (i = 0; i < (int)handles.size(); i++){
			threadhandle = Scr_ExecThread(handles[i], 0);
			Scr_FreeThread(threadhandle);
		}
	}
	__asm jmp scrLoadGameTypeHook.pOriginal
}

void GScr_PrintLnConsole(scr_entref_t entity)
{
	if (Scr_GetNumParam() == 1)
	{
		Com_Printf(0, "%s\n", Scr_GetString(0));
	}
}

void GScr_GetPlayerPing(scr_entref_t entity)
{
	if (Scr_GetNumParam() != 1)
	{
		Scr_Error("getPlayerPing accepts one parameter: clientNum");
	}
	else
	{
		int num = Scr_GetInt(0);
		if (num >= 0 && num < *svs_numclients)
		{
			DWORD clientStart = *(DWORD*)0x412AE4; // lol
			Scr_AddInt(*(short*)(clientStart + 681872 * num + 135880));
		}
		else
		{
			Scr_AddInt(-1);
		}
	}
}
typedef struct
{
	int handle;
	char* currentLine; //for reading functions
} gscr_file_t;

std::vector<gscr_file_t> gscr_files;
std::vector<gscr_file_t>::iterator gscr_files_iter;
#define SCRIPT_LINE_SIZE 2048

void GScr_OpenFile(scr_entref_t entity)
{
	if (!GAME_FLAG(GAME_FLAG_GSCFILESYSTEM))
	{
		Scr_Error("Script tried to use FS commands when gscfilesystem switch isn't enabled");
		return;
	}
	if (Scr_GetNumParam() != 2)
	{
		Scr_Error("USAGE: OpenFile( <filename>, <mode> )\nValid arguments for mode are 'r', 'w' and 'a'.");
		return;
	}
	char* filename = Scr_GetString(0);
	int handle = 0;
	switch (Scr_GetString(1)[0])
	{
		case 'a':
			handle = FS_FOpenFileAppend(filename);
			break;
		case 'w':
			handle = FS_FOpenFileWrite(filename);
			break;
		case 'r':
			FS_FOpenFileRead(filename, &handle, 1);
			break;
		default:
			Scr_Error("USAGE: OpenFile( <filename>, <mode> )\nInvalid argument for mode. Valid arguments are 'r', 'w' and 'a'.");
			return;
	}
	if (handle)
	{
		//all good
		gscr_file_t file;
		file.handle = handle;
		file.currentLine = (char*)malloc(SCRIPT_LINE_SIZE);
		gscr_files.push_back(file);
		Scr_AddInt(handle);
		return;
	}
	Scr_AddInt(-1);
	return;
}
void GScr_CloseFile(scr_entref_t entity)
{
	if (!GAME_FLAG(GAME_FLAG_GSCFILESYSTEM))
	{
		Scr_Error("Script tried to use FS commands when gscfilesystem switch isn't enabled");
		return;
	}
	if (Scr_GetNumParam() != 1)
	{
		Scr_Error("USAGE: CloseFile( <handle> )");
		return;
	}
	int handle = Scr_GetInt(0);
	for (gscr_files_iter = gscr_files.begin(); gscr_files_iter < gscr_files.end(); gscr_files_iter++)
	{
		if (gscr_files_iter->handle == handle)
		{
			FS_FCloseFile(handle);
			gscr_files.erase(gscr_files_iter);
			Scr_AddInt(1);
			return;
		}
	}
	Scr_AddInt(-1);
	return;
}
void GScr_FReadLn(scr_entref_t entity)
{
	if (!GAME_FLAG(GAME_FLAG_GSCFILESYSTEM))
	{
		Scr_Error("Script tried to use FS commands when gscfilesystem switch isn't enabled");
		return;
	}
	if (Scr_GetNumParam() != 1)
	{
		Scr_Error("USAGE: FReadLn( <handle> )");
		return;
	}
	int handle = Scr_GetInt(0);
	for (gscr_files_iter = gscr_files.begin(); gscr_files_iter < gscr_files.end(); gscr_files_iter++)
	{
		if (gscr_files_iter->handle == handle)
		{
			// unfortunatly, functions for reading bytes aren't fun, as the files could also be in zips
			// so I'll just read 1 byte until nl :(
			memset(gscr_files_iter->currentLine, 0, SCRIPT_LINE_SIZE);
			char* temp = gscr_files_iter->currentLine;
			while (true)
			{
				if (temp - gscr_files_iter->currentLine == SCRIPT_LINE_SIZE)
				{
					break; //line over alloc'd space
				}
				if (FS_Read(temp, 1, gscr_files_iter->handle) != 1) //if EOF I guess
				{
					*temp = 0;
					break;
				}
				else if (*temp == '\n')
				{
					*temp = 0;
					break;
				}
				temp++;
			}
			Scr_AddString(gscr_files_iter->currentLine);
			return;
		}
	}
	Scr_AddString("");
	return;
}

void GScr_FGetLn(scr_entref_t entity)
{
	if (!GAME_FLAG(GAME_FLAG_GSCFILESYSTEM))
	{
		Scr_Error("Script tried to use FS commands when gscfilesystem switch isn't enabled");
		return;
	}
	if (Scr_GetNumParam() != 1)
	{
		Scr_Error("USAGE: FGetLn( <handle> )");
		return;
	}
	int handle = Scr_GetInt(0);
	for (gscr_files_iter = gscr_files.begin(); gscr_files_iter < gscr_files.end(); gscr_files_iter++)
	{
		if (gscr_files_iter->handle == handle && gscr_files_iter->currentLine[0] != NULL)
		{
			Scr_AddString(gscr_files_iter->currentLine);
			return;
		}
	}
	Scr_AddString("");
	return;
}
void GScr_FPrintFields(scr_entref_t entity)
{
	if (!GAME_FLAG(GAME_FLAG_GSCFILESYSTEM))
	{
		Scr_Error("Script tried to use FS commands when gscfilesystem switch isn't enabled");
		return;
	}
	if (Scr_GetNumParam() < 2)
	{
		Scr_Error("USAGE: FPrintFields( <handle> , <field1> , <field2> , ... )");
		return;
	}
	int handle = Scr_GetInt(0);
	for (gscr_files_iter = gscr_files.begin(); gscr_files_iter < gscr_files.end(); gscr_files_iter++)
	{
		if (gscr_files_iter->handle == handle)
		{
			for (int i = 1; i < Scr_GetNumParam(); i++)
			{
				int len = strlen(Scr_GetString(i));
				int wlen = FS_Write((void*)Scr_GetString(i), len, gscr_files_iter->handle);
				if (len != wlen) // FS_Write returns amount of bytes written, something went wrong if they aren't equal.
				{
					Scr_AddInt(-1);
					return;
				}

				if (i != Scr_GetNumParam() - 1) // put a space after each arg
				{
					if (FS_Write(" ", 1, gscr_files_iter->handle) != 1)
					{
						Scr_AddInt(-1);
						return;
					}
				}
			}
			if (FS_Write((void*)"\n", 1, gscr_files_iter->handle) != 1)
			{
				Scr_AddInt(-1);
				return;
			}
			Scr_AddInt(1);
			return;
		}
	}
	Scr_AddInt(-1);
	return;
}
void GScr_FPrintLn(scr_entref_t entity)
{
	if (!GAME_FLAG(GAME_FLAG_GSCFILESYSTEM))
	{
		Scr_Error("Script tried to use FS commands when gscfilesystem switch isn't enabled");
		return;
	}
	if (Scr_GetNumParam() < 2)
	{
		Scr_Error("USAGE: FPrintLn( <handle> , <text> )");
		return;
	}
	int handle = Scr_GetInt(0);
	for (gscr_files_iter = gscr_files.begin(); gscr_files_iter < gscr_files.end(); gscr_files_iter++)
	{
		if (gscr_files_iter->handle == handle)
		{
			const char* format = va("%s\n", Scr_GetString(1));
			int len = strlen(format);
			int wlen = FS_Write((void*)format, len, gscr_files_iter->handle);
			if (len == wlen)
			{
				Scr_AddInt(len);
				return;
			}
			Scr_AddInt(-1);
			return;
		}
	}
	Scr_AddInt(-1);
	return;
}

void PatchMW2_Script()
{
	scrGetFunctionHook.initialize(scrGetFunctionHookLoc, Scr_GetFunctionHookStub);
	scrGetFunctionHook.installHook();

	gscrLoadGameTypeScriptHook.initialize(gscrLoadGameTypeScriptHookLoc, GScr_LoadGameTypeScriptHookStub);
	gscrLoadGameTypeScriptHook.installHook();

	scrLoadGameTypeHook.initialize(scrLoadGameTypeHookLoc, Scr_LoadGameTypeHookStub);
	scrLoadGameTypeHook.installHook();

	Scr_DeclareFunction("printlnconsole", GScr_PrintLnConsole);

	Scr_DeclareFunction("getplayerping", GScr_GetPlayerPing);

	//filesystem commands
	if (GAME_FLAG(GAME_FLAG_DEDICATED))
	{
		// change pointers to our functions
		*(DWORD*)0x79A864 = (DWORD)GScr_CloseFile; // closes a file; returns -1 if error, 1 if ok
		*(DWORD*)0x79A858 = (DWORD)GScr_OpenFile; // opens a file; returns -1 if error, other number if ok
		*(DWORD*)0x79A870 = (DWORD)GScr_FPrintLn; // writes a string + \n to the file, returns -1 if fail, other positive number (bytes written) if ok
		*(DWORD*)0x79A87C = (DWORD)GScr_FPrintFields; // prints args + \n to the file, returns -1 if fail, other positive number (bytes written) if ok
		*(DWORD*)0x79A888 = (DWORD)GScr_FReadLn; // reads a line; returns that line; nothing if error

		*(DWORD*)0x79A85C = 0;
		*(DWORD*)0x79A868 = 0;
		*(DWORD*)0x79A874 = 0;
		*(DWORD*)0x79A880 = 0;
		*(DWORD*)0x79A88C = 0;

		Scr_DeclareFunction("fgetln", GScr_FGetLn); // returns the current line, slightly redundant as freadln
													// gets the line too (this one doesn't read any bytes from the stream)
		
	}
}
#endif