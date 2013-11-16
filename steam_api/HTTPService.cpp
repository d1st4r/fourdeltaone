// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: HTTP service handler.
//
// Initial author: NTAuthority
// Started: 2011-06-28
// ==========================================================

#include "StdInc.h"
#include "ExtScript.h"
#include <mongoose.h>

// variable definitions
mg_context* g_mg;
dvar_t** net_port = (dvar_t**)0x64A3004;

// code implementations
void HTTPService_Forbidden(mg_connection* conn)
{
	mg_printf(conn, "HTTP/1.1 403 Forbidden\r\n"
					"Content-Type: text/html\r\n\r\n"
					"<h1>403 - Forbidden</h1>");
}

void HTTPService_NotFound(mg_connection* conn)
{
	mg_printf(conn, "HTTP/1.1 404 Not Found\r\n"
					"Content-Type: text/html\r\n\r\n"
					"<h1>404 - Not Found</h1>");
}

bool FS_GetFileForDownload(const char* path, char* buffer, size_t bufferSize);

void HTTPService_HandleDownloadRequest(mg_connection* conn, const mg_request_info* request_info)
{
	char filename[MAX_PATH];
	const char* requestedFile = &request_info->uri[4];
	
	if (!SV_IsClientIP(request_info->remote_ip))
	{
		HTTPService_Forbidden(conn);
		return;
	}

	if (!FS_GetFileForDownload(requestedFile, filename, sizeof(filename)))
	{
		HTTPService_NotFound(conn);
		return;
	}

	mg_send_file(conn, filename);
}

void* HTTPService_HandleEvent(mg_event event, mg_connection* conn, const mg_request_info* request_info)
{
	if (event == MG_NEW_REQUEST)
	{
		if (memcmp(request_info->uri, "/dl/", 4) == 0)
		{
			HTTPService_HandleDownloadRequest(conn, request_info);
		}
		else
		{
			if (Ext_Initialized())
			{
				Ext_HandleWebRequest(conn, request_info);
			}
			else if (g_extDLL->ScriptabilityIsWeb())
			{
				g_extDLL->HandleWebRequest(conn, request_info);
			}
			else
			{
				HTTPService_NotFound(conn);
			}
		}

		return "";
	}
	else if (event == MG_EVENT_LOG)
	{
		Com_Printf(0, "HTTP: %s\n", request_info->log_message);
		return "";
	}

	return NULL;
}

void HTTPService_Init()
{
	const char* options[] =
	{
		"listening_ports", va("%i", (*net_port)->current.integer),
		"num_threads", "20", // TODO: switch HTTP server library to one which doesn't use worker threads
		NULL
	};

	g_mg = mg_start(HTTPService_HandleEvent, NULL, options);
	if (!g_mg)
	{
		Com_Printf(0, "Couldn't start up HTTP service\n");
	}
	else
	{
		Com_Printf(0, "HTTP service listening on port %i\n", (*net_port)->current.integer);
	}
}

void HTTPService_Shutdown()
{
	mg_stop(g_mg);
}