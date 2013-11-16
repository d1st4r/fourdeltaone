// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Match recording support.
//
// Initial author: NTAuthority
// Started: 2011-12-19
// ==========================================================

#include "StdInc.h"
#include "Script.h"

static std::vector<std::string> _recordedLines;

void GScr_InitData(scr_entref_t entity)
{
	_recordedLines.clear();
}

void GScr_RecordData(scr_entref_t entity)
{
	int numParams = Scr_GetNumParam();
	const char* dataString = Scr_GetString(0);
	_recordedLines.push_back(dataString);
}

void GScr_SendData(scr_entref_t entity)
{
	std::string bigString = "matchdata\n";
	std::vector<std::string>::iterator iter;
	
	for (iter = _recordedLines.begin(); iter < _recordedLines.end(); iter++)
	{
		bigString += *iter;
	}

	NP_SendRandomString(bigString.c_str());

	_recordedLines.clear();
}

void PatchMW2_MatchRecord()
{
	return;
	Scr_DeclareFunction("initdata", GScr_InitData, false);

	/**(DWORD*)0x79A86C = (DWORD)"initdata";
	*(DWORD*)0x79A870 = (DWORD)GScr_InitData;
	*(DWORD*)0x79A874 = 0;*/

	Scr_DeclareFunction("recorddata", GScr_RecordData, false);
	Scr_DeclareFunction("senddata", GScr_SendData, false);
}