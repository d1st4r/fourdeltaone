// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Various generic utility functions.
//
// Initial author: NTAuthority
// Started: 2010-09-10
// ==========================================================

#include "StdInc.h"
#include <ShellAPI.h>
#include <sys/stat.h>
#include <direct.h>
#include <io.h>

/*
void Trace(char* source, char* message, ...)
{
	va_list args;
	char buffer[1024];
	char buffer2[1024];

	va_start(args, message);
	_vsnprintf(buffer, sizeof(buffer), message, args);
	va_end(args);

	_snprintf(buffer2, sizeof(buffer2), "[%s] %s", source, buffer);

	OutputDebugStringA(buffer2);
}

void Trace2(char* message, ...)
{
	va_list args;
	char buffer[1024];

	va_start(args, message);
	_vsnprintf(buffer, sizeof(buffer), message, args);
	va_end(args);

	OutputDebugStringA(buffer);
}
*/

void SearchAndPatch(int patchArray[], int PatchCount, DWORD oldBase, DWORD newBase, DWORD searchEnd)
{
	for(int i = 0;i<PatchCount;i++)
	{
		PBYTE pattern = (PBYTE)&(patchArray[i]);
		DWORD Address = CODE_START;
		int patchTo = patchArray[i] - oldBase + (int)newBase;
		while(Address < searchEnd)
		{
			DWORD ret = FindPattern(Address, (searchEnd - Address), pattern, "xxxx");
			if(ret!=0)
			{
				*(int*)ret = patchTo;
				Address = ret + 1;
			}
			else
			{
				break;
			}
		}
	}
}

//Thanks to Nukem
bool Compare(const BYTE* pData, const BYTE* bMask, const char* szMask)
{
	for(; *szMask; ++szMask, ++pData, ++bMask)
		if(*szMask == 'x' && *pData != *bMask)
			return 0;

	return (*szMask) == NULL;
}

//Thanks to Nukem
DWORD FindPattern(DWORD dwAddress, DWORD dwLen, BYTE *bMask, char *szMask)
{
	dwLen += dwAddress;

	for(DWORD i = dwAddress; i < dwLen; i++)
		if(Compare((BYTE *)i, bMask, szMask))
		{
			return i;
		}

		return 0;
}

#undef malloc
void* custom_malloc(size_t size, char* file, int line)
{
	char buffer[1024];
	_snprintf(buffer, sizeof(buffer), "[debug] %s:%d alloc %d\n", file, line, size);

	OutputDebugStringA(buffer);

	return malloc(size);
}

bool FileExists(const char* file)
{
	struct stat st;

	// note that this doesn't count any of the other ways stat could fail, but that'd be more error checking elsewhere
	if (stat(file, &st) >= 0)
	{
		return true;
	}

	return false;
}

size_t FileSize(const char* file)
{
	struct stat st;

	if (stat(file, &st) >= 0)
	{
		return st.st_size;
	}

	return 0;
}

// a funny thing is how this va() function could possibly come from leaked IW code.
#define VA_BUFFER_COUNT		4
#define VA_BUFFER_SIZE		32768

static char g_vaBuffer[VA_BUFFER_COUNT][VA_BUFFER_SIZE];
static int g_vaNextBufferIndex = 0;

const char *va( const char *fmt, ... )
{
	va_list ap;
	char *dest = &g_vaBuffer[g_vaNextBufferIndex][0];
	g_vaNextBufferIndex = (g_vaNextBufferIndex + 1) % VA_BUFFER_COUNT;
	va_start(ap, fmt);
	int res = _vsnprintf( dest, VA_BUFFER_SIZE, fmt, ap );
	dest[VA_BUFFER_SIZE - 1] = '\0';
	va_end(ap);

	if (res < 0 || res >= VA_BUFFER_SIZE)
	{
		Com_Error(1, "Attempted to overrun string in call to va() - return address 0x%x", _ReturnAddress());
	}

	return dest;
}

size_t Com_AddToString(const char* string, char* buffer, size_t current, size_t length, bool escapeSpaces)
{
	const char* i = string;
	size_t margin = (escapeSpaces) ? 2 : 0;
	bool hadSpaces = false;

	if (length - current <= 0)
	{
		return current;
	}

	if (escapeSpaces)
	{
		if ((length - current) > margin)
		{
			while (*i != 0)
			{
				if (*i == ' ')
				{
					buffer[current++] = '"';
					hadSpaces = true;
					break;
				}

				i++;
			}
		}
	}

	i = string;
	while (*i != '\0')
	{
		if (length - current > margin)
		{
			buffer[current++] = *i;
		}

		i++;
	}

	if (hadSpaces)
	{
		buffer[current++] = '"';
	}

	return current;
}

/*
===================
Info_RemoveKey
===================
*/
void Info_RemoveKey( char *s, const char *key ) {
	char	*start;
	char	pkey[MAX_INFO_KEY];
	char	value[MAX_INFO_VALUE];
	char	*o;

	if (strchr (key, '\\')) {
		return;
	}

	while (1)
	{
		start = s;
		if (*s == '\\')
			s++;
		o = pkey;
		while (*s != '\\')
		{
			if (!*s)
				return;
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value;
		while (*s != '\\' && *s)
		{
			if (!*s)
				return;
			*o++ = *s++;
		}
		*o = 0;

		if (!strcmp (key, pkey) )
		{
			strcpy (start, s);	// remove this part
			return;
		}

		if (!*s)
			return;
	}

}

/*
==================
Info_SetValueForKey

Changes or adds a key/value pair
==================
*/
void Info_SetValueForKey( char *s, const char *key, const char *value ) {
        char    newi[MAX_INFO_STRING];
        const char* blacklist = "\\;\"";

        if ( strlen( s ) >= MAX_INFO_STRING ) {
                Com_Printf( 0, "Info_SetValueForKey: oversize infostring");
		return;
        }

        for(; *blacklist; ++blacklist)
        {
                if (strchr (key, *blacklist) || strchr (value, *blacklist))
                {
                        Com_Printf (0, "Can't use keys or values with a '%c': %s = %s\n", *blacklist, key, value);
                        return;
                }
        }

        Info_RemoveKey (s, key);
        if (!value || !strlen(value))
                return;

        _snprintf (newi, sizeof(newi), "\\%s\\%s", key, value);

        if (strlen(newi) + strlen(s) >= MAX_INFO_STRING)
        {
                Com_Printf (0, "Info string length exceeded\n");
                return;
        }

        strcat (newi, s);
        strcpy (s, newi);
}


#define	BIG_INFO_STRING		8192  // used for system info key only
#define	BIG_INFO_KEY		  8192
#define	BIG_INFO_VALUE		8192

/*
===============
Info_ValueForKey

Searches the string for the given
key and returns the associated value, or an empty string.
FIXME: overflow check?
===============
*/
char *Info_ValueForKey( const char *s, const char *key )
{
	char	pkey[BIG_INFO_KEY];
	static	char value[2][BIG_INFO_VALUE];	// use two buffers so compares
	// work without stomping on each other
	static	int	valueindex = 0;
	char	*o;

	if ( !s || !key ) {
		return "";
	}

	if ( strlen( s ) >= BIG_INFO_STRING ) {
		//Com_Error( ERR_DROP, "Info_ValueForKey: oversize infostring" );
		return "";
	}

	valueindex ^= 1;
	if (*s == '\\')
		s++;
	while (1)
	{
		o = pkey;
		while (*s != '\\')
		{
			if (!*s)
				return "";
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value[valueindex];

		while (*s != '\\' && *s)
		{
			*o++ = *s++;
		}
		*o = 0;

		if (!_stricmp (key, pkey) )
			return value[valueindex];

		if (!*s)
			break;
		s++;
	}

	return "";
}

// determine which patchset to use
unsigned int _gameFlags;

typedef struct  
{
	const wchar_t* argument;
	unsigned int flag;
} flagDef_t;

flagDef_t flags[] =
{
	{ L"dedicated", GAME_FLAG_DEDICATED },
	{ L"console", GAME_FLAG_CONSOLE },
	{ L"dump", GAME_FLAG_DUMPDATA },
	{ L"entries", GAME_FLAG_ENTRIES },
	{ L"gscfilesystem", GAME_FLAG_GSCFILESYSTEM },
	{ L"iamaterriblefaggotwhodoesnotdeservetolive", GAME_FLAG_D3D9 },
	{ 0, 0 }
};

bool hasLicenseKey = false;
char licenseKey[48];

const char* GetLicenseKey()
{
	return (hasLicenseKey) ? &licenseKey[1] : NULL;
}

void DetermineGameFlags()
{
	int numArgs;
	LPCWSTR commandLine = GetCommandLineW();
	LPWSTR* argv = CommandLineToArgvW(commandLine, &numArgs);

	for (int i = 0; i < numArgs; i++)
	{
		if (argv[i][0] == L'#' || argv[i][0] == L'@')
		{
			WideCharToMultiByte(CP_ACP, 0, argv[i], -1, licenseKey, sizeof(licenseKey), "?", NULL);

			hasLicenseKey = true;
		}

		if (argv[i][0] != L'-') continue;

		for (wchar_t* c = argv[i]; *c != L'\0'; c++)
		{
			if (*c != L'-')
			{
				for (flagDef_t* flag = flags; flag->argument; flag++)
				{
					if (!wcscmp(c, flag->argument))
					{
						_gameFlags |= flag->flag;
						break;
					}
				}
				break;
			}
		}
	}

	LocalFree(argv);
}

// You must free the result if result is non-NULL.
char *str_replace(char *orig, char *rep, char *with)
{
	char *result; // the return string
	char *ins;    // the next insert point
	char *tmp;    // varies
	int len_rep;  // length of rep
	int len_with; // length of with
	int len_front; // distance between rep and end of last rep
	int count;    // number of replacements

	if (!orig)
		return NULL;
	if (!rep || !(len_rep = strlen(rep)))
		return NULL;
	if (!(ins = strstr(orig, rep))) 
		return NULL;
	if (!with)
		with = "";
	len_with = strlen(with);

	for (count = 0; tmp = strstr(ins, rep); ++count) {
		ins = tmp + len_rep;
	}

	// first time through the loop, all the variable are set correctly
	// from here on,
	//    tmp points to the end of the result string
	//    ins points to the next occurrence of rep in orig
	//    orig points to the remainder of orig after "end of rep"
	tmp = result = (char*)malloc(strlen(orig) + (len_with - len_rep) * count + 1);

	if (!result)
		return NULL;

	while (count--) {
		ins = strstr(orig, rep);
		len_front = ins - orig;
		tmp = strncpy(tmp, orig, len_front) + len_front;
		tmp = strcpy(tmp, with) + len_with;
		orig += len_front + len_rep; // move to next "end of rep"
	}
	strcpy(tmp, orig);
	return result;
}

void CreateDirectoryAnyDepth(const char *path)
{
	char opath[MAX_PATH]; 
	char *p; 
	size_t len; 
	strncpy_s(opath, path, sizeof(opath)); 
	len = strlen(opath); 
	if(opath[len - 1] == L'/') 
		opath[len - 1] = L'\0'; 

	for(p = opath; *p; p++) 
	{
		if(*p == L'/' || *p == L'\\') 
		{ 
			*p = L'\0'; 
			if(_access(opath, 0)) 
				_mkdir(opath);
			*p = L'\\';
		}
	}
	if(_access(opath, 0))
		_mkdir(opath);
}

void ProcessSignature(signature_t* signature)
{
	// part of code copied from somewhere
	unsigned int size = 0x2D6000;

	unsigned char *pBasePtr = (unsigned char *)0x401000;
	unsigned char *pEndPtr = pBasePtr + size;
	size_t i;
	int occurred = 0;

	while(pBasePtr < pEndPtr) {
		for(i = 0;i < signature->size;i++) {
			if((signature->mask[i] != '?') && (signature->signature[i] != pBasePtr[i]))
				break;
		}

		if(i == signature->size)
		{
			DWORD addy = ((DWORD)pBasePtr) + signature->logOffset;
			//printf(" | grep -v %x", addy);

			occurred++;
			OutputDebugString(va("0x%x\n", addy));

			// patch output
			if (signature->replaceCB)
			{
				signature->replaceCB(pBasePtr + signature->replaceOffset);
			}

			if (signature->replace)
			{
				memcpy(pBasePtr + signature->replaceOffset, signature->replace, signature->replaceSize);
			}
		}

		pBasePtr++;
	}

	OutputDebugString(va("signature occurred %d times\n", occurred));
}

void InitProfile()
{
	return;
	FILE* f = fopen("profile.txt", "w");
	fclose(f);
}

void WriteProfile(int time, const char* type, const char* message)
{
	return;
	FILE* f = fopen("profile.txt", "a");
	if (f)
	{
		fprintf(f, "[%i] %s - %s\n", time, type, message);
		fclose(f);
	}
}

bool startsWith(const char *str, const char *pre)
{
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}