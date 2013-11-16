// ==========================================================
// project 'secretSchemes'
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Manages the initialization of iw4cli.
//
// Initial author: NTAuthority
// Started: 2011-05-04
// ==========================================================

#include "StdInc.h"

#if LINUX
#pragma comment(linker,"/FIXED /BASE:0x8000000 /MERGE:.rdata=.text /SECTION:.text,EWR")
#endif

#define IW4M_OLD_CODE

static BYTE originalCode[5];
static PBYTE originalEP = 0;

void HideCode_FindDeviceIoControl();

void Main_UnprotectModule(HMODULE hModule)
{
	PIMAGE_DOS_HEADER header = (PIMAGE_DOS_HEADER)hModule;
	PIMAGE_NT_HEADERS ntHeader = (PIMAGE_NT_HEADERS)((DWORD)hModule + header->e_lfanew);

	// unprotect the entire PE image
	SIZE_T size = ntHeader->OptionalHeader.SizeOfImage;
	DWORD oldProtect;
	VirtualProtect((LPVOID)hModule, size, PAGE_EXECUTE_READWRITE, &oldProtect);
}

void Main_DoInit()
{
	// unprotect our entire PE image
	HMODULE hModule;
	if (SUCCEEDED(GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCSTR)Main_DoInit, &hModule)))
	{
		Main_UnprotectModule(hModule);
	}

	#ifndef IW4M_OLD_CODE
	static char currentPath[16384];
	GetEnvironmentVariable("PATH", currentPath, sizeof(currentPath));
	static char newPath[19384];
	sprintf(newPath, "S:\\games\\steam\\steamapps\\common\\zero gear\\server;E:\\mm\\mw2;%s", currentPath);
	#endif

	//SetEnvironmentVariable("PATH", newPath);
	//SetCurrentDirectoryA("S:\\games\\steam\\steamapps\\common\\zero gear\\server");
	//SetCurrentDirectoryA("E:\\mm\\mw2");

	HideCode_FindDeviceIoControl();
	Sys_RunInit();

	// return to the original EP
	memcpy(originalEP, &originalCode, sizeof(originalCode));
	__asm jmp originalEP
}

void Main_SetSafeInit()
{
	// find the entry point for the executable process, set page access, and replace the EP
	HMODULE hModule = GetModuleHandle(NULL); // passing NULL should be safe even with the loader lock being held (according to ReactOS ldr.c)

	if (hModule)
	{
		PIMAGE_DOS_HEADER header = (PIMAGE_DOS_HEADER)hModule;
		PIMAGE_NT_HEADERS ntHeader = (PIMAGE_NT_HEADERS)((DWORD)hModule + header->e_lfanew);

		Main_UnprotectModule(hModule);

		// back up original code
		PBYTE ep = (PBYTE)((DWORD)hModule + ntHeader->OptionalHeader.AddressOfEntryPoint);
		memcpy(originalCode, ep, sizeof(originalCode));

		// patch to call our EP
		int newEP = (int)Main_DoInit - ((int)ep + 5);
		ep[0] = 0xE9; // for some reason this doesn't work properly when run under the debugger
		memcpy(&ep[1], &newEP, 4);

		originalEP = ep;
	}
}

extern "C" void __declspec(dllimport) CCAPI_Initialize();

bool __stdcall DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		// check for anything IW4-related
		if (*(DWORD*)0x41BFA3 != 0x7CF08B1C)
		{
			return true;
		}

#if !LINUX
		Main_SetSafeInit();
#endif

#if !PRE_RELEASE_DEMO
		CCAPI_Initialize();
#endif

		static_assert(sizeof(client_t) == 681872, "sizeof(client_t) is not 681872");
		static_assert(sizeof(gentity_t) == 628, "sizeof(gentity_t) is not 628");
	}
	/*else if (dwReason == DLL_THREAD_ATTACH)
	{
		DWORD tlsIndex = *(DWORD*)0x66D94A8;
		TlsSetValue(tlsIndex, new char[0x28]);
	}
	else if (dwReason == DLL_THREAD_DETACH)
	{
		DWORD tlsIndex = *(DWORD*)0x66D94A8;
		delete[] TlsGetValue(tlsIndex);
	}*/

	return true;
}