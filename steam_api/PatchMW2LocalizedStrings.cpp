// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Code to replace localized strings in the game.
//
// Initial author: NTAuthority
// Edited by zxz0O0 to add the ability of loading custom str files
// Started: 2013-01-17
// ==========================================================
 
#include "StdInc.h"
#include <google/dense_hash_map>
 
struct localizedEntry_s
{
  const char* value;
	const char* name;
};
 
StompHook loadLocalizeHook;
DWORD loadLocalizeHookLoc = 0x629B90;
 
static google::dense_hash_map<std::string, std::string> _localizedStrings;

static bool isW2Strings = false;
 
int LoadStrFiles(char* language)
{
	char buf[256];
	sprintf_s(buf, sizeof(buf), "%s/localizedstrings/", language);
	int count;
	char** list = FS_ListFiles(buf, "str", 0, &count);
	for(int i = 0;i < count;i++)
	{
		sprintf_s(buf, sizeof(buf), "%s/localizedstrings/%s", language, list[i]);
		Com_Printf(0, "Parsing localized string file: %s\n", buf);
		char* Error = SE_Load(buf, 0);
		if(Error)
			Com_Printf(0, "Error parsing localized string file: %s\n", Error);
	}
	FS_FreeFileList(list);
 
	return count;
}
 
void SELoadLanguageHookStub()
{
	_localizedStrings.clear();
 
	//get language from loc_modLanguage
	dvar_t* modLang = Dvar_FindVar("loc_modLanguage");
	char** enumList = *(char***)&(modLang->max); //not sure if this is really dvar_maxmin_t max.
	char* language = enumList[modLang->current.integer];
 
	//'official' iw4m localized strings
	isW2Strings = true;
	SE_Load("localizedstrings/w2.str", 0);
	isW2Strings = false;
 
	if(LoadStrFiles(language)==0)
		LoadStrFiles("english");//load english str files if there aren't any of selected language
 
	__asm
	{
		leave
		mov eax, 0x41D885 //back to SE_LoadLanguage
		jmp eax
	}
}
 
struct SEPackage
{
	char pad[64];
	union
	{
		char* namePtr;
		char name[16];
	};

	int pad2;
	int nameLength;
};

//can't use dense_hash_map in a naked
void AddCustomString(SEPackage* self, char* key, char* value)
{
	if (!isW2Strings)
	{
		const char* package = (self->nameLength < 16) ? self->name : self->namePtr;

		key = (char*)va("%s_%s", package, key);
	}

	_localizedStrings[key] = value;
}
 
void __declspec(naked) SetString(char* key, char* value, bool isEnglish)
{
	__asm
	{
		push [esp+8]
		push [esp+8]
		push ecx
		call AddCustomString
		add esp,0Ch
		mov eax,[esp] //get return address
		add esp,0x10 //remove stack space for parameters + return address
		jmp eax //return
	}
}
 
const char* SEH_GetLocalizedString(const char* key)
{
	google::dense_hash_map<std::string, std::string>::const_iterator iter = _localizedStrings.find(key);
 
	if (iter != _localizedStrings.end())
	{
		return (*iter).second.c_str();
	}
 
	localizedEntry_s* entry = (localizedEntry_s*)DB_FindXAssetHeader(ASSET_TYPE_LOCALIZE, key);
 
	if (entry)
	{
		return entry->value;
	}
	
	return NULL;
}
 
//loc_language codes
//"english", "french", "german", "italian", "spanish", "british", "russian", "polish", "korean", "taiwanese", "japanese", "chinese", "thai", "leet", "czech"
char* ModLanguageEnum[] = { "english", "french", "german", "italian", "spanish", "english", "russian", "polish",  0 };
 
void PatchMW2_LocalizedStrings()
{
	_localizedStrings.set_empty_key("");
 
	loadLocalizeHook.initialize(loadLocalizeHookLoc, SEH_GetLocalizedString);
	loadLocalizeHook.installHook();
 
	call(0x4CE5EE, SetString, PATCH_CALL); //instead of calling CStringEdPackage::SetString in CStringEdPackage::ParseLine we call our SetString
	call(0x41D860, SELoadLanguageHookStub, PATCH_JUMP); //instead of the game loading str files according to loc_language we load them according to loc_modlanguage
 
	Dvar_RegisterEnum("loc_modLanguage", ModLanguageEnum, 0, DVAR_FLAG_SAVED, "Preferred language for custom localized strings.");
}
