// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: internal usage: model dumping :))
//
// Initial author: NTAuthority
// Started: 2012-06-26
// ==========================================================

#include "StdInc.h"

#include "s10e5.h"
//#if !D3D_EXPERIMENTS
//class IDirect3DVertexBuffer9;
//class IDirect3DIndexBuffer9;
//#else
#include <D3D9.h>
//#endif

// badly converted from vs_3_0 assembly
void __declspec(naked) TransformTexCoord(DWORD in, float* out)
{
/*	float r0[4];
	// mul r0, c10.xxyy, v2.zxzx
	r0[0] = 0.0009765625f * in[2]; // / 1024
	r0[1] = 0.0009765625f * in[0]; // / 1024
	r0[2] = 0.0000305175781f * in[2]; // / 32768
	r0[3] = 0.0000305175781f * in[0]; // / 32768

	// mad r0, v2.wywy, c10.zzww, r0
	r0[0] = (in[3] * 0.25f) + r0[0];
	r0[1] = (in[1] * 0.25f) + r0[1];
	r0[2] = (in[3] * 0.0078125f) + r0[2];
	r0[3] = (in[1] * 0.0078125f) + r0[3];

	// frc r1, r0
	float r1[4];
	r1[0] = r0[0] - floor(r0[0]);
	r1[1] = r0[1] - floor(r0[1]);
	r1[2] = r0[2] - floor(r0[2]);
	r1[3] = r0[3] - floor(r0[3]);

	// add r0.zw, r0, -r1
	r0[2] = r0[2] + -r1[2];
	r0[3] = r0[3] + -r1[3];

	// mad r0.xy, r1, -c11.x, r1.zwzw
	r0[0] = (r1[0] * -0.03125f) + r1[2];
	r0[1] = (r1[1] * -0.03125f) + r1[3];

	// mad r0, r0, c12.xxyy, c12.zzww
	r0[0] = (r0[0] * 32.0f) + -15.0f;
	r0[1] = (r0[1] * 32.0f) + -15.0f;
	r0[2] = (r0[2] * -2.0f) + 1.0f;
	r0[3] = (r0[3] * -2.0f) + 1.0f;

	// mad r0.zw, r0, r1.xyxy, r0
	r0[2] = (r0[2] * r1[0]) + r0[2];
	r0[3] = (r0[3] * r1[1]) + r0[3];

	// exp r1.x, r0.x
	r1[0] = (float)pow(2, r0[0]);

	// exp r1.y, r0.y
	r1[1] = (float)pow(2, r0[1]);

	// mul o2.xy, r0.zwzw, r1
	out[0] = r0[2] * r1[0];
	out[1] = r0[3] * r1[1];*/

	__asm
	{
		push	ebp
		mov		ebp, esp
		push	esi
		push	ebx
		sub     esp, 10h
		mov     ebx, [ebp+8]
		mov     esi, [ebp+0Ch]
		mov     ecx, ebx
		shr     ecx, 10h
		jnz     doSecond

		xor     eax, eax
		mov     [esi], eax
		add     esi, 4
		mov     ecx, ebx
		and     ecx, 0FFFFh

		jz doReturn

doFirst:
		mov     edx, ecx
		shl     edx, 0Eh
		shl     ecx, 10h
		and     ecx, 80000000h
		mov     eax, edx
		and     eax, 0FFFC000h
		not     edx
		and     edx, 10000000h
		sub     eax, edx
		sub     eax, 80000000h
		shr     eax, 1
		or      ecx, eax
		mov     [ebp-0Ch], ecx
		mov     eax, [ebp-0Ch]
		mov     [esi], eax
		add     esp, 10h
		pop     ebx
		pop     esi
		pop     ebp
		retn

doSecond:
		mov     edx, ecx
		shl     edx, 0Eh
		shl     ecx, 10h
		and     ecx, 80000000h
		mov     eax, edx
		and     eax, 0FFFC000h
		not     edx
		and     edx, 10000000h
		sub     eax, edx
		sub     eax, 80000000h
		shr     eax, 1
		or      ecx, eax
		mov     [ebp-0Ch], ecx
		mov     eax, [ebp-0Ch]
		mov     [esi], eax
		add     esi, 4
		mov     ecx, ebx
		and     ecx, 0FFFFh
		jnz		doFirst


doReturn:
		xor     eax, eax
		mov     [esi], eax
		add     esp, 10h
		pop     ebx
		pop     esi
		pop     ebp
		retn
	}
}

typedef void (__cdecl * DB_GetVertexBufferAndOffset_t)(unsigned char, void*, IDirect3DVertexBuffer9** buffer, DWORD* base);
DB_GetVertexBufferAndOffset_t DB_GetVertexBufferAndOffset = (DB_GetVertexBufferAndOffset_t)0x5BC050;

typedef void (__cdecl * DB_GetIndexBufferAndBase_t)(unsigned char, void*, IDirect3DIndexBuffer9** buffer, DWORD* base);
DB_GetIndexBufferAndBase_t DB_GetIndexBufferAndBase = (DB_GetIndexBufferAndBase_t)0x4B4DE0;

void DumpXModel_f()
{
	if (Cmd_Argc() != 3)
	{
		Com_Printf(0, "usage: dumpXModel [xmodel name] [xmodelsurfs name]\n");
		return;
	}

	XModel* xmodel = (XModel*)DB_FindXAssetHeader(ASSET_TYPE_XMODEL, Cmd_Argv(1));
	XModelSurfs* surfaces = (XModelSurfs*)DB_FindXAssetHeader(ASSET_TYPE_XMODELSURFS, Cmd_Argv(2));

	if (!surfaces)
	{
		Com_Printf(0, "no such xmodelsurfs asset\n");
		return;
	}

	for (int i = 0; i < surfaces->numSurfaces; i++)
	{
		//Com_Printf(0, "surface %i:\n  %i vertices\n  %i triangles\n\n", i, surfaces->surfaces[i].numVertices, surfaces->surfaces[i].numPrimitives);

//#if D3D_EXPERIMENTS
		XSurface* surface = &surfaces->surfaces[i];

		IDirect3DVertexBuffer9* vertexBuffer;
		DWORD vertexBase;

		IDirect3DIndexBuffer9* indexBuffer;
		DWORD indexBase;

		DB_GetVertexBufferAndOffset(surface->streamHandle, surface->vertexBuffer, &vertexBuffer, &vertexBase);
		DB_GetIndexBufferAndBase(surface->streamHandle, surface->indexBuffer, &indexBuffer, &indexBase);

		unsigned short* indexData;
		char* vertexData;
		//indexBuffer->Lock(indexBase * 2, surface->numPrimitives * 2 * 3, (void**)&indexData, D3DLOCK_READONLY);
		indexData = (unsigned short*)surface->indexBuffer;
		vertexData = (char*)surface->vertexBuffer;

		Material* material = xmodel->materials[i];
		int oddNum = *((char*)material + 36);
		MaterialTechniqueSet* techSet = material->techniqueSet;

		if (!techSet->techniques[9])
		{
			continue;
		}

		/*ID3DXBuffer* buffer;
		D3DXDisassembleShader(techSet->techniques[9]->passes[0].vertexShader->bytecode, TRUE, NULL, &buffer);

		FILE* shaderDisassembly = fopen(va("raw/%s.html", techSet->techniques[9]->passes[0].vertexShader->name), "w");
		fwrite(buffer->GetBufferPointer(), 1, buffer->GetBufferSize(), shaderDisassembly);
		fclose(shaderDisassembly);*/

		D3DVERTEXELEMENT9 decl[MAXD3DDECLLENGTH];
		UINT numElements;

		VertexDecl* vertexDecl = techSet->techniques[9]->passes[0].vertexDecl;

		bool foundCorrectDecl = false;
		int vertexSize = 0;
		D3DDECLTYPE positionType;
		D3DDECLTYPE texcoordType;
		D3DDECLMETHOD texcoordMethod;
		int positionOffset;
		int texcoordOffset;
		int colorOffset;

		for (int declNum = 0; declNum < 10; declNum++)
		{
			if (vertexDecl->declarations[declNum] == NULL)
			{
				continue;
			}

			vertexDecl->declarations[declNum]->GetDeclaration(decl, &numElements);

			for (int j = 0; j < numElements; j++)
			{
				int newSize = decl[j].Offset;

				switch (decl[j].Type)
				{
					case D3DDECLTYPE_FLOAT1:
					case D3DDECLTYPE_UBYTE4:
					case D3DDECLTYPE_UBYTE4N:
					case D3DDECLTYPE_D3DCOLOR:
						newSize += 4;
						break;
					case D3DDECLTYPE_FLOAT2:
						newSize += 8;
						break;
					case D3DDECLTYPE_FLOAT3:
						newSize += 12;
						break;
					case D3DDECLTYPE_FLOAT4:
						newSize += 16;
						break;
				}

				if (newSize > vertexSize)
				{
					vertexSize = newSize;
				}

				switch (decl[j].Usage)
				{
				case D3DDECLUSAGE_POSITION:
					positionOffset = decl[j].Offset;
					positionType = (D3DDECLTYPE)decl[j].Type;
					break;
				case D3DDECLUSAGE_TEXCOORD:
					if (decl[j].UsageIndex == 0)
					{
						texcoordOffset = decl[j].Offset;
						texcoordType = (D3DDECLTYPE)decl[j].Type;
					}
					break;
				case D3DDECLUSAGE_COLOR:
					colorOffset = decl[j].Offset;
					break;
				}
			}

			if (vertexSize == 32 && colorOffset == 16)
			{
				foundCorrectDecl = true;
				break;
			}
		}

		if (!foundCorrectDecl)
		{
			continue;
		}

		//vertexBuffer->Lock(vertexBase, surface->numVertices * vertexSize, (void**)&vertexData, D3DLOCK_READONLY);

		FILE* file = fopen(va("raw/%s_surf%i.obj", surfaces->name, i), "w");

		for (int j = 0; j < surface->numVertices; j++)
		{
			fprintf(file, "v %g %g %g\n", *(float*)(vertexData + (j * vertexSize) + positionOffset), *(float*)(vertexData + (j * vertexSize) + positionOffset + 4), *(float*)(vertexData + (j * vertexSize) + positionOffset + 8));
		}

		fprintf(file, "\n");

		for (int j = 0; j < surface->numVertices; j++)
		{
			/*float v[2];
			float in[4];
			in[0] = (float)(*(unsigned char*)(vertexData + (j * vertexSize) + texcoordOffset));
			in[1] = (float)(*(unsigned char*)(vertexData + (j * vertexSize) + texcoordOffset + 1));
			in[2] = (float)(*(unsigned char*)(vertexData + (j * vertexSize) + texcoordOffset + 2));
			in[3] = (float)(*(unsigned char*)(vertexData + (j * vertexSize) + texcoordOffset + 3));

			TransformTexCoord(in, v);*/

			//float v[2];
			//TransformTexCoord(*(DWORD*)(vertexData + (j * vertexSize) + texcoordOffset), v);

			s10e5 tX;
			s10e5 tY;

			tX.setBits(*(unsigned short*)(vertexData + (j * vertexSize) + texcoordOffset));
			tY.setBits(*(unsigned short*)(vertexData + (j * vertexSize) + texcoordOffset + 2));

			fprintf(file, "vt %g %g\n", (float)tX, (float)tY);
		}

		fprintf(file, "\n");

		/*for (int j = 0; j < surface->numVertices; j++)
		{
			DWORD packedUnitVec = *(DWORD*)(vertexData + (j * vertexSize) + texcoordOffset + 4);
			float v[3];

			float f1 = ((packedUnitVec >> 24) + 192.0f) / 3238.5f;
			v[0] = ((packedUnitVec & 0xFF) - 127) * f1;
			v[1] = (((packedUnitVec >> 8) & 0xFF) - 127) * f1;
			v[2] = (((packedUnitVec >> 16) & 0xFF) - 127) * f1;

			fprintf(file, "vn %g %g %g\n", v[0], v[1], v[2]);
		}*/

		fprintf(file, "\n");

		for (int j = 0; j < surface->numPrimitives; j++)
		{
			//fprintf(file, "f %i/%i/%i %i/%i/%i %i/%i/%i\n", indexData[j * 3] + 1, indexData[j * 3] + 1, indexData[j * 3] + 1, indexData[(j * 3) + 1] + 1, indexData[(j * 3) + 1] + 1, indexData[(j * 3) + 1] + 1, indexData[(j * 3) + 2] + 1, indexData[(j * 3) + 2] + 1, indexData[(j * 3) + 2] + 1);
			fprintf(file, "f %i/%i %i/%i %i/%i\n", indexData[j * 3] + 1, indexData[j * 3] + 1, indexData[(j * 3) + 1] + 1, indexData[(j * 3) + 1] + 1, indexData[(j * 3) + 2] + 1, indexData[(j * 3) + 2] + 1);
		}

		fclose(file);

		//vertexBuffer->Unlock();
		//indexBuffer->Unlock();
//#endif
	}
}

void PatchMW2_DumpModel()
{
//#if !D3D_EXPERIMENTS
	//return;
//#endif

	// D3DUSAGE_WRITEONLY flags
	//*(BYTE*)0x51E69C = 0;
	//*(BYTE*)0x51E7C8 = 0;

	static cmd_function_t dumpXModel;
	Cmd_AddCommand("dumpXModel", DumpXModel_f, &dumpXModel, 0);
}