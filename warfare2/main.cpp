// ==========================================================
// Warfare2 project
// 
// Component: warfare2
// Purpose: Initialization code for the game executable.
//
// Initial author: NTAuthority
// Started: 2013-06-xx
// ==========================================================

#include "StdInc.h"

#pragma code_seg(".w2")
#pragma data_seg(".w2d")
#pragma bss_seg(".w2b")
#pragma const_seg(".w2r")

#pragma comment(linker, "/merge:.data=.w2d")
#pragma comment(linker, "/merge:.rdata=.w2r")
#pragma comment(linker, "/merge:.w2=.zdata")
#pragma comment(linker, "/merge:.text=.zdata")
#pragma comment(linker, "/section:.zdata,rwe")
#pragma comment(linker, "/section:.ctext,rwe")

#include "aes256.h"

#include <stdio.h>
#include <windows.h>

#include "MemoryModule.h"

__declspec(thread) int tls1;

extern "C" BOOL WINAPI _CRT_INIT(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved);

void main()
{
	_CRT_INIT(GetModuleHandle(NULL), DLL_PROCESS_ATTACH, NULL);

	if (Bootstrap_RunInit())
	{
		ExitProcess(0);
	}

	wchar_t exeName[512];
	GetModuleFileName(GetModuleHandle(NULL), exeName, sizeof(exeName) / 2);

	wchar_t* exeBaseName = wcsrchr(exeName, L'\\');
	exeBaseName[0] = L'\0';
	exeBaseName++;

	// expand the path to include bin/
	static wchar_t path[32768];
	static wchar_t newPath[32768];
	GetEnvironmentVariable(L"PATH", path, sizeof(path));

	_snwprintf(newPath, sizeof(newPath), L"%s\\bin;%s", exeName, path);

	SetEnvironmentVariable(L"PATH", newPath);

	// set the current directory
	SetCurrentDirectory(exeName);

	bool devMode = false;

	if (GetFileAttributes(va(L"%s.iw4dev", exeBaseName)) != INVALID_FILE_ATTRIBUTES || wcsstr(GetCommandLine(), L"dedicated"))
	{
		devMode = true;
	}

	if (!devMode)
	{
		if (!Bootstrap_DoBootstrap())
		{
			ExitProcess(0);
			return;
		}
	}

	if (!Installer_GetIW4C())
	{
		ExitProcess(0);
	}

	// load the game
	W2_StartGame();
}

void W2_StartGame()
{
	uint8_t* data = new uint8_t[3936256];

	FILE* f = fopen("bin\\w2core.dll", "rb");
	fread(data, 1, 3936256, f);
	fclose(f);

	tls1 = 1;

	if (tls1 == 2)
	{
		return;
	}

	HMODULE base = GetModuleHandle(NULL);
	PIMAGE_DOS_HEADER dos_header = (PIMAGE_DOS_HEADER)base;
	PIMAGE_NT_HEADERS header = (PIMAGE_NT_HEADERS)&((const unsigned char *)(base))[dos_header->e_lfanew];

	DWORD oldProtect;

	VirtualProtect(header, sizeof(*header), PAGE_EXECUTE_READWRITE, &oldProtect);
	header->OptionalHeader.AddressOfEntryPoint = 0x2BAC0F;

	PIMAGE_TLS_DIRECTORY tlsDirectory = (PIMAGE_TLS_DIRECTORY)(((char*)base) + header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);
	*(DWORD*)0x66D94A8 = *(DWORD*)(tlsDirectory->AddressOfIndex);

	MemoryLoadLibrary(data);
}

__declspec(thread) int tls2;
__declspec(thread) int tls3;
__declspec(thread) int tls4;
__declspec(thread) int tls5;
__declspec(thread) int tls6;
__declspec(thread) int tls7;
__declspec(thread) int tls8;
__declspec(thread) int tls9;
__declspec(thread) int tls10;

#include "aes256.c"

#pragma bss_seg(".ctext")
//char text[0x2D6000];
char text[0x2D531F];

#pragma bss_seg(".crdata")
char rdata[0xBD74C];

#pragma bss_seg(".cdata")
char data[0x68808F8];

#pragma data_seg(".zdata")
char zdata[200] = { 1 };