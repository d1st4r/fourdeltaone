// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Loading and replacing of string tables (.csv
//          assets)
//
// Initial author: NTAuthority
// Started: 2011-11-06
// ==========================================================

#include "StdInc.h"

#include <google/dense_hash_map>
#include <string>
#include <xhash>

// we don't like Boost
#define strtk_no_tr1_or_boost

// conflicting windows.h definitions
#undef max
#undef min

// strtk code
#include <strtk.hpp>

// code
typedef struct stringTable_s {
	char* fileName;
	int columns;
	int rows;
	char** data;
} stringTable_t;

int StringTableHash(char* data)
{
	int hash = 0;

	while (*data != 0)
	{
		hash = tolower(*data) + (31 * hash);

		data++;
	}

	return hash;
}

stringTable_t* StringTable_Load_LoadObj(const char* filename, const char* buffer)
{
	stringTable_t* origTable = (stringTable_t*)DB_FindXAssetHeader(ASSET_TYPE_STRINGTABLE, filename);

	strtk::token_grid::options options;
	options.set_row_delimiters("\r\n");
	options.set_column_delimiters(",");
	options.set_column_split_option(strtk::split_options::default_mode);
	options.set_row_split_option(strtk::split_options::default_mode);

	strtk::token_grid grid(buffer, strlen(buffer), options);
	int columns = grid.max_column_count();
	int rows = grid.row_count();

	stringTable_t* newTable = (stringTable_t*)malloc(sizeof(stringTable_t));
	newTable->fileName = (char*)filename;
	newTable->data = (char**)malloc(sizeof(char*) * rows * columns * 2);

	for (std::size_t ri = 0; ri < (std::size_t)rows; ri++)
	{
		strtk::token_grid::row_type r = grid.row(ri);

		for (std::size_t ci = 0; ci < r.size(); ci++)
		{
			std::string strSource = r.get<std::string>(ci);
			char* str = (char*)malloc(strSource.length() + 1);
			memcpy(str, strSource.c_str(), strSource.length() + 1);

			newTable->data[((ri * columns) + ci) * 2] = str;
			newTable->data[(((ri * columns) + ci) * 2) + 1] = (char*)StringTableHash(str);
		}
	}

	newTable->columns = columns;
	newTable->rows = rows;

	return newTable;
}

stringTable_t* StringTable_Load(const char* filename)
{
	char* buffer;
	char newfilename[255];

	strcpy(newfilename, filename);
	newfilename[0] = 'a';

	if (FS_ReadFile(newfilename, (char**)&buffer) > 0)
	{
		stringTable_t* table = StringTable_Load_LoadObj(filename, buffer);
		FS_FreeFile(buffer);

		return table;
	}

	return (stringTable_t*)1;
}

static google::dense_hash_map<std::string, stringTable_t*, stdext::hash_compare<std::string>> stringTables;

void StringTable_GetAsset(char* filename, stringTable_t** table)
{
	if (!stringTables[filename])
	{
		stringTables[filename] = StringTable_Load(filename);
	}

	// 1 = get from XAsset DB
	if ((int)stringTables[filename] == 1)
	{
		*table = (stringTable_t*)DB_FindXAssetHeader(ASSET_TYPE_STRINGTABLE, filename);
	}
	else
	{
		*table = stringTables[filename];
	}
}

StompHook stringTableHook;
DWORD stringTableHookLoc = 0x49BB60;

// entry point
void PatchMW2_StringTable()
{
	stringTables.set_empty_key("");

	stringTableHook.initialize(stringTableHookLoc, StringTable_GetAsset);
	stringTableHook.installHook();
}