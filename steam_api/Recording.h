// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: extdll
// Purpose: Server-side demo recording.
//
// Initial author: NTAuthority
// Started: 2013-02-24
// ==========================================================

#pragma once

// you may notice how ClientArchive is missing from this list
// this is the 'predicted origin'; which is used to 'smooth out' the change from cgame's predicted values
// to the server's consistent values - disabling the function getting them causes lots of stutter
// unless enabling cg_nopredict as well, which is what we should do when a demo is playing (usercmds don't make
// any sense during playback anyway)
enum DemoMessageType : unsigned char
{
	DMT_HEADER,
	DMT_BOOKMARK,
	DMT_SNAPSHOT,
	
};

struct DemoMessageHeader
{
	DemoMessageType type;
	size_t length;
	size_t packedLength;
};