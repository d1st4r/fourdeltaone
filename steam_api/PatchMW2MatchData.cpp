// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Modern Warfare 2 patches: matchdata storage
//
// Initial author: NTAuthority
// Started: 2010-11-27
// ==========================================================

#include "StdInc.h"

#if USE_MANAGED_CODE
#include <json/writer.h>
#include <json/value.h>
#include <iostream>
#include <fstream>
#include <time.h>
#include "AdminPlugin.h"

typedef void (__cdecl * Com_Printf_t)(int, const char*, ...);
extern Com_Printf_t Com_Printf;

typedef int (__cdecl * Script_NumParams_t)(void);
Script_NumParams_t Script_NumParams = (Script_NumParams_t)0x482970;

typedef int (__cdecl * Script_GetIntParam_t)(int);
Script_GetIntParam_t Script_GetIntParam = (Script_GetIntParam_t)0x4DE150;

typedef char* (__cdecl * Script_GetStringParam_t)(int);
Script_GetStringParam_t Script_GetStringParam = (Script_GetStringParam_t)0x44E300;

using namespace System;
using namespace System::Collections::Generic;

public ref class MatchEntry
{
public:
	int type;
	Dictionary<Object^, MatchEntry^>^ list;
	String^ string;
	int integer;

	void Set(String^ str)
	{
		type = 0;
		string = str; 
	}

	void Set(int inte)
	{
		type = 1;
		integer = inte;
	}

	void SetArray()
	{
		if (type == 2) return;

		type = 2;
		list = gcnew Dictionary<Object^, MatchEntry^>();
	}

	void SetObject()
	{
		if (type == 3) return;

		type = 3;
		list = gcnew Dictionary<Object^, MatchEntry^>();
	}

	MatchEntry^ SetIndex(String^ i)
	{
		if (list->ContainsKey(i))
		{
			return list[i];
		}

		MatchEntry^ entry = gcnew MatchEntry();
		list->Add(i, entry);
		return entry;
	}

	MatchEntry^ SetIndex(int i)
	{
		if (list->ContainsKey(i))
		{
			return list[i];
		}

		MatchEntry^ entry = gcnew MatchEntry();
		list->Add(i, entry);
		return entry;
	}
};

ref class MatchData
{
private:
	static MatchEntry^ root;
public:
	static MatchData()
	{
		Clear();
	}

	static void Clear()
	{
		root = gcnew MatchEntry();
		root->SetObject();
	}

	static void FromScript(int params)
	{
		MatchEntry^ object = root;

		for (int i = 0; i < (params - 1); i++)
		{
			if (i % 2)
			{
				// array index
				object->SetArray();
				object = object->SetIndex(Script_GetIntParam(i));
			}
			else
			{
				// object key
				object->SetObject();
				object = object->SetIndex(gcnew String(Script_GetStringParam(i)));
			}
		}

		char* string = Script_GetStringParam(params - 1);
		object->Set(gcnew String(string));
		//IW4::AdminPluginCode::TriggerTest(root);
		//object->Set(42);
	}

	static Json::Value ProcessEntry(MatchEntry^ entry)
	{
		switch (entry->type)
		{
		case 0:
			{
				IntPtr ptr = Runtime::InteropServices::Marshal::StringToHGlobalAnsi(entry->string);
				Json::Value value(std::string((char*)ptr.ToPointer()));
				Runtime::InteropServices::Marshal::FreeHGlobal(ptr);

				return value;
			}
		case 1:
			return Json::Value(entry->integer);
		case 2:
			{
				Json::Value value(Json::ValueType::arrayValue);

				for each (KeyValuePair<Object^, MatchEntry^>^ thing in entry->list)
				{
					int index = (int)thing->Key;

					value[index] = ProcessEntry(thing->Value);
				}

				return value;
			}
		case 3:
			{
				Json::Value value(Json::ValueType::objectValue);

				for each (KeyValuePair<Object^, MatchEntry^>^ thing in entry->list)
				{
					String^ mindex = (String^)thing->Key;

					IntPtr ptr = Runtime::InteropServices::Marshal::StringToHGlobalAnsi(mindex);
					value[(char*)ptr.ToPointer()] = ProcessEntry(thing->Value);
					Runtime::InteropServices::Marshal::FreeHGlobal(ptr);
				}

				return value;
			}
		}

		return Json::Value(0);
	}

	static void Save()
	{
		Json::StyledStreamWriter writer;
		//Json::Value jroot(Json::ValueType::objectValue);
		Json::Value jroot = ProcessEntry(root);		
		std::ofstream ostream;
		char filename[255];
		time_t rawtime;
		tm* timeinfo;

		time(&rawtime);
		timeinfo = localtime(&rawtime);
		strftime(filename, sizeof(filename), "main/matchdata-%Y-%m-%d-%H%M.json", timeinfo);

		ostream.open(filename);
		writer.write(ostream, jroot);
		ostream.flush();

		ostream.close();
	}
};

void MatchData_FromScript(int params)
{
	MatchData::FromScript(params);
}

void MatchData_Clear()
{
	MatchData::Clear();
}

void MatchData_Save()
{
	MatchData::Save();
}

#pragma unmanaged

void Script_SetMatchData()
{
	int params = Script_NumParams();

	if (params < 2)
	{
		return;
	}

	MatchData_FromScript(params);
}

void Script_SetMatchDataDef()
{
	MatchData_Clear();
}

void Script_SendMatchData()
{
	MatchData_Save();
}

void PatchMW2_MatchData()
{
	// patch script function calls
	*(DWORD*)0x79ACD8 = (DWORD)Script_SetMatchData;
	*(DWORD*)0x79ACF0 = (DWORD)Script_SendMatchData;
	*(DWORD*)0x79AD08 = (DWORD)Script_SetMatchDataDef;
}
#else
void PatchMW2_MatchData()
{
}
#endif