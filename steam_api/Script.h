// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Functionality to interact with the GameScript 
//          runtime.
//
// Initial author: NTAuthority
// Started: 2011-12-19
// ==========================================================

#pragma once

typedef int scr_entref_t;
typedef void (__cdecl * scr_function_t)(scr_entref_t);

void Scr_DeclareFunction(const char* name, scr_function_t func, bool developerOnly);
int GScr_LoadScriptAndLabel(char* script, char* function);

typedef int (__cdecl * Scr_GetNumParam_t)(void);
extern Scr_GetNumParam_t Scr_GetNumParam;

typedef char* (__cdecl * Scr_GetString_t)(int);
extern Scr_GetString_t Scr_GetString;

typedef float (__cdecl * Scr_GetFloat_t)(int);
extern Scr_GetFloat_t Scr_GetFloat;

typedef int (__cdecl * Scr_GetInt_t)(int);
extern Scr_GetInt_t Scr_GetInt;

typedef int (__cdecl * Scr_GetFunctionHandle_t)(char*, char*);
extern Scr_GetFunctionHandle_t Scr_GetFunctionHandle;

typedef int (__cdecl *Scr_AddString_t)(char*);
extern Scr_AddString_t Scr_AddString;

typedef int (__cdecl * Scr_AddInt_t)(int);
extern Scr_AddInt_t Scr_AddInt;

typedef int (__cdecl * Scr_AddFloat_t)(float);
extern Scr_AddFloat_t Scr_AddFloat;

typedef int (__cdecl * Scr_LoadScript_t)(char*);
extern Scr_LoadScript_t Scr_LoadScript;

typedef int (__cdecl * Scr_ExecThread_t)(int, int);
extern Scr_ExecThread_t Scr_ExecThread;

typedef int (__cdecl * Scr_ExecEntThread_t)(int*, int, int);
extern Scr_ExecEntThread_t Scr_ExecEntThread;

typedef int (__cdecl * Scr_FreeThread_t)(int);
extern Scr_FreeThread_t Scr_FreeThread;

typedef char* (__cdecl * SL_ConvertToString_t)(unsigned short);
extern SL_ConvertToString_t SL_ConvertToString;