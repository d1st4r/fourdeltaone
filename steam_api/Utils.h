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

//void Trace(char* source, char* message, ...);
//void Trace2(char* message, ...);
#define Trace(source, message, ...) Trace2("[" source "] " message, __VA_ARGS__)
#define Trace2(message, ...) Com_Printf(0, message, __VA_ARGS__)

void InitProfile();
void WriteProfile(int time, const char* type, const char* message);

bool FileExists(const char* file);
size_t FileSize(const char* file);
char *str_replace(char *orig, char *rep, char *with);
void CreateDirectoryAnyDepth(const char *path);

const char* va(const char* format, ...);

size_t Com_AddToString(const char* source, char* buffer, size_t current, size_t length, bool escapeSpaces);

#define	MAX_INFO_STRING		1024
#define	MAX_INFO_KEY		  1024
#define	MAX_INFO_VALUE		1024

void Info_RemoveKey( char *s, const char *key );
void Info_SetValueForKey( char *s, const char *key, const char *value );
char *Info_ValueForKey( const char *s, const char *key );

// flag settings
#define GAME_FLAG_DEDICATED		(1 << 0)
#define GAME_FLAG_CONSOLE		(1 << 1)
#define GAME_FLAG_DUMPDATA		(1 << 2)
#define GAME_FLAG_ENTRIES		(1 << 3)
#define GAME_FLAG_GSCFILESYSTEM	(1 << 4)
#define GAME_FLAG_D3D9			(1 << 5)

//#ifndef PRE_RELEASE_DEMO
#define GAME_FLAG(x)			((_gameFlags & x) == x)
//#else
//#define GAME_FLAG(x)			false
//#endif

extern unsigned int _gameFlags;
void DetermineGameFlags();
const char* GetLicenseKey();

#define MAX_SIGNATURES 256
typedef struct signature_s
{
	unsigned char* signature;
	char* mask;
	size_t size;
	unsigned char* replace;
	int replaceOffset;
	size_t replaceSize;
	void (*replaceCB)(void* address);
	int logOffset;
} signature_t;

void ProcessSignature(signature_t* signature);

bool Compare(const BYTE* pData, const BYTE* bMask, const char* szMask);
void SearchAndPatch(int patchArray[], int PatchCount, DWORD oldBase, DWORD newBase, DWORD searchEnd = 0x6A0000);
DWORD FindPattern(DWORD dwAddress, DWORD dwLen, BYTE *bMask, char *szMask);
bool startsWith(const char *str, const char *pre);