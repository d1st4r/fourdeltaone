#include "StdInc.h"

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
		//return "";
	}

	return dest;
}

static wchar_t g_vaBufferW[VA_BUFFER_COUNT][VA_BUFFER_SIZE];
static int g_vaNextBufferWIndex = 0;

const wchar_t *va( const wchar_t *fmt, ... )
{
	va_list ap;
	wchar_t *dest = &g_vaBufferW[g_vaNextBufferWIndex][0];
	g_vaNextBufferWIndex = (g_vaNextBufferWIndex + 1) % VA_BUFFER_COUNT;
	va_start(ap, fmt);
	int res = _vsnwprintf( dest, VA_BUFFER_SIZE, fmt, ap );
	dest[VA_BUFFER_SIZE - 1] = '\0';
	va_end(ap);

	if (res < 0 || res >= VA_BUFFER_SIZE)
	{
		//return "";
	}

	return dest;
}

#include <direct.h>
#include <io.h>

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