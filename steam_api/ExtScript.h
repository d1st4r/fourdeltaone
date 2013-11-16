// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Game extensibility code.
//
// Initial author: NTAuthority
// Started: 2012-04-21
// ==========================================================

#pragma once
#include <mongoose.h>

void Ext_Initialize();
bool Ext_Initialized();
void Ext_HandleWebRequest(mg_connection* conn, const mg_request_info* request_info);
void ImageExport_HandleWebRequest(mg_connection* conn, const mg_request_info* request_info);