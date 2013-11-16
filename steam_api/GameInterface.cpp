#include "StdInc.h"
#include "Script.h"
#include <assert.h>

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/threads.h>
#include <mono/metadata/exception.h>

static VariableValue backupStack[2048];
static VariableValue backup;

#ifdef COMPILING_IW4M
static DWORD* scr_retArgs = (DWORD*)0x2040D08;
static VariableValue** scr_stack = (VariableValue**)0x2040D00;

static DWORD* scr_numParam = (DWORD*)0x2040D0C;

VariableValue** stackPtr = (VariableValue**)0x2040D00;

typedef void (__cdecl * Scr_AddEntityNum_t)(int index, int type);
Scr_AddEntityNum_t Scr_AddEntityNum = (Scr_AddEntityNum_t)0x4AD230;

typedef void (__cdecl * Scr_AddVector_t)(float* vector);
Scr_AddVector_t Scr_AddVector = (Scr_AddVector_t)0x4D0FA0;

#define Cmd_Argc_sv Cmd_ArgcSV
#define Cmd_Argv_sv Cmd_ArgvSV

#define Cmd_AddCommand(a, b, c) Cmd_AddCommand(a, b, c, 0)

#include <unordered_map>
#endif

#ifdef COMPILING_IW5M
VariableValue** stackPtr = (VariableValue**)0x1F3E410;
#endif

//void Scriptability_HandleReturns();
void Scriptability_HandleReturns()
{
	g_scriptability->notifyNumArgs = *scr_retArgs;
	g_scriptability->notifyStack = *scr_stack;
}

extern "C"
{
	__declspec(dllexport) void GI_PushInt(int value)
	{
		Scr_AddInt(value);
	}

	__declspec(dllexport) void GI_PushFloat(float value)
	{
		Scr_AddFloat(value);
	}

	__declspec(dllexport) void GI_PushEntRef(int value)
	{
		Scr_AddEntityNum(value & 0xFFFF, value >> 16);
	}

	__declspec(dllexport) void GI_PushVector(float x, float y, float z)
	{
		float value[3];
		value[0] = x;
		value[1] = y;
		value[2] = z;

		Scr_AddVector(value);
	}

	typedef void (__cdecl * scriptCall_t)(int entref);

#ifdef COMPILING_IW5M
	__declspec(dllexport) void GI_Call(int func, int entref, int numParams)
#endif
#ifdef COMPILING_IW4M
	std::unordered_map<std::string, scriptCall_t> _funcCache;
	typedef scriptCall_t (__cdecl * Scr_GetFunction_t)(const char** functionName, int* developerOnly);
	Scr_GetFunction_t Scr_GetFunction = (Scr_GetFunction_t)0x44E700;
	Scr_GetFunction_t Scr_GetInstanceFunction = (Scr_GetFunction_t)0x4EC870;

	scriptCall_t findFunc(const char* func, int entref)
	{
		scriptCall_t thisCall;
		auto iter = _funcCache.find(func);

		if (iter != _funcCache.end())
		{
			thisCall = iter->second;
		}
		else
		{
			int devOnly;

			if (entref == -1)
			{
				thisCall = Scr_GetFunction(&func, &devOnly);
			}
			else
			{
				thisCall = Scr_GetInstanceFunction(&func, &devOnly);
			}

			if (!thisCall)
			{
				return false;
			}

			_funcCache[func] = thisCall;
		}

		return thisCall;
	}

	__declspec(dllexport) bool GI_Call(const char* func, int entref, int numParams)
#endif
	{
		typedef int (__cdecl * __setjmp3_t)(void* env, int a2, int a3);
		typedef void (__cdecl* RemoveRefToValue_t)(int a1, int a2);

#ifdef COMPILING_IW5M
		__setjmp3_t __setjmp3 = (__setjmp3_t)0x62DD98;
		RemoveRefToValue_t RemoveRefToValue = (RemoveRefToValue_t)0x4D8E40;

		DWORD oldNumParam = *scr_numParam;
		*scr_numParam = numParams;
		
		scriptCall_t* instanceFunctions = (scriptCall_t*)0x1BD8BF8;
		scriptCall_t* globalFunctions = (scriptCall_t*)0x1BF84E4;
		scriptCall_t thisCall;
		
		if (func > 0x1C7)
		{
			// TODO: throw exception if not passing an entref	
			thisCall = instanceFunctions[func];
		}
		else
		{
			thisCall = globalFunctions[func];
		}

		*(DWORD*)0x1F3BB7C += 1;

		if (__setjmp3((void*)(0x1F3DB98 + (16 * *(DWORD*)0x1F3BB7C)), 0, 0))
		{
			// TODO: raise an exception
			DebugBreak();

			MessageBoxA(NULL, "Script error!", "Oshibka", MB_OK);

			*(DWORD*)0x1F3E410 -= (8 * numParams);
			*(DWORD*)0x1F3E414 = 0;
			*(DWORD*)0x1F3BB7C -= 1;

			*scr_numParam = oldNumParam;
			return;
		}

		*(DWORD*)0x1F3E414 = 0;

		thisCall(entref);

		__asm
		{
			mov eax, 4DC6C0h
			call eax
		}

		//*(DWORD*)0x1F3E410 -= (8 * numParams);
		*(DWORD*)0x1F3BB7C -= 1;

		*scr_numParam = oldNumParam;

		// handle return values
		Scriptability_HandleReturns();

		*(DWORD*)0x1F3E414 = 0;
#endif

#ifdef COMPILING_IW4M
		__setjmp3_t __setjmp3 = (__setjmp3_t)0x6B6FE8;
		RemoveRefToValue_t RemoveRefToValue = (RemoveRefToValue_t)0x48E170; //!

		DWORD oldNumParam = *scr_numParam;
		*scr_numParam = numParams;
		
		scriptCall_t thisCall;
		
		thisCall = findFunc(func, entref);

		if (!thisCall)
		{
			return false;
		}

		*(DWORD*)0x204048C += 1;

		/*if (__setjmp3((void*)(0x1F3DB98 + (16 * *(DWORD*)0x1F3BB7C)), 0, 0))
		{
			// TODO: raise an exception
			DebugBreak();

			MessageBoxA(NULL, "Script error!", "Oshibka", MB_OK);

			*(DWORD*)0x1F3E410 -= (8 * numParams);
			*(DWORD*)0x1F3E414 = 0;
			*(DWORD*)0x1F3BB7C -= 1;

			*scr_numParam = oldNumParam;
			return false;
		}*/

		MonoException* exc = 0;

		*(DWORD*)0x2040D08 = 0;

		__try
		{
			thisCall(entref);
		}
		__except (((GetExceptionInformation())->ExceptionRecord->ExceptionCode == STATUS_LONGJUMP) ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
		{
			const char* errMsg = (const char*)0x2045098;

			exc = mono_exception_from_name_msg((MonoImage*)g_scriptability->scriptManagerImage, "InfinityScript", "ScriptException", errMsg);
		}

		if (exc)
		{
			mono_raise_exception(exc);
		}

		__asm
		{
			mov eax, 4386E0h
			call eax
		}

		//*(DWORD*)0x1F3E410 -= (8 * numParams);
		*(DWORD*)0x204048C -= 1;

		*scr_numParam = oldNumParam;

		// handle return values
		Scriptability_HandleReturns();

		*(DWORD*)0x2040D08 = 0;

		return true;
#endif
	}

	__declspec(dllexport) void GI_SetField(int entref, int fieldID)
	{
		DWORD oldNumParam = *scr_numParam;
		*scr_numParam = 1;

#ifdef COMPILING_IW5M
		*(DWORD*)0x1F3BB7C += 1;
		*(DWORD*)0x1F3E414 = 0;

		DWORD _setField = 0x4A7350;
#endif

#ifdef COMPILING_IW4M
		*(DWORD*)0x204048C += 1;
		*(DWORD*)0x2040D08 = 0;

		DWORD _setField = 0x4F20F0; // Scr_SetObjectField
#endif

		int entType = (entref >> 16);
		int entNum = (entref & 0xFFFF);

		__asm
		{
			push fieldID
			push entNum
			push entType
			call _setField
			add esp, 0Ch

#ifdef COMPILING_IW5M
			mov eax, 4DC6C0h // clean up params
#endif

#ifdef COMPILING_IW4M
			mov eax, 4386E0h
#endif
			call eax
		}

#ifdef COMPILING_IW5M
		*(DWORD*)0x1F3BB7C -= 1;
		*scr_numParam = oldNumParam;
		*(DWORD*)0x1F3E414 = 0;
#endif

#ifdef COMPILING_IW4M
		*(DWORD*)0x204048C -= 1;
		*scr_numParam = oldNumParam;
		*(DWORD*)0x2040D08 = 0;
#endif
	}

	__declspec(dllexport) void GI_GetField(int entref, int fieldID)
	{
#ifdef COMPILING_IW5M
		*(DWORD*)0x1F3BB7C += 1;
		*(DWORD*)0x1F3E414 = 0;

		DWORD _getField = 0x4A7440;
#endif

#ifdef COMPILING_IW4M
		*(DWORD*)0x204048C += 1;
		*(DWORD*)0x2040D08 = 0;

		DWORD _getField = 0x4FF3D0; // Scr_GetObjectField
#endif

		int entType = (entref >> 16);
		int entNum = (entref & 0xFFFF);

		__asm
		{
			push fieldID
			push entNum
			push entType
			call _getField
			add esp, 0Ch
		}

#ifdef COMPILING_IW5M
		*(DWORD*)0x1F3BB7C -= 1;
		Scriptability_HandleReturns();
		*(DWORD*)0x1F3E414 = 0;
#endif

#ifdef COMPILING_IW4M
		*(DWORD*)0x204048C -= 1;
		Scriptability_HandleReturns();
		*(DWORD*)0x2040D08 = 0;
#endif
	}

	__declspec(dllexport) void GI_TempFunc()
	{
		short** arrayTable = (short**)0x6EAC78;
		int arrayEl = 6;
		int i = 0;

		for (short** entry = arrayTable; *entry != NULL; entry += arrayEl)
		{
			OutputDebugString(va("AddFieldMapping(\"%s\", %i);\n", SL_ConvertToString(**entry), i));
			i++;
		}

		arrayTable = (short**)0x6E5CB0;
		arrayEl = 6;
		i = 24576;

		for (short** entry = arrayTable; *entry != NULL; entry += arrayEl)
		{
			OutputDebugString(va("AddFieldMapping(\"%s\", %i);\n", SL_ConvertToString(**entry), i));
			i++;
		}

		arrayTable = (short**)0x705C70;
		arrayEl = 6;
		i = 32768;

		for (short** entry = arrayTable; *entry != NULL; entry += arrayEl)
		{
			OutputDebugString(va("AddFieldMapping(\"%s\", %i);\n", SL_ConvertToString(**entry), i));
			i++;
		}

		arrayTable = (short**)0x6E7050;
		arrayEl = 8;
		i = 0;

		for (short** entry = arrayTable; *entry != NULL; entry += arrayEl)
		{
			OutputDebugString(va("AddFieldMapping(\"%s\", %i);\n", SL_ConvertToString(**entry), i));
			i++;
		}

		arrayTable = (short**)0x790BE0;
		arrayEl = 4;
		i = 0;

		for (short** entry = arrayTable; *entry != NULL; entry += arrayEl)
		{
			OutputDebugString(va("AddFieldMapping(\"%s\", %i);\n", SL_ConvertToString(**entry), i));
			i++;
		}
	}

	__declspec(dllexport) int GI_GetObjectType(int object)
	{
		int retval = 0;

#ifdef COMPILING_IW5M
		DWORD GetObjectType = 0x4D9350;
#endif

#ifdef COMPILING_IW4M
		DWORD GetObjectType = 0x4C5570;
#endif

		__asm
		{
			push object
			call GetObjectType
			add esp, 4h

			mov retval, eax
		}

		return retval;
	}

	__declspec(dllexport) void GI_SetDropItemEnabled(bool enabled)
	{
		/*if (enabled)
		{
			*(WORD*)0x47D53B = 0x0B75;
		}
		else
		{
			*(WORD*)0x47D53B = 0x9090;
		}*/
	}
};

extern "C" __declspec(dllexport) int GI_ToEntRef(int obj)
{
	int entRef = obj;

	__asm
	{
		push entRef

#ifdef COMPILING_IW5M
		mov eax, 4D9650h
#endif

#ifdef COMPILING_IW4M
		mov eax, 4B03F0h
#endif
		call eax
		add esp, 4h
		mov entRef, eax
	}

	return entRef;
}

extern "C" __declspec(dllexport) int GI_GetEntRef(int index)
{
	int entRef = (g_scriptability->notifyStack[-index]).integer;

	return GI_ToEntRef(entRef);
}

extern "C" __declspec(dllexport) int GI_GetType(int index)
{
	return (g_scriptability->notifyStack[-index]).type;
}

extern "C" __declspec(dllexport) int GI_NotifyNumArgs()
{
	return g_scriptability->notifyNumArgs;
}

extern "C" __declspec(dllexport) int GI_Cmd_Argc()
{
	return Cmd_Argc();
}

extern "C" __declspec(dllexport) int GI_Cmd_Argc_sv()
{
	return Cmd_Argc_sv();
}

extern "C" __declspec(dllexport) int GI_GetPing(int entity)
{
#ifdef COMPILING_IW5M
	return *(int*)(0x4A0CB08 + (0x1E1A2 * entity));
#endif

#ifdef COMPILING_IW4M
	return svs_clients[entity].ping;
#endif
}

extern "C" __declspec(dllexport) int64_t GI_GetClientAddress(int client)
{
#ifdef COMPILING_IW5M
	unsigned int base = 0x49EB690 + (client * 493192);
	//netadr_t* adr = (netadr_t*)(base + 147068);
	netadr_t adr = *(netadr_t*)(base + 40);
#endif

#ifdef COMPILING_IW4M
	netadr_t adr = svs_clients[client].adr;
#endif

	int ip = *(int*)adr.ip;
	int port = adr.port;
	return ((int64_t)ip << 32) + port;
}

extern "C" __declspec(dllexport) void GI_Cmd_EndTokenizedString()
{
	Cmd_EndTokenizedString();
}

extern "C" __declspec(dllexport) int GI_GetTempEntRef()
{
	if (g_scriptability->tempEntRef == (DWORD)g_entities) return 0;
	return (g_scriptability->tempEntRef - (DWORD)g_entities) / 628;
}


extern "C" __declspec(dllexport) void GI_GetVector(int index, float* vector)
{
	memcpy(vector, (g_scriptability->notifyStack[-index]).vector, sizeof(float) * 3);
}

extern "C" __declspec(dllexport) int GI_GetInt(int index)
{
	return (g_scriptability->notifyStack[-index]).integer;
}

extern "C" __declspec(dllexport) float GI_GetFloat(int index)
{
	return (g_scriptability->notifyStack[-index]).number;
}

extern "C" __declspec(dllexport) void GI_CleanReturnStack()
{
	DWORD oldNumParam = *scr_numParam;
	*scr_numParam = g_scriptability->notifyNumArgs;

	__asm
	{
#ifdef COMPILING_IW5M
		mov eax, 4DC6C0h
#endif

#ifdef COMPILING_IW4M
		mov eax, 4386E0H
#endif

		call eax
	}

	*scr_numParam = oldNumParam;
}