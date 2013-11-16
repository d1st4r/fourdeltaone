// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Shows server name/IP below the cgame scoreboard.
//
// Initial author: NTAuthority
// Started: 2011-11-24
// ==========================================================

#include "StdInc.h"

const char* svi_hostname = (const char*)0x7ED3F8;

typedef void* (__cdecl * UI_GetContext_t)(void*);
UI_GetContext_t UI_GetContext = (UI_GetContext_t)0x4F8940;

typedef float (__cdecl * UI_GetScoreboardLeft_t)();
UI_GetScoreboardLeft_t UI_GetScoreboardLeft = (UI_GetScoreboardLeft_t)0x590390;

typedef int (__cdecl * UI_TextWidth_t)(const char*, int flags, Font*, float);
UI_TextWidth_t UI_TextWidth = (UI_TextWidth_t)0x6315C0;

typedef void (__cdecl * UI_DrawText_t)(void* context, const char* text, int color, Font* font, float x, float y, int, int, float fontSize, float* stuff, int);
UI_DrawText_t UI_DrawText = (UI_DrawText_t)0x49C0D0;

float GetScoreboardLeft(void* a1)
{
	__asm
	{
		mov eax, a1
		call UI_GetScoreboardLeft
	}
}

void DrawScoreboardInfo(void* a1)
{
	Font* font = (Font*)DB_FindXAssetHeader(ASSET_TYPE_FONT, "fonts/bigfont");
	void* cxt = UI_GetContext(a1);

	float fontSize = 0.35f;

	const char* ipText = CL_GetServerIPAddress();

	if (!strcmp(ipText, "0.0.0.0:0"))
	{
		ipText = "Listen Server";
	}

	// get x positions
	float left = GetScoreboardLeft(a1);
	float hostnameX = left + 9.0f;

	dvar_t* cg_scoreboardWidth = Dvar_FindVar("cg_scoreboardWidth");
	float ipX = left + cg_scoreboardWidth->current.value;
	ipX -= UI_TextWidth(ipText, 0, font, fontSize);
	ipX -= 9;

	// get height
	dvar_t* cg_scoreboardHeight = Dvar_FindVar("cg_scoreboardHeight");
	float y = (480.0f - cg_scoreboardHeight->current.value) * 0.5f;
	y += cg_scoreboardHeight->current.value + 6.0f;

	// draw hostname
	UI_DrawText(cxt, svi_hostname, 0x7FFFFFFF, font, hostnameX, y, 0, 0, fontSize, (float*)0x747F34, 3);

	// draw IP
	UI_DrawText(cxt, ipText, 0x7FFFFFFF, font, ipX, y, 0, 0, fontSize, (float*)0x747F34, 3);
}

CallHook cgDrawScoreboardHook;
DWORD cgDrawScoreboardHookLoc = 0x4FC6EA;

void __declspec(naked) CG_DrawScoreboardHookFunc()
{
	__asm
	{
		push eax
		call DrawScoreboardInfo
		pop eax
		jmp cgDrawScoreboardHook.pOriginal
	}
}

void PatchMW2_ScoreboardInfo()
{
	cgDrawScoreboardHook.initialize(cgDrawScoreboardHookLoc, CG_DrawScoreboardHookFunc);
	cgDrawScoreboardHook.installHook();
}