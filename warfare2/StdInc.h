// ==========================================================
// Warfare2 project
// 
// Component: warfare2
// Purpose: Common include file for W2.
//
// Initial author: NTAuthority
// Started: 2013-07-09
// ==========================================================

#pragma once

#define WARFARE2_EXE_VERSION 9

#define _USING_V110_SDK71_

#include <Windows.h>

#include <stdio.h>
#include <string.h>
#include <wchar.h>

#include <stdint.h>

void W2_StartGame();

const char *va( const char *fmt, ... );
const wchar_t *va( const wchar_t *fmt, ... );
void CreateDirectoryAnyDepth(const char *path);
int DL_RequestURL(const char* url, char* buffer, size_t bufSize);

// bootstrapper functions
// move the bootstrapper files if called by the initializer
bool Bootstrap_RunInit();

// run the bootstrapper/updater functions
bool Bootstrap_DoBootstrap();

// downloader functions
void CL_InitDownloadQueue();
void CL_QueueDownload(const char* url, const char* file, int64_t size, bool compressed);
void CL_QueueDownload(const char* url, const char* file, int64_t size, bool compressed, int segments);
//void CL_QueueDownload(const char* url, const char* file, int size, bool compressed, const uint8_t* hash, uint32_t hashLen);
bool DL_Process();

bool DL_RunLoop();

// UI functions
void UI_DoCreation();
void UI_DoDestruction();
void UI_UpdateText(int textControl, const wchar_t* text);
void UI_UpdateProgress(double percentage);
bool UI_IsCanceled();

// updater functions
bool Updater_RunUpdate(int numCaches, ...);
bool Installer_GetIW4C();