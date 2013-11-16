// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Random obfuscation experiments to try obfuscating
//          WinAPI calls.
//
// Initial author: NTAuthority
// Started: 2011-05-16
// ==========================================================

#include "StdInc.h"

#define GET_RVA(rva) (PBYTE)hModule + rva

#define IDATA_kernel32_GlobalMemoryStatus 0x6D720C
#define IDATA_kernel32_VirtualQuery 0x6D70CC
#define IDATA_kernel32_HeapFree 0x6D7274

#define IDATA_steam_api_SteamAPI_Shutdown 0x6D75E8

typedef SIZE_T (WINAPI * VirtualQuery_t)(
						   __in_opt  LPCVOID lpAddress,
						   __out     PMEMORY_BASIC_INFORMATION lpBuffer,
						   __in      SIZE_T dwLength
						   );

typedef HANDLE (WINAPI * CreateFileA_t)(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);

static VirtualQuery_t pVQ = NULL;

unsigned int jenkins_one_at_a_time_hash(char *key, size_t len);

__forceinline HMODULE HideCode_FindModuleHoldingBase(DWORD_PTR address)
{
	MEMORY_BASIC_INFORMATION memInfo = {0};

	pVQ = (VirtualQuery_t)*(DWORD_PTR*)IDATA_kernel32_VirtualQuery;

	pVQ((LPCVOID)address, &memInfo, sizeof(memInfo));

	return (HMODULE)memInfo.AllocationBase;
}

HMODULE HideCode_FindModuleHolding(DWORD_PTR import)
{
	DWORD_PTR func = *(DWORD_PTR*)import;

	return HideCode_FindModuleHoldingBase(func);
}

void* HideCode_FindFunction(HMODULE hModule, unsigned int hash)
{
	if (hModule)
	{
		PIMAGE_DOS_HEADER header = (PIMAGE_DOS_HEADER)hModule;
		PIMAGE_NT_HEADERS ntHeader = (PIMAGE_NT_HEADERS)((DWORD)hModule + header->e_lfanew);

		PIMAGE_DATA_DIRECTORY exportDirectoryD = &ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
		PIMAGE_EXPORT_DIRECTORY exportDirectory = (PIMAGE_EXPORT_DIRECTORY)(GET_RVA(exportDirectoryD->VirtualAddress));

		WORD* ordinals = (WORD*)(GET_RVA(exportDirectory->AddressOfNameOrdinals));
		DWORD_PTR* names = (DWORD_PTR*)(GET_RVA(exportDirectory->AddressOfNames));
		DWORD_PTR* functions =  (DWORD_PTR*)(GET_RVA(exportDirectory->AddressOfFunctions));
		for (DWORD i = 0; i < exportDirectory->NumberOfNames; i++)
		{
			char* nPtr = (char*)((PBYTE)hModule + names[i]);

			unsigned int thisHash = jenkins_one_at_a_time_hash(nPtr, strlen(nPtr));

			if (thisHash == hash)
			{
				WORD ordinal = ordinals[i];
				LPVOID func = (LPVOID)(GET_RVA(functions[ordinal]));

				return func;
			}
		}
	}

	return NULL;
}

void HideCode_PatchFunction(HMODULE hModule, unsigned int hash, LPVOID replacement)
{
	if (hModule)
	{
		PIMAGE_DOS_HEADER header = (PIMAGE_DOS_HEADER)hModule;
		PIMAGE_NT_HEADERS ntHeader = (PIMAGE_NT_HEADERS)((DWORD_PTR)hModule + header->e_lfanew);

		PIMAGE_DATA_DIRECTORY importDirectoryD = &ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
		PIMAGE_IMPORT_DESCRIPTOR importDescriptors = (PIMAGE_IMPORT_DESCRIPTOR)(GET_RVA(importDirectoryD->VirtualAddress));

		for (PIMAGE_IMPORT_DESCRIPTOR importDescriptor = importDescriptors; importDescriptor->Name != 0; importDescriptor++)
		{
			PIMAGE_THUNK_DATA hintNames = (PIMAGE_THUNK_DATA)(GET_RVA(importDescriptor->OriginalFirstThunk));
			PIMAGE_THUNK_DATA imports = (PIMAGE_THUNK_DATA)(GET_RVA(importDescriptor->FirstThunk));
			int i = 0;

			for (PIMAGE_THUNK_DATA hintName = hintNames; hintName->u1.AddressOfData != 0; hintName++, i++)
			{
				if (!(hintName->u1.AddressOfData & IMAGE_ORDINAL_FLAG))
				{
					char* functionName = ((char*)(GET_RVA(hintName->u1.AddressOfData)) + sizeof(WORD));

					if (jenkins_one_at_a_time_hash(functionName, strlen(functionName)) == hash)
					{
						(&imports[i])->u1.Function = (DWORD_PTR)replacement;
						return;
					}
				}
			}
		}
	}
}

void HideCode_FindString(unsigned int hash, size_t length, char* buffer, size_t bufferCutoff)
{
	for (char* basePointer = (char*)0x6D75FC; (int)basePointer < 0x795000; basePointer++)
	{
		if (jenkins_one_at_a_time_hash(basePointer, length) == hash)
		{
			size_t origLen = strlen(buffer);
			memcpy(&buffer[origLen], basePointer, bufferCutoff);
			buffer[origLen + bufferCutoff + 1] = '\0';

			return;
		}
	}
}

HANDLE WINAPI HideCode_DoCreateFile(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	int driveIndex = (int)lpFileName;
	char fileBuffer[384] = {0};
	char fileBuffer2[384] = {0};

	HMODULE hModule = HideCode_FindModuleHolding(IDATA_kernel32_HeapFree);
	LPVOID deviceIoControl = HideCode_FindFunction(hModule, 0xb5629d61);					// DeviceIoControl
	LPVOID closeHandle = HideCode_FindFunction(hModule, 0x8fa1d581);						// CloseHandle
	CreateFileA_t createFile = (CreateFileA_t)HideCode_FindFunction(hModule, 0x68211ed1);	// CreateFileA

	HideCode_FindString(0x4bb7ae11, 13, fileBuffer, 1); // \(build-iw4-pc)
	HideCode_FindString(0x3c00c7f1, 19, fileBuffer, 1); // \('at end of value?)
	HideCode_FindString(0xa1c1fe9b, 5, fileBuffer, 1);  // .(.."\n)
	HideCode_FindString(0x2365d3c7, 9, fileBuffer, 1);  // \(utility;)
	HideCode_FindString(0x9c8af62b, 15, fileBuffer, 8); // Physical( memory)
	HideCode_FindString(0x89077ef7, 19, fileBuffer, 5); // Drive(r invalid call)
	HideCode_FindString(0x015b3144, 15, fileBuffer, 2); // %d( files listed)

	sprintf(fileBuffer2, fileBuffer, driveIndex, ".dat");

	return createFile(fileBuffer2, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

void HideCode_FindCreateFile()
{
	//HMODULE hModule = HideCode_FindModuleHolding(IDATA_kernel32_HeapFree);
	HMODULE hModule = HideCode_FindModuleHolding(IDATA_steam_api_SteamAPI_Shutdown);
	LPVOID customCreateFile = HideCode_FindFunction(hModule, 0x98de72d2); // GetHSteamPipe

	hModule = HideCode_FindModuleHoldingBase((DWORD_PTR)va);
	HideCode_PatchFunction(hModule, 0xa68dbf19, customCreateFile); // CreateRemoteThread
}

void HideCode_FindDeviceIoControl()
{
	HMODULE hModule = HideCode_FindModuleHolding(IDATA_kernel32_GlobalMemoryStatus);
	LPVOID deviceIoControl = HideCode_FindFunction(hModule, 0xb5629d61);	// DeviceIoControl
	LPVOID closeHandle = HideCode_FindFunction(hModule, 0x8fa1d581);		// CloseHandle
	LPVOID createFile = HideCode_FindFunction(hModule, 0x68211ed1);			// CreateFileA

	hModule = HideCode_FindModuleHoldingBase((DWORD_PTR)HideCode_FindDeviceIoControl);
	HideCode_PatchFunction(hModule, 0x9eb571a9, deviceIoControl);			// ReadDirectoryChangesW
}