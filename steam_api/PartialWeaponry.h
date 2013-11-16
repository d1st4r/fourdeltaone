// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Support for 'partial weaponry' - header file
//
// Initial author: NTAuthority
// Started: 2013-07-08
// ==========================================================

#pragma once

enum AttachmentPatchOperation
{
	APO_ADD,
	APO_REPLACE,
	APO_MULTIPLY
};

enum AttachmentPatchType
{
	APT_STRING,
	APT_INTEGER,
	APT_FLOAT
};

struct AttachmentPatch
{
	const char* fieldName;

	AttachmentPatchOperation operation;
	AttachmentPatchType type;

	union
	{
		const char* string;
		int integer;
		float value;
	};
};

struct AttachmentDef
{
	const char* name;

	int numPatches;
	AttachmentPatch* patches;
};