// ==========================================================
// alterOps project
// 
// Component: t5cli
// Sub-component: clientdll
// Purpose: Authorization to the aO web service.
//
// Initial author: NTAuthority
// Started: 2011-08-21 (finally...)
// ==========================================================

#undef UNICODE
#define NTDDI_VERSION 0x06000000
#include "StdInc.h"
#include <wincred.h>
#include <Shlwapi.h>
#include <shellapi.h>

#define CURL_STATICLIB
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

typedef DWORD (WINAPI * CredUIPromptForWindowsCredentialsW_t)(
								   __in_opt PCREDUI_INFOW pUiInfo,
								   __in DWORD dwAuthError,
								   __inout ULONG *pulAuthPackage,
								   __in_bcount_opt(ulInAuthBufferSize) LPCVOID pvInAuthBuffer,
								   __in ULONG ulInAuthBufferSize,
								   __deref_out_bcount_full(*pulOutAuthBufferSize) LPVOID * ppvOutAuthBuffer,
								   __out ULONG * pulOutAuthBufferSize,
								   __inout_opt BOOL *pfSave,
								   __in DWORD dwFlags
								   );

typedef BOOL (WINAPI * CredUnPackAuthenticationBufferW_t)(
								__in DWORD                                      dwFlags,
								__in_bcount(cbAuthBuffer) PVOID                 pAuthBuffer,
								__in DWORD                                      cbAuthBuffer,
								__out_ecount_opt(*pcchMaxUserName) LPWSTR       pszUserName,
								__inout DWORD*                                  pcchMaxUserName,
								__out_ecount_opt(*pcchMaxDomainName) LPWSTR     pszDomainName,
								__inout_opt DWORD*                              pcchMaxDomainName,
								__out_ecount_opt(*pcchMaxPassword) LPWSTR       pszPassword,
								__inout DWORD*                                  pcchMaxPassword
								);

bool Auth_DisplayLoginDialogDownLevel(const wchar_t** usernameP, const wchar_t** passwordP, PCREDUI_INFOW info)
{
	static WCHAR username[CREDUI_MAX_USERNAME_LENGTH * sizeof(WCHAR)] = { 0 };
	static WCHAR password[CREDUI_MAX_PASSWORD_LENGTH * sizeof(WCHAR)] = { 0 };
	BOOL save = FALSE;

	HRESULT result = CredUIPromptForCredentialsW(info, L"fourDeltaOne.net", NULL, NULL, username, CREDUI_MAX_USERNAME_LENGTH, password, CREDUI_MAX_PASSWORD_LENGTH, &save, CREDUI_FLAGS_GENERIC_CREDENTIALS | CREDUI_FLAGS_ALWAYS_SHOW_UI | CREDUI_FLAGS_DO_NOT_PERSIST | CREDUI_FLAGS_EXCLUDE_CERTIFICATES);

	if (result == NO_ERROR)
	{
		*usernameP = username;
		*passwordP = password;
	}

	return (result == NO_ERROR);
}

bool Auth_DisplayLoginDialog(const wchar_t** usernameP, const wchar_t** passwordP)
{
	static WCHAR username[CREDUI_MAX_USERNAME_LENGTH * sizeof(WCHAR)] = { 0 };
	static WCHAR password[CREDUI_MAX_PASSWORD_LENGTH * sizeof(WCHAR)] = { 0 };
	static bool doneFile = false;
	FILE* iw4mini;
	errno_t err;
	
	if (!doneFile && (err = fopen_s(&iw4mini, "iw4m.ini","r")) == 0){
		fgetws(username, CREDUI_MAX_USERNAME_LENGTH * sizeof(WCHAR), iw4mini);
		fgetws(password, CREDUI_MAX_PASSWORD_LENGTH * sizeof(WCHAR), iw4mini);
		for(int i = wcslen(username)-1; i >= 0; i--){
			if(isspace(username[i])) username[i] = 0; //stripping those nl chars
			else break;
		}
		for(int i = wcslen(password)-1; i >= 0; i--){
			if(isspace(password[i]) && password[i] != ' ') password[i] = 0; //spaces might be part of the password
			else break;
		}
		*usernameP = username;
		*passwordP = password;
		fclose(iw4mini);
		doneFile = true;
		return true;
	}
	doneFile = true; //prevent opening file twice on wrong info

	HMODULE credUI = LoadLibrary("credui.dll");

	if (!credUI)
	{
		// wtf
		__asm int 3
	}

	CredUIPromptForWindowsCredentialsW_t credUIPromptForWindowsCredentialsW = (CredUIPromptForWindowsCredentialsW_t)GetProcAddress(credUI, "CredUIPromptForWindowsCredentialsW");

	CREDUI_INFOW info;
	memset(&info, 0, sizeof(info));
	info.cbSize = sizeof(info);
	info.pszCaptionText = L"Authentication for IW4M";
	info.pszMessageText = L"Please log in to use IW4M. Use your fourDeltaOne.net forum credentials.";

	if (credUIPromptForWindowsCredentialsW == NULL)
	{
		return Auth_DisplayLoginDialogDownLevel(usernameP, passwordP, &info);
	}

	CredUnPackAuthenticationBufferW_t credUnPackAuthenticationBufferW = (CredUnPackAuthenticationBufferW_t)GetProcAddress(credUI, "CredUnPackAuthenticationBufferW");

	DWORD uLen = CREDUI_MAX_USERNAME_LENGTH;
	DWORD pLen = CREDUI_MAX_PASSWORD_LENGTH;
	LPVOID outStuff;
	ULONG outSize = 0;
	ULONG outPackage = 0;
	static BOOL save = false;
	//CredUIPromptForCredentials(&info, _T("Target"), NULL, NULL, username, CREDUI_MAX_USERNAME_LENGTH, password, CREDUI_MAX_PASSWORD_LENGTH, &save, CREDUI_FLAGS_GENERIC_CREDENTIALS | CREDUI_FLAGS_SHOW_SAVE_CHECK_BOX | CREDUI_FLAGS_ALWAYS_SHOW_UI | CREDUI_FLAGS_DO_NOT_PERSIST | CREDUI_FLAGS_EXCLUDE_CERTIFICATES);

	HRESULT result = credUIPromptForWindowsCredentialsW(&info, 0, &outPackage, NULL, 0, &outStuff, &outSize, &save, CREDUIWIN_GENERIC | CREDUIWIN_CHECKBOX);

	if (result == ERROR_SUCCESS)
	{
		credUnPackAuthenticationBufferW(0, outStuff, outSize, username, &uLen, NULL, 0, password, &pLen);

		*usernameP = username;
		*passwordP = password;

		if(save && (err = fopen_s(&iw4mini, "iw4m.ini","w")) == 0){
			//security is pointless; everyone has access to this code
			fputws(username, iw4mini);
			fputws(L"\n", iw4mini);
			fputws(password, iw4mini);
			fclose(iw4mini);
		}
	}

	return (result == ERROR_SUCCESS);
}

void Auth_Error(const char* message)
{
	//->int btn;
	static wchar_t buf[32768];
	MultiByteToWideChar(CP_UTF8, 0, message, -1, buf, sizeof(buf));

	MessageBoxW(NULL, buf, L"Oshibka", MB_OK | MB_ICONSTOP);
}

size_t AuthDataReceived(void *ptr, size_t size, size_t nmemb, void *data)
{
	if ((strlen((char*)data) + (size * nmemb)) > 8192)
	{
		return (size * nmemb);
	}

	strncat((char*)data, (const char*)ptr, size * nmemb);
	return (size * nmemb);
}

int authUserID;
char authUsername[256];
char authSessionID[40];

char* Auth_GetUsername()
{
	return authUsername;
}

char* Auth_GetSessionID()
{
	return authSessionID;
}

int Auth_GetUserID()
{
	return authUserID;
}

static int curlDebug(CURL *, curl_infotype type, char * data, size_t size, void *)
{
	if (type == CURLINFO_TEXT)
	{
		char text[8192];
		memcpy(text, data, size);
		text[size] = '\0';

		OutputDebugString(va("%s\n", text));
	}

	return 0;
}

bool Auth_ParseResultBuffer(const char* result)
{
	bool ok = false;
	static char buf[16384];
	strncpy(buf, result, sizeof(buf));

	int i = 0;
	char* tokens[16];
	char* tok = strtok(buf, "#");

	while (tok && (i < 16))
	{
		tokens[i++] = tok;
		tok = strtok(NULL, "#");
	}

	// ok
	if (tokens[0][0] == 'o')
	{
		ok = true;
	}

	if (!ok)
	{
		Auth_Error(tokens[1]);
	}
	else
	{
		authUserID = atoi(tokens[2]);
		strcpy(authSessionID, tokens[5]);
		strncpy(authUsername, tokens[3], sizeof(authUsername));
	}

	return ok;
}

bool Auth_PerformSessionLogin(const char* username, const char* password)
{
	curl_global_init(CURL_GLOBAL_ALL);

	CURL* curl = curl_easy_init();

	if (curl)
	{
		char url[255];
		_snprintf(url, sizeof(url), "http://%s/remauth.php", "auth.iw4.prod.fourdeltaone.net:1337");

		char buf[8192] = {0};
		char postBuf[8192];
		_snprintf(postBuf, sizeof(postBuf), "%s&&%s", username, password);

		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, AuthDataReceived);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&buf);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "IW4M");
		curl_easy_setopt(curl, CURLOPT_FAILONERROR, true);
		curl_easy_setopt(curl, CURLOPT_POST, 1);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postBuf);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, -1);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, curlDebug);

		CURLcode code = curl_easy_perform(curl);
		curl_easy_cleanup(curl);

		curl_global_cleanup();

		if (code == CURLE_OK)
		{
			return Auth_ParseResultBuffer(buf);
		}
		else
		{
			Auth_Error(va("Could not reach the fourDeltaOne.net server. Error code from CURL: %x.", code));
		}

		return false;
	}

	curl_global_cleanup();
	return false;
}

#pragma optimize("", off)
void Auth_VerifyIdentity()
{
	const wchar_t* wusername;
	const wchar_t* wpassword;
	bool canceled = false;

#if !PRE_RELEASE_DEMO
	goto skipLogin;
#endif
	
	while (!canceled)
	{
		canceled = !Auth_DisplayLoginDialog(&wusername, &wpassword);

		if (!canceled)
		{
			char username[CREDUI_MAX_USERNAME_LENGTH];
			char password[CREDUI_MAX_PASSWORD_LENGTH];
			WideCharToMultiByte(CP_UTF8, 0, wusername, -1, username, sizeof(username), NULL, NULL);
			WideCharToMultiByte(CP_UTF8, 0, wpassword, -1, password, sizeof(password), NULL, NULL);

			bool result = Auth_PerformSessionLogin(username, password);
			
			if (result)
			{
				return;
			}
		}
	}

#if !PRE_RELEASE_DEMO
skipLogin:
	bool result = Auth_PerformSessionLogin("xnp", "xnpxnp");

	if (result)
	{
		return;
	}
#endif

	ExitProcess(0x8000D3AD);
}
#pragma optimize("", on)

size_t DLCDataReceived(void *ptr, size_t size, size_t nmemb, void *data)
{
	size_t rsize = (size * nmemb);
	char* text = (char*)ptr;

	ShellExecute(NULL, "open", text, 0, 0, SW_SHOWNORMAL);

	return rsize;
}

void Auth_OpenDLCStore()
{
	curl_global_init(CURL_GLOBAL_ALL);

	CURL* curl = curl_easy_init();

	if (curl)
	{
		char url[255];
		_snprintf(url, sizeof(url), "http://iw4.prod.fourdeltaone.net/content/dlc.txt");

		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, DLCDataReceived);
		//curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&steamID);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "IW5M");
		curl_easy_setopt(curl, CURLOPT_FAILONERROR, true);

		CURLcode code = curl_easy_perform(curl);
		curl_easy_cleanup(curl);

		curl_global_cleanup();

		if (code == CURLE_OK)
		{
			return;
		}
		else
		{
			// TODO: set some offline mode flag
		}
	}

	curl_global_cleanup();
}