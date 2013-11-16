// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: branding for IW4M
//
// Initial author: NTAuthority
// Started: 2011-06-09
// ==========================================================

#include "StdInc.h"

#define IW4M_OLD_CODE

typedef void* (__cdecl * R_RegisterFont_t)(const char* asset);
R_RegisterFont_t R_RegisterFont = (R_RegisterFont_t)0x505670;

//typedef void (__cdecl * R_AddCmdDrawText_t)(const char* text, int, void* font, float screenX, float screenY, float, float, float rotation, float* color, int);
//R_AddCmdDrawText_t R_AddCmdDrawText = (R_AddCmdDrawText_t)0x509D80;

typedef void (__cdecl * CL_DrawTextPhysical_t)(const char* text, int, void* font, float screenX, float screenY, float, float, float* color, int);
CL_DrawTextPhysical_t CL_DrawTextPhysical = (CL_DrawTextPhysical_t)0x4376A0;

Font* LoadGameFont(const char* name, int stuff);

CallHook drawDevStuffHook;
DWORD drawDevStuffHookLoc = 0x5ACB99;

int randBetween(int min, int max)
{
	return (rand() % (max - min)) + min;
}

void DrawDemoWarning()
{
	static int lastPosChange;
	static int x, y;
	
	if (!g_nuiDraw)
	{
		return;
	}

	if (Com_Milliseconds() > (lastPosChange + 1000))
	{
		lastPosChange = Com_Milliseconds();

		x = randBetween(10, g_nuiDraw->nuiWidth - 400);
		y = randBetween(50, g_nuiDraw->nuiHeight - 50);
	}

	//void* font = DB_FindXAssetHeader(ASSET_TYPE_FONT, "fonts/objectivefont");
	void* font = LoadGameFont("fonts/objectivefont", 0);
	float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	#ifndef COMPILING_IW4M
	CL_DrawTextPhysical("Warfare^^72 pre-alpha", 0x7FFFFFFF, font, x, y, 1.5f, 1.5f, color, 0);
	#endif
	//R_AddCmdDrawText("Warfare^^72 pre-alpha", 0x7FFFFFFF, font, 10, 65, 1.5f, 1.5f, 0.0f, color, 0);
}

#pragma optimize("", off)
void __declspec(naked) DrawDevStuffHookStub()
{
	__asm
	{
		call DrawDemoWarning
		jmp drawDevStuffHook.pOriginal
	}
}
#pragma optimize("", on)

void CL_XStartPrivateMatch_f()
{
	Com_Error(2, "This feature is not available.");
}

/*
StompHook customSearchPathHook;
DWORD customSearchPathHookLoc = 0x44A510;

dvar_t** fs_basepath = (dvar_t**)0x63D0CD4;

void CustomSearchPathStuff()
{
	const char* basePath = (*fs_basepath)->current.string;
	const char* dir = "m2demo";

	__asm
	{
		mov ecx, 642EF0h
		mov ebx, basePath
		mov eax, dir
		call ecx
	}
}
*/

HWND WINAPI CreateWindowExAWrap_WC(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	if (strcmp(lpClassName, "IW4 WinConsole"))
	{
		return CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
	}

	return CreateWindowExW(dwExStyle, L"IW4 WinConsole", L"Warfare\xB2 Console", dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

HWND WINAPI CreateWindowExAWrap_G(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	return CreateWindowExW(dwExStyle, L"IW4", L"Warfare\xB2", dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

void PatchMW2_Branding()
{
	#ifdef IW4M_OLD_CODE
	// displayed build tag in UI code
	*(DWORD*)0x43F73B = (DWORD)VERSIONSTRING;

	// console '%s: %s> ' string
	*(DWORD*)0x5A44B4 = (DWORD)(VERSIONSTRING "> ");

	// console version string
	//*(DWORD*)0x4B12BB = (DWORD)(VERSIONSTRING " " BUILDHOST " (built " __DATE__ " " __TIME__ ")");

	// version string
	*(DWORD*)0x60BD56 = (DWORD)(VERSIONSTRING " (built " __DATE__ " " __TIME__ ")");

	// set fs_basegame to 'm2demo' (will apply before fs_game, unlike the above line)
	*(DWORD*)0x6431D1 = (DWORD)"m2demo";

	// increase font sizes for chat on higher resolutions
	static float float13 = 13.0f;
	static float float10 = 10.0f;
	*(float**)0x5814AE = &float13;
	*(float**)0x5814C8 = &float10;
	#endif

	#ifndef IW4M_OLD_CODE
	drawDevStuffHook.initialize(drawDevStuffHookLoc, DrawDevStuffHookStub);
	drawDevStuffHook.installHook();

	// createwindowexa on winconsole
	static DWORD wcCWEx = (DWORD)CreateWindowExAWrap_WC;
	static DWORD wcGEx = (DWORD)CreateWindowExAWrap_G;
	*(DWORD**)0x4289CA = &wcCWEx;
	*(DWORD**)0x5076AC = &wcGEx;

	// displayed build tag in UI code
	*(DWORD*)0x43F73B = (DWORD)VERSIONSTRING;

	// console '%s: %s> ' string
	*(DWORD*)0x5A44B4 = (DWORD)("W2 MP> ");

	// console version string
	*(DWORD*)0x4B12BB = (DWORD)(VERSIONSTRING " " BUILDHOST " (built " __DATE__ " " __TIME__ ")");

	// version string
	*(DWORD*)0x60BD56 = (DWORD)(VERSIONSTRING " (built " __DATE__ " " __TIME__ ")");

	// add 'm2demo' game folder
	//customSearchPathHook.initialize(5, (PBYTE)customSearchPathHookLoc);
	//customSearchPathHook.installHook(CustomSearchPathStuff, true, false);

	// set fs_basegame to 'm2demo' (will apply before fs_game, unlike the above line)
	*(DWORD*)0x6431D1 = (DWORD)"GameData/ShipFiles";

	// main to GameData/VendorPaks
	*(DWORD*)0x461A84 = (DWORD)"GameData/VendorPaks";
	*(DWORD*)0x4825A9 = (DWORD)"GameData/VendorPaks";
	*(DWORD*)0x48265A = (DWORD)"GameData/VendorPaks";
	*(DWORD*)0x4290C2 = (DWORD)"GameData/VendorPaks";

	// players -> UserData
	*(DWORD*)0x47DCB6 = (DWORD)"UserData";
	*(DWORD*)0x482410 = (DWORD)"UserData";
	*(DWORD*)0x60B1E2 = (DWORD)"UserData";
	*(DWORD*)0x5AE106 = (DWORD)"UserData";
	*(DWORD*)0x642E8F = (DWORD)"UserData";
	*(DWORD*)0x682F76 = (DWORD)"UserData";

	// devraw -> DevData/devraw, and similar
	*(DWORD*)0x4823D7 = (DWORD)"DevData/devraw";
	*(DWORD*)0x4823C4 = (DWORD)"DevData/devraw_shared";
	*(DWORD*)0x4823EA = (DWORD)"DevData/raw_shared";
	*(DWORD*)0x4823FD = (DWORD)"DevData/raw";

	// video file path
	*(DWORD*)0x51C2A4 = (DWORD)"%s\\GameData\\Videos\\%s.bik";

	// always play the intro video
	nop(0x60BECF, 2);

	*(DWORD*)0x60BED2 = (DWORD)"unskippablecinematic fdoLogo\n";

	// increase font sizes for chat on higher resolutions
	//*(BYTE*)0x5814AA = 0xEB;
	//*(BYTE*)0x5814C4 = 0xEB;
	static float float13 = 10.0f;
	static float float10 = 7.0f;

	*(float**)0x5814AE = &float13;
	*(float**)0x5814C8 = &float10;

	// safe areas; set to 1 (changing the dvar has no effect later on as the ScrPlace viewport is already made)
	static float float1 = 1.0f;
	*(float**)0x42E3A8 = &float1;
	*(float**)0x42E3D9 = &float1;

	// related to above: default cg_debugInfoCornerOffset to 0 0 instead of 0 -30
	static float float0 = 0.0f;

	*(float**)0x4F8FDB = &float0;

	// always assume a localized pak file is english
	*(BYTE*)0x4D11F3 = 0xEB;
	#endif

#ifdef PRE_RELEASE_DEMO
	// disable private parties if demo
	//*(DWORD*)0x40554E = (DWORD)CL_XStartPrivateMatch_f;
	//*(DWORD*)0x5A992E = (DWORD)CL_XStartPrivateMatch_f;
	//*(DWORD*)0x4058B9 = (DWORD)CL_XStartPrivateMatch_f; // xpartygo
	//*(DWORD*)0x4152AA = (DWORD)CL_XStartPrivateMatch_f; // map

	// use M2 playlists
	strcpy((char*)0x6EE7AC, "mp_playlists_m2");
	*(DWORD*)0x4D47FB = (DWORD)"mp_playlists_m2.ff";
#endif
}