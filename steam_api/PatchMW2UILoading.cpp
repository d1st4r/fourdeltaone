// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: UI file loading
//
// Initial author: NTAuthority
// Started: 2011-10-21
// ==========================================================

#include "StdInc.h"

typedef struct  
{
	char* filename;
	int count;
	menuDef_t** menuFiles;
} menuFile_t;

// Q3TA precompiler code

//undef if binary numbers of the form 0b... or 0B... are not allowed
#define BINARYNUMBERS
//undef if not using the token.intvalue and token.floatvalue
#define NUMBERVALUE
//use dollar sign also as punctuation
#define DOLLAR

//maximum token length
#define MAX_TOKEN					1024

//punctuation
typedef struct punctuation_s
{
	char *p;						//punctuation character(s)
	int n;							//punctuation indication
	struct punctuation_s *next;		//next punctuation
} punctuation_t;

//token
typedef struct token_s
{
	char string[MAX_TOKEN];			//available token
	int type;						//last read token type
	int subtype;					//last read token sub type
#ifdef NUMBERVALUE
	unsigned long int intvalue;	//integer value
	long double floatvalue;			//floating point value
#endif //NUMBERVALUE
	char *whitespace_p;				//start of white space before token
	char *endwhitespace_p;			//start of white space before token
	int line;						//line the token was on
	int linescrossed;				//lines crossed in white space
	struct token_s *next;			//next token in chain
} token_t;

//script file
typedef struct script_s
{
	char filename[64];				//file name of the script
	char *buffer;					//buffer containing the script
	char *script_p;					//current pointer in the script
	char *end_p;					//pointer to the end of the script
	char *lastscript_p;				//script pointer before reading token
	char *whitespace_p;				//begin of the white space
	char *endwhitespace_p;			//end of the white space
	int length;						//length of the script in bytes
	int line;						//current line in script
	int lastline;					//line before reading token
	int tokenavailable;				//set by UnreadLastToken
	int flags;						//several script flags
	punctuation_t *punctuations;	//the punctuations used in the script
	punctuation_t **punctuationtable;
	token_t token;					//available token
	struct script_s *next;			//next script in a chain
} script_t;

//macro definitions
typedef struct define_s
{
	char *name;							//define name
	int flags;							//define flags
	int builtin;						// > 0 if builtin define
	int numparms;						//number of define parameters
	token_t *parms;						//define parameters
	token_t *tokens;					//macro tokens (possibly containing parm tokens)
	struct define_s *next;				//next defined macro in a list
	struct define_s *hashnext;			//next define in the hash chain
} define_t;

//indents
//used for conditional compilation directives:
//#if, #else, #elif, #ifdef, #ifndef
typedef struct indent_s
{
	int type;								//indent type
	int skip;								//true if skipping current indent
	script_t *script;						//script the indent was in
	struct indent_s *next;					//next indent on the indent stack
} indent_t;

//source file
typedef struct source_s
{
	char filename[64];					//file name of the script
	char includepath[64];					//path to include files
	punctuation_t *punctuations;			//punctuations to use
	script_t *scriptstack;					//stack with scripts of the source
	token_t *tokens;						//tokens to read first
	define_t *defines;						//list with macro definitions
	define_t **definehash;					//hash chain with defines
	indent_t *indentstack;					//stack with indents
	int skip;								// > 0 if skipping conditional code
	token_t token;							//last read token
} source_t;

#define MAX_TOKENLENGTH		1024

typedef struct pc_token_s
{
	int type;
	int subtype;
	int intvalue;
	float floatvalue;
	char string[MAX_TOKENLENGTH];
} pc_token_t;

//token types
#define TT_STRING						1			// string
#define TT_LITERAL					2			// literal
#define TT_NUMBER						3			// number
#define TT_NAME						4			// name
#define TT_PUNCTUATION				5			// punctuation

//typedef int menuDef_t;
//typedef int itemDef_t;

#define KEYWORDHASH_SIZE	512

typedef struct keywordHash_s
{
	char *keyword;
	bool (*func)(itemDef_t *item, int handle);
	//struct keywordHash_s *next;
} keywordHash_t;

std::vector<menuDef_t*> UI_ParseMenu(const char *menuFile);
bool Menu_Parse(int handle, menuDef_t *menu);
int PC_LoadSource(const char *filename);
int PC_FreeSource(int handle);

typedef script_t * (__cdecl * LoadScriptFile_t)(const char*);
typedef int (__cdecl * PC_ReadToken_t)(source_t*, token_t*);

LoadScriptFile_t LoadScriptFile = (LoadScriptFile_t)0x480110;
PC_ReadToken_t PC_ReadToken = (PC_ReadToken_t)0x4ACCD0;

source_t **sourceFiles = (source_t **)0x7C4A98;
keywordHash_t **menuParseKeywordHash = (keywordHash_t **)0x63AE928;

// 'shared' code
int KeywordHash_Key(char *keyword) {
	int register hash, i;

	hash = 0;
	for (i = 0; keyword[i] != '\0'; i++) {
		if (keyword[i] >= 'A' && keyword[i] <= 'Z')
			hash += (keyword[i] + ('a' - 'A')) * (119 + i);
		else
			hash += keyword[i] * (119 + i);
	}
	hash = (hash ^ (hash >> 10) ^ (hash >> 20)) & (KEYWORDHASH_SIZE-1);
	return hash;
}

/*keywordHash_t *KeywordHash_Find(keywordHash_t *table[], char *keyword)
{
	keywordHash_t *key;
	int hash;

	hash = KeywordHash_Key(keyword);
	for (key = table[hash]; key; key = key->next) {
		if (!stricmp(key->keyword, keyword))
			return key;
	}
	return NULL;
}*/

typedef void (__cdecl * FreeMemory_t)(void* buffer);
FreeMemory_t FreeMemory = (FreeMemory_t)0x4D6640;

void FreeScript(script_t *script)
{
#ifdef PUNCTABLE
	if (script->punctuationtable) FreeMemory(script->punctuationtable);
#endif //PUNCTABLE
	FreeMemory(script);

}

void StripDoubleQuotes(char *string)
{
	if (*string == '\"')
	{
		strcpy(string, string+1);
	} //end if
	if (string[strlen(string)-1] == '\"')
	{
		string[strlen(string)-1] = '\0';
	} //end if
} 

source_t* LoadSourceString(const char* string);

source_t *LoadSourceFile(const char *filename)
{
	int fh;
	int length = FS_FOpenFileRead(filename, &fh, 0);

	if (length >= 0)
	{
		char* buffer = new char[length + 1];
		FS_Read(buffer, length, fh);
		FS_FCloseFile(fh);

		buffer[length] = '\0';

		source_t* retval = LoadSourceString(buffer);
		delete[] buffer;

		return retval;
	}

	return NULL;
}

typedef script_t* (__cdecl * Script_Alloc_t)(int length);
Script_Alloc_t Script_Alloc = (Script_Alloc_t)0x422E70;

typedef void (__cdecl * Script_SetupTokens_t)(script_t* script, void* tokens);
Script_SetupTokens_t Script_SetupTokens = (Script_SetupTokens_t)0x4E6950;

typedef int (__cdecl * Script_CleanString_t)(char* buffer);
Script_CleanString_t Script_CleanString = (Script_CleanString_t)0x498220;

script_t* LoadScriptString(const char* string)
{
	static_assert(sizeof(script_t) == 1200, "script_t != 1200");

	int length = strlen(string);
	script_t* script = Script_Alloc(sizeof(script_t) + 1 + length);

	strcpy_s(script->filename, sizeof(script->filename), "script_t");
	script->buffer = (char*)(script + 1);
	
	*((char*)(script + 1) + length) = '\0';

	script->script_p = script->buffer;
	script->lastscript_p = script->buffer;
	script->length = length;
	script->end_p = &script->buffer[length];
	script->line = 1;
	script->lastline = 1;
	script->tokenavailable = 0;
	
	Script_SetupTokens(script, (void*)0x797F80);
	script->punctuations = (punctuation_t*)0x797F80;

	strcpy(script->buffer, string);

	script->length = Script_CleanString(script->buffer);

	return script;
}

source_t* LoadSourceString(const char* string)
{
	source_t *source;
	script_t *script;

	script = LoadScriptString(string);
	if (!script) return NULL;

	script->next = NULL;

	source = (source_t *) malloc(sizeof(source_t));
	memset(source, 0, sizeof(source_t));

	strncpy(source->filename, "string", 64);
	source->scriptstack = script;
	source->tokens = NULL;
	source->defines = NULL;
	source->indentstack = NULL;
	source->skip = 0;
	source->definehash = (define_t**)malloc(4096);
	memset(source->definehash, 0, 4096);

	return source;
} 

void FreeSource(source_t *source)
{
	script_t *script;
	token_t *token;
	define_t *define;
	indent_t *indent;
	//->int i;

	//PC_PrintDefineHashTable(source->definehash);
	//free all the scripts
	while(source->scriptstack)
	{
		script = source->scriptstack;
		source->scriptstack = source->scriptstack->next;
		FreeScript(script);
	} //end for
	//free all the tokens
	while(source->tokens)
	{
		token = source->tokens;
		source->tokens = source->tokens->next;
		//PC_FreeToken(token);
	} //end for
#if DEFINEHASHING
	for (i = 0; i < DEFINEHASHSIZE; i++)
	{
		while(source->definehash[i])
		{
			define = source->definehash[i];
			source->definehash[i] = source->definehash[i]->hashnext;
			PC_FreeDefine(define);
		} //end while
	} //end for
#else //DEFINEHASHING
	//free all defines
	while(source->defines)
	{
		define = source->defines;
		source->defines = source->defines->next;
		//PC_FreeDefine(define);
	} //end for
#endif //DEFINEHASHING
	//free all indents
	while(source->indentstack)
	{
		indent = source->indentstack;
		source->indentstack = source->indentstack->next;
		free(indent);
	} //end for
//#if DEFINEHASHING
	//
	if (source->definehash) free(source->definehash);
//#endif //DEFINEHASHING
	//free the source itself
	free(source);
}

#define MAX_SOURCEFILES		64

source_t* lastSource;

int PC_LoadSourceString(const char* string)
{
	source_t *source;
	int i;

	for (i = 1; i < MAX_SOURCEFILES; i++)
	{
		if (!sourceFiles[i])
			break;
	} //end for
	if (i >= MAX_SOURCEFILES)
		return 0;
	//PS_SetBaseFolder("");
	source = LoadSourceString(string);
	if (!source)
		return 0;
	lastSource = source;
	sourceFiles[i] = source;
	return i;
}

int PC_LoadSource(const char *filename)
{
	source_t *source;
	int i;

	for (i = 1; i < MAX_SOURCEFILES; i++)
	{
		if (!sourceFiles[i])
			break;
	} //end for
	if (i >= MAX_SOURCEFILES)
		return 0;
	//PS_SetBaseFolder("");
	source = LoadSourceFile(filename);
	if (!source)
		return 0;
	lastSource = source;
	sourceFiles[i] = source;
	return i;
}

int PC_FreeSource(int handle)
{
	if (handle < 1 || handle >= MAX_SOURCEFILES)
		return false;
	if (!sourceFiles[handle])
		return false;

	FreeSource(sourceFiles[handle]);
	sourceFiles[handle] = NULL;
	return true;
}

int PC_ReadTokenHandle(int handle, pc_token_t *pc_token)
{
	token_t token;
	int ret;

	if (handle < 1 || handle >= MAX_SOURCEFILES)
		return 0;
	if (!sourceFiles[handle])
		return 0;

	ret = PC_ReadToken(sourceFiles[handle], &token);
	strcpy(pc_token->string, token.string);
	pc_token->type = token.type;
	pc_token->subtype = token.subtype;
	pc_token->intvalue = token.intvalue;
	pc_token->floatvalue = (float)token.floatvalue;
	if (pc_token->type == TT_STRING)
		StripDoubleQuotes(pc_token->string);
	return ret;
}

#ifndef BUILDING_EXTDLL
// UI code
/*void Item_SetScreenCoords(itemDef_t *item, float x, float y) {
  
  if (item == NULL) {
    return;
  }

  if (item->window.border != 0) {
    x += item->window.borderSize;
    y += item->window.borderSize;
  }

  item->window.rect.x = x + item->window.rectClient.x;
  item->window.rect.y = y + item->window.rectClient.y;
  item->window.rect.w = item->window.rectClient.w;
  item->window.rect.h = item->window.rectClient.h;

  // force the text rects to recompute
  item->textRect.w = 0;
  item->textRect.h = 0;
}

void Menu_UpdatePosition(menuDef_t *menu) {
  int i;
  float x, y;

  if (menu == NULL) {
    return;
  }
  
  x = menu->window.rect.x;
  y = menu->window.rect.y;
  if (menu->window.border != 0) {
    x += menu->window.borderSize;
    y += menu->window.borderSize;
  }

  for (i = 0; i < menu->itemCount; i++) {
    Item_SetScreenCoords(menu->items[i], x, y);
  }
}*/

void Menu_PostParse(menuDef_t *menu) {
	return;
	/*if (menu == NULL) {
		return;
	}
	if (menu->fullScreen) {
		menu->window.rect.x = 0;
		menu->window.rect.y = 0;
		menu->window.rect.w = 640;
		menu->window.rect.h = 480;
	}
	Menu_UpdatePosition(menu);*/
}

void Menu_Init(menuDef_t *menu) {
	memset(menu, 0, 2048);
	menu->items = (itemDef_t**)malloc(sizeof(itemDef_t*) * 512);
	//Window_Init(&menu->window);
}

int KeywordHash_Data(char* key)
{
	// patch this function on-the-fly, as it's some ugly C.
	DWORD func = 0x63FE90;
	int retval = 0;

	*(DWORD*)0x63FE9E = 3523;
	*(DWORD*)0x63FECB = 0x7F;

	__asm
	{
		mov eax, key
		call func
		mov retval, eax
	}

	*(DWORD*)0x63FE9E = 531;
	*(DWORD*)0x63FECB = 0x1FF;

	return retval;
}

//#define PC_SourceError(x, y, ...) Com_Printf(0, y, __VA_ARGS__)
#define PC_SourceError(x, y, ...) ((void (*)(int, const char*, ...))0x467A00)(x, y, __VA_ARGS__)

bool Menu_Parse(int handle, menuDef_t *menu) {
	pc_token_t token;
	keywordHash_t *key;

	if (!PC_ReadTokenHandle(handle, &token))
		return false;
	if (*token.string != '{') {
		return false;
	}
    
	while ( 1 ) {

		memset(&token, 0, sizeof(pc_token_t));
		if (!PC_ReadTokenHandle(handle, &token)) {
			PC_SourceError(handle, "end of file inside menu\n");
			return false;
		}

		if (*token.string == '}') {
			return true;
		}

		//key = KeywordHash_Find(menuParseKeywordHash, token.string);
		int idx = KeywordHash_Data(token.string);

		key = menuParseKeywordHash[idx];
		if (!key) {
			PC_SourceError(handle, "unknown menu keyword %s", token.string);
			continue;
		}
		if ( !key->func((itemDef_t*)menu, handle) ) {
			PC_SourceError(handle, "couldn't parse menu keyword %s", token.string);
			return false;
		}
	}
	return false; 	// bk001205 - LCC missing return value
}

menuDef_t* Menu_New(int handle) {
	menuDef_t *menu = (menuDef_t*)malloc(2048); // FIXME: tentative size

	Menu_Init(menu);
	if (Menu_Parse(handle, menu)) {
		Menu_PostParse(menu);
	}

	return menu;
}

std::vector<menuDef_t*> UI_ParseMenu(const char *menuFile) {
	int handle;
	pc_token_t token;
	std::vector<menuDef_t*> retval;

	Com_Printf(0, "Parsing menu file: %s\n", menuFile);

	handle = PC_LoadSource(menuFile);
	if (!handle) {
		return retval;
	}

	while ( 1 ) {
		memset(&token, 0, sizeof(pc_token_t));
		if (!PC_ReadTokenHandle( handle, &token )) {
			break;
		}

		if ( token.string[0] == '}' ) {
			break;
		}

		if (_stricmp(token.string, "loadmenu") == 0)
		{
			PC_ReadTokenHandle(handle, &token);

			std::vector<menuDef_t*> tempMenus = UI_ParseMenu(token.string);
			
			for (auto it = tempMenus.begin(); it != tempMenus.end(); it++)
			{
				retval.push_back(*it);
			}
		}

		if (_stricmp(token.string, "menudef") == 0) {
			// start a new menu
			retval.push_back(Menu_New(handle));
		}
	}
	
	PC_FreeSource(handle);

	return retval;
}

menuFile_t* UI_ParseScriptMenu(const char* filename)
{
	if (FS_ReadFile(filename, NULL) <= 0)
	{
		return (menuFile_t*)DB_FindXAssetHeader(ASSET_TYPE_MENUFILE, filename);
	}

	menuFile_t* newFile = (menuFile_t*)malloc(sizeof(menuFile_t));
	newFile->filename = (char*)filename;

	std::vector<menuDef_t*> menus = UI_ParseMenu(filename);

	newFile->count = menus.size();
	newFile->menuFiles = (menuDef_t**)malloc(sizeof(menuDef_t*) * menus.size());

	for (int i = 0; i < menus.size(); i++)
	{
		newFile->menuFiles[i] = menus[i];
	}

	return newFile;
}

menuDef_t* LoadScreens_MessWithConnectMenu(menuDef_t* menu);

menuFile_t* uimp_code;

#define NUM_CUSTOM_MENUS 1

void* MenuFileHookFunc(const char* filename)
{
	if (!GAME_FLAG(GAME_FLAG_DEDICATED))
	{
		if (FS_ReadFile(filename, NULL) >= 0)
		{
			return UI_ParseScriptMenu(filename);
		}
	}

	menuFile_t* file = (menuFile_t*)DB_FindXAssetHeader(ASSET_TYPE_MENUFILE, filename);

	if (GAME_FLAG(GAME_FLAG_DEDICATED))
	{
		return file;
	}

	if (!strcmp(file->menuFiles[0]->window.name, "default_menu"))
	{
		return UI_ParseScriptMenu(filename);
	}

	if (!strcmp(filename, "ui_mp/code.txt") && uimp_code)
	{
		return uimp_code;
	}

	int numCustomMenus = 0;

	if (!strcmp(filename, "ui_mp/menus.txt"))
	{
		numCustomMenus = NUM_CUSTOM_MENUS;
	}

	menuFile_t* newFile = (menuFile_t*)malloc(sizeof(menuFile_t));
	memcpy(newFile, file, sizeof(menuFile_t));

	newFile->menuFiles = (menuDef_t**)malloc(sizeof(menuDef_t*) * (newFile->count + numCustomMenus));
	memset(newFile->menuFiles, 0, sizeof(menuDef_t*) * (newFile->count + numCustomMenus));
	memcpy(newFile->menuFiles, file->menuFiles, sizeof(menuDef_t*) * newFile->count);

	newFile->count += numCustomMenus;

	bool replacedMenu = false;

	for (int i = 0; i < file->count; i++)
	{
		menuDef_t* menu = file->menuFiles[i];

		if (!menu)
		{
			continue;
		}

		char uifilename[512];
		sprintf(uifilename, "ui_mp/%s.menu", menu->window.name);
		
		if (FS_ReadFile(uifilename, 0) >= 0)
		{
			std::vector<menuDef_t*> menus = UI_ParseMenu(uifilename);

			if (menus.size() > 0)
			{
				newFile->menuFiles[i] = menus[0];
			}

			if (menus.size() > 1)
			{
				Com_PrintError(0, "Menu load warning: more than 1 menu in %s\n", uifilename);
			}

			if (!strcmp(menu->window.name, "connect"))
			{
				newFile->menuFiles[i] = LoadScreens_MessWithConnectMenu(newFile->menuFiles[i]);
			}

			replacedMenu = true;
		}
		else
		{
			// odd hook for loadscreens
			if (!strcmp(menu->window.name, "connect"))
			{
				newFile->menuFiles[i] = LoadScreens_MessWithConnectMenu(menu);

				replacedMenu = true;
			}
		}
	}

#define CUSTOM_MENU(name) { \
		std::vector<menuDef_t*> menu = UI_ParseMenu(name); \
		\
		if (menu.size() > 0) \
		{ \
			newFile->menuFiles[file->count] = menu[0]; \
		} \
		else \
		{ \
			file->count--; \
		} \
	}

	if (numCustomMenus)
	{
		CUSTOM_MENU("ui_mp/popup_friends.menu");
	}

	if (!replacedMenu)
	{
		free(newFile->menuFiles);
		free(newFile);
	}
	else if (!strcmp(filename, "ui_mp/code.txt"))
	{
		uimp_code = newFile;
	}

	return (replacedMenu) ? newFile : file;
}

StompHook menuFileHook;
DWORD menuFileHookLoc = 0x63FE70;	

void __declspec(naked) MenuFileHookStub()
{
	__asm jmp MenuFileHookFunc
}

CallHook uiAddMenuAssetHook;
DWORD uiAddMenuAssetHookLoc = 0x453406;

void __declspec(naked) UI_AddMenuAssetHookStub()
{
	__asm
	{
		cmp [esp + 28h], 0
		jz doOriginal
		xor eax, eax
		retn

doOriginal:
		jmp uiAddMenuAssetHook.pOriginal
	}
}
cmd_function_t Cmd_OpenMenu;
void Cmd_OpenMenu_f()
{
	if (Cmd_Argc() != 2)
	{
		Com_Printf(0, "USAGE: openmenu <menu name>\n");
		return;
	}
	char* menu = Cmd_Argv(1);
	__asm
	{
		push menu
			push 62E2858h
			mov eax, 4CCE60h
			call eax
	}
}

void __declspec(naked) ParseExpressionHookStub()
{
	__asm
	{
		mov dword ptr [esp + 8], 300

		push esi
		push edi
		push 18h
		mov eax, 4F3680h
		call eax

		push 413059h
		retn
	}
}

void PatchMW2_UILoading()
{
//#ifndef PRE_RELEASE_DEMO
	menuFileHook.initialize(menuFileHookLoc, MenuFileHookStub);
	menuFileHook.installHook();
//#endif

	//uiAddMenuAssetHook.initialize((PBYTE)uiAddMenuAssetHookLoc);
	//uiAddMenuAssetHook.installHook(UI_AddMenuAssetHookStub, false);

	// disable any cheat protections
	//*(BYTE*)0x647682 = 0xEB;

	// disable the 2 new tokens in ItemParse_rect
	*(BYTE*)0x640693 = 0xEB;

	// don't load ASSET_TYPE_MENU assets for every menu (might cause patch menus to fail)
	memset((void*)0x453406, 0x90, 5);

	// hook to increment amount of tokens allowed in expressions
	call(0x413050, ParseExpressionHookStub, PATCH_JUMP);

	Cmd_AddCommand("openmenu", Cmd_OpenMenu_f, &Cmd_OpenMenu, 0);
}
#endif