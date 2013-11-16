// ==========================================================
// IW5M project
// 
// Component: client
// Sub-component: clientdll
// Purpose: An easy-to-use installer for game DLC.
//
// Initial author: NTAuthority
// Started: 2012-10-11
// ==========================================================

#include "StdInc.h"
#include <shellapi.h>
#include <Shlobj.h>

void DLCInstaller_Register()
{
	LSTATUS result;

#define CHECK_STATUS(x) \
	result = (x); \
	if (result != ERROR_SUCCESS) { \
	Trace("DLC", #x " failed: %x", result); \
	return; \
	}

	HKEY iw5dlc;
	wchar_t path[MAX_PATH];
	wchar_t command[1024];

	GetModuleFileNameW(NULL, path, sizeof(path));
	swprintf_s(command, L"\"%s\" -dlc \"%%1\"", path);

	CHECK_STATUS(RegCreateKey(HKEY_CURRENT_USER, "SOFTWARE\\Classes\\.iw4dlc", &iw5dlc));
	CHECK_STATUS(RegSetValueEx(iw5dlc, NULL, 0, REG_SZ, (BYTE*)"IW4M.DLCPack", 13));
	CHECK_STATUS(RegCloseKey(iw5dlc));

	CHECK_STATUS(RegCreateKey(HKEY_CURRENT_USER, "SOFTWARE\\Classes\\IW4M.DLCPack", &iw5dlc));
	CHECK_STATUS(RegSetValueEx(iw5dlc, NULL, 0, REG_SZ, (BYTE*)"IW4M DLC Package", 17));
	CHECK_STATUS(RegCloseKey(iw5dlc));

	CHECK_STATUS(RegCreateKey(HKEY_CURRENT_USER, "SOFTWARE\\Classes\\IW4M.DLCPack\\shell\\open\\command", &iw5dlc));
	CHECK_STATUS(RegSetValueExW(iw5dlc, NULL, 0, REG_SZ, (BYTE*)command, (wcslen(command) * sizeof(wchar_t)) + 2));
	CHECK_STATUS(RegCloseKey(iw5dlc));
}

void DLCInstaller_Install(const wchar_t* path)
{
	wchar_t spath[MAX_PATH];
	wchar_t epath[MAX_PATH];
	char dpath[MAX_PATH];

	// get relevant directories
	GetModuleFileNameW(NULL, epath, sizeof(epath));
	wcsrchr(epath, L'\\')[0] = L'\0';

	GetModuleFileName(NULL, dpath, sizeof(dpath));
	strrchr(dpath, '\\')[0] = '\0';

	wcscpy_s(spath, path);
	wcsrchr(spath, L'\\')[0] = L'\0';

	SetCurrentDirectoryW(epath);

	// so, do we want to hop into the dl-c?
	FILE* dlc = _wfopen(path, L"r");

	char dlcName[1024];
	wchar_t dlcFile[1024];
	wchar_t dlcSourcePaths[MAX_PATH * 100];
	wchar_t dlcTargetPaths[MAX_PATH * 100];

	wchar_t* dlcSource = dlcSourcePaths;
	wchar_t* dlcTarget = dlcTargetPaths;

	if (fscanf(dlc, "name: %[^\n]\n", dlcName) != 1)
	{
		return;
	}

	// build a file list now so they won't be surprised when the DLC file is invalid
	while (fwscanf(dlc, L"file: %s\n", dlcFile) == 1)
	{
		wchar_t dlcFullFile[1024];
		const wchar_t* ext = wcsrchr(dlcFile, '.');

		if (_wcsicmp(ext, L".ff") == NULL)
		{
			swprintf(dlcFullFile, L"zone\\dlc\\%s", dlcFile);
		}
		else if (_wcsicmp(ext, L".iwd") == NULL)
		{
			swprintf(dlcFullFile, L"main\\%s", dlcFile);
		}
		else
		{
			continue;
		}

		swprintf(dlcSource, L"%s\\%s", spath, dlcFullFile);
		dlcSource += wcslen(dlcSource) + 1;

		swprintf(dlcTarget, L"%s\\%s", epath, dlcFullFile);
		dlcTarget += wcslen(dlcTarget) + 1;
	}

	dlcSource[0] = '\0';
	dlcTarget[0] = '\0';

	// ask the user whether they want to install
	int mbResult = MessageBoxA(NULL, va("Do you want to install the '%s' content pack for IW4M to the game directory '%s'?", dlcName, dpath), "IW4M DLC", MB_YESNO | MB_ICONQUESTION);

	if (mbResult == IDNO)
	{
		return;
	}

	// okay, the user wants to proceed.
	swprintf(dlcFile, L"%s\\zone\\dlc", epath);
	SHCreateDirectoryExW(NULL, dlcFile, NULL);

	SHFILEOPSTRUCTW fileOp;
	fileOp.hwnd = NULL;
	fileOp.wFunc = FO_COPY;
	fileOp.pFrom = dlcSourcePaths;
	fileOp.pTo = dlcTargetPaths;
	fileOp.fFlags = FOF_MULTIDESTFILES | FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR;

	int result = SHFileOperationW(&fileOp);

	if (result == ERROR_SUCCESS && !fileOp.fAnyOperationsAborted)
	{
		MessageBoxA(NULL, va("The '%s' IW4M content pack has been successfully installed.", dlcName), "IW4M DLC", MB_OK | MB_ICONINFORMATION);
	}
	else
	{
		MessageBoxA(NULL, va("The '%s' IW4M content pack did not install successfully. Error code 0x%x was returned.", dlcName, result), "IW4M DLC", MB_OK | MB_ICONERROR);
	}
}

void DLCInstaller_Run()
{
	DLCInstaller_Register();

	int argc;
	wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);

	if (argc == 3)
	{
		if (wcscmp(argv[1], L"-dlc") == NULL)
		{
			FreeConsole();

			DLCInstaller_Install(argv[2]);

			LocalFree(argv);
			ExitProcess(0);
		}
	}

	LocalFree(argv);
}