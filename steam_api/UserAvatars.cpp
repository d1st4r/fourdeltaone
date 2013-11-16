// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Material/image modification code
//
// Initial author: NTAuthority
// Started: 2013-03-09
// ==========================================================

#include "StdInc.h"
#include <d3d9.h>
#include <png.h>

GfxImage avatars[19];
Material* scoreboardMats[19];
Material playercardMats[4];
int userIDs[19];

struct avatarinfo_t
{
	int guid;
	uint8_t* buffer;
};
avatarinfo_t avatarinfo[19];

typedef char* (*SetPlayerCardInfo_t)(int a1, int a2, int a3);
SetPlayerCardInfo_t SetPlayerCardInfo = (SetPlayerCardInfo_t)0x458D80;

extern void Image_Setup(GfxImage* image, short width, short height, short depth, unsigned int flags, int format);
extern Material* Material_CreateWithImage(const char* name, GfxImage* image);

GfxImage* UserAvatars_GetImages(int num)
{
	return &avatars[num];
}

Material* UserAvatars_GetMaterial(const char* filename)
{
	if (startsWith(filename, "playercard_"))
	{
		int slot = atoi(filename + strlen("playercard_"));
		switch (slot)
		{
		case 6:
			slot = 0;
			break;
		case 7:
			slot = 1;
			break;
		case 8:
			slot = 2;
			break;
		case 0:
			memcpy(&playercardMats[3], scoreboardMats[18], sizeof(Material));
			slot = 3;
		}
		return &playercardMats[slot];
	}

	return NULL;
}

char* SetPlayerCardInfoHook(int a1, int a2, int slot)
{
	int cid = atoi(Cmd_Argv(1));
	int slot2;
	switch (slot)
	{
	case 6:
		slot2 = 0;
		break;
	case 7:
		slot2 = 1;
		break;
	case 8:
		slot2 = 2;
		break;
	}
	memcpy(&playercardMats[slot2], scoreboardMats[cid], sizeof(Material));
	Com_Printf(0, "Setting card info slot %d (%d) mat to client %d's mat\n", slot, slot2, cid);
	return SetPlayerCardInfo(a1, a2, slot);
}

CallHook initGFXImages;
DWORD initGFXImagesLoc = 0x506C90;

extern int Auth_GetUserID();
extern void DownloadUserAvatar(int clientnum, int guid);

#pragma optimize("", off)
void InitGFXImages()
{
	for (int i = 0; i < 19; i++)
	{
		Image_Setup(&avatars[i], 96, 96, 1, 0x1000003, D3DFMT_A8R8G8B8);
		scoreboardMats[i] = Material_CreateWithImage("wub wub wub wub", NULL);
		scoreboardMats[i]->maps->image = &avatars[i];
		avatarinfo[i].guid = -1;
		avatarinfo[i].buffer = (uint8_t*)malloc(1024 * 100);
	}

	memcpy(&playercardMats[0], scoreboardMats[0], sizeof(Material));
	memcpy(&playercardMats[1], scoreboardMats[0], sizeof(Material));
	memcpy(&playercardMats[2], scoreboardMats[0], sizeof(Material));
	memcpy(&playercardMats[3], scoreboardMats[0], sizeof(Material));

	DownloadUserAvatar(18, Auth_GetUserID());
}
#pragma optimize("", on)

void __declspec(naked) InitGFXImagesHook()
{
	__asm
	{
		call initGFXImages.pOriginal
		jmp InitGFXImages
	}
}

void UserAvatars_SendGUIDsToAllClients()
{
	const char* list = va("%c", 20);
	for(int i = 0; i < 18; i++)
	{
		if(svs_clients[i].state >= 3)
		{
			list = va("%s %d", list, svs_clients[i].steamid);
		}
		else
		{
			list = va("%s 0", list);
		}
	}
	SV_GameSendServerCommand(-1, 0, list);
	//Com_Printf(0, "%s\n", list);
}

void RecieveUserAvatarCB(NPAsync<NPGetUserAvatarResult>* async)
{
	int client = (int)async->GetUserData();

	if (async->GetResult()->result != GetFileResultOK)
	{
		Com_Printf(0, "failed for client %d (guid %d): %d\n", client, async->GetResult()->guid, async->GetResult()->result);
		return;
	}
	Com_Printf(0, "client %d (guid %d): success in getting data: length: %d\n", client, async->GetResult()->guid, async->GetResult()->fileSize);

	if (avatarinfo[client].guid != async->GetResult()->guid)
	{
		Com_Printf(0, "client %d (guid %d): avatar response invalid\n", client, async->GetResult()->guid);
		return; // this avatar response is now invalid (client disconnected, etc)
	}

	uint8_t* buffer = async->GetResult()->buffer;
	int fileSize = async->GetResult()->fileSize;

	//FILE* t = tmpfile();
	const char* fn = _tempnam(NULL, "iw4");
	FILE* t = fopen(fn, "wb+");
	fwrite(buffer, fileSize, 1, t);
	fseek(t, 0, SEEK_SET);

	byte pngSignature[8];
	fread(pngSignature, 8, 1, t);
	if(!png_check_sig(pngSignature, 8))
	{
		Com_Printf(0, "client %d (guid %d): png invalid\n", client, async->GetResult()->guid);
		return;
	}

	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	png_infop info_ptr = png_create_info_struct(png_ptr);

	png_set_sig_bytes(png_ptr, 8);
	png_init_io(png_ptr, t);
	png_read_info(png_ptr, info_ptr);

	D3DLOCKED_RECT lockedRect;
	((IDirect3DTexture9*)avatars[client].texture)->LockRect(0, &lockedRect, NULL, 0);

	size_t bytesPerRow = png_get_rowbytes(png_ptr, info_ptr);
	int bytesPerPixel = bytesPerRow / 96;

	byte* rowData = (byte*)malloc(bytesPerRow);
	byte* rect = (byte*)lockedRect.pBits;
	Com_Printf(0, "client %d (guid %d): png bpw: %d bpp: %d pitch: %d\n", client, async->GetResult()->guid, bytesPerRow, bytesPerPixel, lockedRect.Pitch);

	for(int y = 0; y < 96; y++)
	{
		png_read_row(png_ptr, rowData, NULL);
		char a = '\xFF';
		char b = 0;
		for(int x = 0; x < 96; x++)
		{
			//the bad thing is, the bytes are reversed :( lets not worry about that for now, k
			memcpy(rect + y * lockedRect.Pitch + x * 4 + 0, rowData + x * bytesPerPixel + 2, 1);
			memcpy(rect + y * lockedRect.Pitch + x * 4 + 1, rowData + x * bytesPerPixel + 1, 1);
			memcpy(rect + y * lockedRect.Pitch + x * 4 + 2, rowData + x * bytesPerPixel + 0, 1);
			if(bytesPerPixel == 4)
				memcpy(rect + y * lockedRect.Pitch + x * 4 + 3, rowData + x * bytesPerPixel + 3, 1);
			else
				memcpy(rect + y * lockedRect.Pitch + x * 4 + 3, &a, 1);
		}
	}

	((IDirect3DTexture9*)avatars[client].texture)->UnlockRect(0);

	free(rowData);
	fclose(t);
	unlink(fn);
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	Com_Printf(0, "Finished with client %d (guid %d)\n", client, async->GetResult()->guid);
}

void DownloadUserAvatar(int clientnum, int guid)
{
	if (avatarinfo[clientnum].guid != guid)
	{
		Com_Printf(0, "Starting for client %d (guid %d)\n", clientnum, guid);
		NPAsync<NPGetUserAvatarResult>* async = NP_GetUserAvatar(guid, avatarinfo[clientnum].buffer, 1024 * 100);
		async->SetCallback(RecieveUserAvatarCB, (void*)clientnum);
		avatarinfo[clientnum].guid = guid;
	}
}

int OnServerCommandFunc()
{
	if (Cmd_Argc() > 17 && Cmd_Argv(0)[0] == 20)
	{
		for (int client = 0; client < 18; client++)
		{
			int guid = atoi(Cmd_Argv(client + 1));
			if(guid != 0)
			{
				DownloadUserAvatar(client, guid);

				userIDs[client] = guid;
			}
		}
		return 1;
	}
	return 0;
}

DWORD OnServerCommandRetn = 0x594536;
DWORD OnServerCommandMidRetn = 0x5944AE;
void __declspec(naked) OnServerCommandStub()
{
	__asm call OnServerCommandFunc
	__asm test al, al
	__asm jnz returnStuff
	__asm jmp OnServerCommandMidRetn

returnStuff:
	__asm jmp OnServerCommandRetn
}

typedef void (__cdecl * UI_DrawHandlePic_t)(float* placement, float x, float y, float width, float height, int, int, int, Material*);
UI_DrawHandlePic_t UI_DrawHandlePic = (UI_DrawHandlePic_t)0x4D0EA0;

Material* DrawScoreboardRankGetImage(int* clientNum)
{
	return scoreboardMats[*clientNum];
}

void __declspec(naked) DrawScoreboardRankStub()
{
	__asm
	{
		push ebp
		call DrawScoreboardRankGetImage
		add esp, 4h

		mov [esp + 24h], eax
		jmp UI_DrawHandlePic
	}
}

void PatchMW2_UserAvatars()
{
	call(0x594295, SetPlayerCardInfoHook, PATCH_CALL);

	initGFXImages.initialize(initGFXImagesLoc, InitGFXImagesHook);
	initGFXImages.installHook();

	call(0x59449F, OnServerCommandStub, PATCH_JUMP);

	call(0x5910BC, DrawScoreboardRankStub, PATCH_CALL);
}