// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Direct3D hooking performing FXAA support for MW2.
//
// Initial author: NTAuthority
// Started: 2011-10-15
// ==========================================================

#include "StdInc.h"
#if OLD_CRUFTY_CRUFT
#include <d3d9.h>
#include "myIDirect3D9.h"
#include "myIDirect3DDevice9.h"

myIDirect3D9* g_d3d9;
myIDirect3DDevice9* g_dx;

#ifndef USE_PRECOMPILED_SHADERS
//#include <d3dx9.h>
#endif

struct QVertex
{
	QVertex()
	{

	}

	QVertex(float x, float y, float z, float u, float v)
	{
		_x = x;
		_y = y;
		_z = z;
		_u = u;
		_v = v;
	}

	float _x, _y, _z, _u, _v;
};

const D3DVERTEXELEMENT9 g_vDec[3] =
{
	{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
	{ 0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
	D3DDECL_END()
};

static struct {
	IDirect3DTexture9* rtTexture;
	IDirect3DSurface9* rtSurface, *rtBackBuffer, *rtLast;
	IDirect3DIndexBuffer9* indexBuffer;
	IDirect3DVertexBuffer9* vertexBuffer;
	IDirect3DVertexDeclaration9* vertexDeclaration;
	IDirect3DTexture9* noiseTexture;
	//LPD3DXEFFECT postProcessingEffect;
	bool bInitialized;
	bool presented;
	bool doneThisScene;
	int scenes;
} g_postFX;

typedef HRESULT (WINAPI * Direct3DCreate9Ex_t)(UINT, IDirect3D9Ex**);

IDirect3D9* WINAPI CustomDirect3DCreate9(UINT SDKVersion)
{
	HMODULE hD3D9 = GetModuleHandle("d3d9.dll");
	Direct3DCreate9Ex_t procD3D9Ex = (Direct3DCreate9Ex_t)GetProcAddress(hD3D9, "Direct3DCreate9Ex");

	if (procD3D9Ex && !GAME_FLAG(GAME_FLAG_D3D9))
	{
		IDirect3D9Ex *d3d9;
		procD3D9Ex(SDKVersion, &d3d9);
		g_d3d9 = new myIDirect3D9(d3d9);
		return (g_d3d9);
	}

	return Direct3DCreate9(SDKVersion);
	//g_d3d9 = new myIDirect3D9(Direct3DCreate9(SDKVersion));
	//return g_d3d9;
}

void PatchMW2_FXAA()
{
	// hook Direct3DCreate9; might be better off elsewhere...
	*(DWORD*)0x6D74D0 = (DWORD)CustomDirect3DCreate9;
}

#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }

void PostFX_Initialize(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	//g_dx->GetRenderTarget(0, &g_postFX.rtBackBuffer);
	g_dx->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &g_postFX.rtBackBuffer);

	// RT
	D3DSURFACE_DESC desc;
	g_postFX.rtBackBuffer->GetDesc(&desc);

	g_dx->CreateTexture(desc.Width, desc.Height, 1, D3DUSAGE_RENDERTARGET, desc.Format, D3DPOOL_DEFAULT, &g_postFX.rtTexture, NULL);
	g_postFX.rtTexture->GetSurfaceLevel(0, &g_postFX.rtSurface);

	// vertex buffer
	QVertex* v;
	g_dx->CreateVertexBuffer(4 * sizeof(QVertex), D3DUSAGE_WRITEONLY, (D3DFVF_XYZ | D3DFVF_TEX1), D3DPOOL_MANAGED, &g_postFX.vertexBuffer, NULL);

	g_postFX.vertexBuffer->Lock(0, 0, (void**)&v, 0);
	v[0] = QVertex(-1.0f, -1.0f, 1.0f, 0.0f, 1.0f);
	v[1] = QVertex(-1.0f, 1.0f, 1.0f, 0.0f, 0.0f);
	v[2] = QVertex( 1.0f, 1.0f, 1.0f, 1.0f, 0.0f);
	v[3] = QVertex( 1.0f, -1.0f, 1.0f, 1.0f, 1.0f);
	g_postFX.vertexBuffer->Unlock();

	// index buffer
	WORD* i;
	g_dx->CreateIndexBuffer(6 * 2, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &g_postFX.indexBuffer, NULL);

	g_postFX.indexBuffer->Lock(0, 0, (void**)&i, 0);
	i[0] = 0; i[1] = 1; i[2] = 2;
	i[3] = 0; i[4] = 2; i[5] = 3;
	g_postFX.indexBuffer->Unlock();

	g_dx->CreateVertexDeclaration(g_vDec, &g_postFX.vertexDeclaration);

#if 0
	// load the HLSL file
	void* testEffect;
	int len = FS_ReadFile("test.fx", &testEffect);

	LPD3DXBUFFER errorBuffer = NULL;
	
	if (FAILED(D3DXCreateEffectFromFile(g_dx, "raw/test.fx",/* testEffect, len, */ NULL, NULL, 0, NULL, &g_postFX.postProcessingEffect, &errorBuffer)))
	{
		char* data = new char[errorBuffer->GetBufferSize()];
		memcpy(data, errorBuffer->GetBufferPointer(), errorBuffer->GetBufferSize());

		OutputDebugString(data);

		delete[] data;
		errorBuffer->Release();
	}

	FS_FreeFile(testEffect);

	// load the noise texture
	void* noiseTexture;
	len = FS_ReadFile("noise.dds", &noiseTexture);

	D3DXCreateTextureFromFileInMemory(g_dx, noiseTexture, len, &g_postFX.noiseTexture);

	FS_FreeFile(noiseTexture);
#endif

	// and done?
	g_postFX.bInitialized = true;
}

void PostFX_Reset()
{
	g_postFX.bInitialized = false;

	SAFE_RELEASE(g_postFX.rtSurface);
	SAFE_RELEASE(g_postFX.rtTexture);
	SAFE_RELEASE(g_postFX.rtLast);
	SAFE_RELEASE(g_postFX.rtBackBuffer);
	SAFE_RELEASE(g_postFX.indexBuffer);
	SAFE_RELEASE(g_postFX.vertexBuffer);
	SAFE_RELEASE(g_postFX.vertexDeclaration);
	//SAFE_RELEASE(g_postFX.postProcessingEffect);
	SAFE_RELEASE(g_postFX.noiseTexture);
}

void PostFX_PreBeginScene()
{
	
}

void PostFX_PostBeginScene()
{
	/*if (g_postFX.bInitialized)
	{
		g_dx->GetRenderTarget(0, &g_postFX.rtLast);

		if (g_postFX.presented/* && g_postFX.rtLast == g_postFX.rtBackBuffer* /)
		{
			g_dx->SetRenderTarget(0, g_postFX.rtSurface);
		}
	}

	if (g_postFX.bInitialized && g_postFX.presented/* && g_postFX.rtLast == g_postFX.rtBackBuffer* /) {
		g_dx->Clear(0,
			NULL,
			D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL,
			D3DCOLOR_ARGB(0,0,0,0),
			1.0f,
			0);

		g_postFX.presented = false;
		g_postFX.doneThisScene = false;
	}

	g_postFX.scenes++;*/
}

void PostFX_SetRenderTarget(DWORD rti, IDirect3DSurface9* rt)
{
	/*if (g_postFX.bInitialized && g_postFX.scenes == 1) {
		if (rt == g_postFX.rtBackBuffer) {
			SAFE_RELEASE(g_postFX.rtLast);
			g_postFX.rtLast = rt;
			g_dx->SetRenderTarget(0, g_postFX.rtSurface);

			g_postFX.doneThisScene = false;
			g_dx->Clear(0,
				NULL,
				D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL,
				D3DCOLOR_ARGB(0,0,0,0),
				1.0f,
				0);
		}
	}*/
}

void PostFX_Present()
{
	//g_postFX.presented = true;
}

void PostFX_DrawFrame() {
	if (g_postFX.bInitialized && !g_postFX.doneThisScene/* && g_postFX.rtLast == g_postFX.rtBackBuffer*/) {
		//g_dx->SetRenderTarget(0, g_postFX.rtLast);
		//g_dx->BeginScene();

		/*g_dx->Clear(0,
                   NULL,
                   D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL,
                   D3DCOLOR_ARGB(0,0,0,0),
                   1.0f,
                   0);*/

		//g_dx->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &p)
		g_dx->StretchRect(g_postFX.rtBackBuffer, NULL, g_postFX.rtSurface, NULL, D3DTEXF_NONE);
		//g_dx->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
		g_dx->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);

		//IDirect3DStateBlock9 * pDeviceState = NULL;
	    //g_dx->CreateStateBlock ( D3DSBT_ALL, &pDeviceState );
#if 0
		if (!GetAsyncKeyState(VK_F15))
		{
			g_postFX.postProcessingEffect->SetTechnique("DrawRegular"); //normal technique does not do anything, just draw the scene again
		}
		else
		{
			g_postFX.postProcessingEffect->SetTechnique("Draw"); //normal technique does not do anything, just draw the scene again
		}

		//g_postFX.postProcessingEffect->SetTechnique("Draw");
		g_postFX.postProcessingEffect->SetTexture("ColorTex", g_postFX.rtTexture); //set texture in shader to our postprocessing texture

		IDirect3DTexture9** floatZ = (IDirect3DTexture9**)0x6C39BE0;
		g_postFX.postProcessingEffect->SetTexture("ZTex", *floatZ);

		g_postFX.postProcessingEffect->SetTexture("RandTex", g_postFX.noiseTexture);

		UINT Pass, Passes; //Draw a rectangle to the screen using a rectangle mesh and quadtexture
		g_postFX.postProcessingEffect->Begin(&Passes, 0);
		for (Pass = 0; Pass < Passes; Pass++)
		{
 			g_postFX.postProcessingEffect->BeginPass(Pass);
			//PPQuadMesh->DrawSubset(0);
			g_dx->SetVertexDeclaration(g_postFX.vertexDeclaration);
			g_dx->SetStreamSource(0, g_postFX.vertexBuffer, 0, sizeof(QVertex));
			g_dx->SetIndices(g_postFX.indexBuffer);
			g_dx->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 6, 0, 2);
			g_postFX.postProcessingEffect->EndPass();
		}
		g_postFX.postProcessingEffect->End();

		/*if ( pDeviceState )
		{
			pDeviceState->Apply ( );
			pDeviceState->Release ( );
		}
		g_dx->EndScene();*/
#endif

		//g_postFX.doneThisScene = true;
	}

	SAFE_RELEASE(g_postFX.rtLast);

	//g_postFX.scenes--;
	g_postFX.presented = true;
}
#endif