#include "StdInc.h"

bool Installer_GetIW4C()
{
	if (GetFileAttributes(L"GameData/XAssets/english/common_gametype_mp.ff") != INVALID_FILE_ATTRIBUTES &&
		GetFileAttributes(L"GameData/VendorPaks/iw_image_00.iwd") != INVALID_FILE_ATTRIBUTES)
	{
		return true;
	}

	int mbRet = MessageBox(NULL, L"You do not seem to have the Call of Duty: Online game files.\n\n"
								 L"Warfare\xB2 can download these files for you, but this might be slow (a 4 GB download at 1.5 MB/second max).\n"
								 L"If you can find the file \"CODOL_V0.0.3.13_FULL.exe\" elsewhere, place it in the 'bin' subdirectory of the game folder (where Warfare2.exe is) before continuing.\n\n"
								 L"Do you wish to continue (and download these files if they're missing)?", L"Warfare\xB2", MB_ICONQUESTION | MB_YESNO);

	if (mbRet == IDNO)
	{
		return false;
	}

	bool hasInstaller = false;
	HANDLE hFile;

	if ((hFile = CreateFile(L"bin/CODOL_V0.0.3.13_FULL.exe", FILE_READ_ACCESS, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL)) != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER fileSize;
		GetFileSizeEx(hFile, &fileSize);

		if (fileSize.QuadPart == 4254180289)
		{
			hasInstaller = true;
		}

		CloseHandle(hFile);
	}

	if (!hasInstaller)
	{
		CL_QueueDownload("http://down.qq.com/codol/full/Formal/CODOL_V0.0.3.13_FULL.exe", "bin/CODOL_V0.0.3.13_FULL.exe", 4254180289, false, 5);

		UI_DoCreation();

		if (!DL_RunLoop())
		{
			UI_DoDestruction();
			return false;
		}

		UI_DoDestruction();
	}

	wchar_t exeName[512];
	GetModuleFileName(GetModuleHandle(NULL), exeName, sizeof(exeName) / 2);

	wchar_t* exeBaseName = wcsrchr(exeName, L'\\');
	exeBaseName[0] = L'\0';
	exeBaseName++;

	SetEnvironmentVariable(L"INSTALLDIR", exeName);

	SHELLEXECUTEINFO execInfo;
	execInfo.cbSize = sizeof(execInfo);
	execInfo.lpFile = L"bin\\installer\\setup_codo.cmd";
	execInfo.fMask = SEE_MASK_DOENVSUBST | SEE_MASK_NOCLOSEPROCESS;
	execInfo.hwnd = NULL;
	execInfo.lpVerb = L"open";
	execInfo.lpParameters = NULL;
	execInfo.lpDirectory = exeName;
	execInfo.nShow = SW_SHOWDEFAULT;
	execInfo.hInstApp = (HINSTANCE)SE_ERR_DDEFAIL;
	
	ShellExecuteEx(&execInfo);

	WaitForSingleObject(execInfo.hProcess, INFINITE);
	CloseHandle(execInfo.hProcess);
	//ShellExecute(NULL, L"open", L"bin\\installer\\setup_codo.cmd", NULL, exeName, SW_SHOWDEFAULT);
}