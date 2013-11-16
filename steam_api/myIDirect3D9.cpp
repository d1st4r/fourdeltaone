#include "StdInc.h"

#if OLD_CRUFTY_CRUFT
#include <d3d9.h>
#include "myIDirect3D9.h"
#include "myIDirect3DDevice9.h"

myIDirect3D9::myIDirect3D9(IDirect3D9Ex *pOriginal)
{
    m_pIDirect3D9 = pOriginal;
}

myIDirect3D9::~myIDirect3D9(void)
{
}

HRESULT  __stdcall myIDirect3D9::QueryInterface(REFIID riid, void** ppvObj)
{
    *ppvObj = NULL;

	// call this to increase AddRef at original object
	// and to check if such an interface is there

	HRESULT hRes = m_pIDirect3D9->QueryInterface(riid, ppvObj); 

	if (hRes == NOERROR) // if OK, send our "fake" address
	{
		*ppvObj = this;
	}
	
	return hRes;
}

ULONG    __stdcall myIDirect3D9::AddRef(void)
{
    return(m_pIDirect3D9->AddRef());
}

ULONG    __stdcall myIDirect3D9::Release(void)
{
    extern myIDirect3D9* g_d3d9;

	// call original routine
	ULONG count = m_pIDirect3D9->Release();
	
    // in case no further Ref is there, the Original Object has deleted itself
	// so do we here
	if (count == 0) 
	{
		g_d3d9 = NULL;
  	    delete(this); 
	}

	return(count);
}

HRESULT __stdcall myIDirect3D9::RegisterSoftwareDevice(void* pInitializeFunction)
{
    return(m_pIDirect3D9->RegisterSoftwareDevice(pInitializeFunction));
}

UINT __stdcall myIDirect3D9::GetAdapterCount(void)
{
    return(m_pIDirect3D9->GetAdapterCount());
}

HRESULT __stdcall myIDirect3D9::GetAdapterIdentifier(UINT Adapter,DWORD Flags,D3DADAPTER_IDENTIFIER9* pIdentifier)
{
    return(m_pIDirect3D9->GetAdapterIdentifier(Adapter,Flags,pIdentifier));
}

UINT __stdcall myIDirect3D9::GetAdapterModeCount(UINT Adapter, D3DFORMAT Format)
{
    return(m_pIDirect3D9->GetAdapterModeCount(Adapter, Format));
}

HRESULT __stdcall myIDirect3D9::EnumAdapterModes(UINT Adapter,D3DFORMAT Format,UINT Mode,D3DDISPLAYMODE* pMode)
{
    return(m_pIDirect3D9->EnumAdapterModes(Adapter,Format,Mode,pMode));
}

HRESULT __stdcall myIDirect3D9::GetAdapterDisplayMode( UINT Adapter,D3DDISPLAYMODE* pMode)
{
    return(m_pIDirect3D9->GetAdapterDisplayMode(Adapter,pMode));
}

HRESULT __stdcall myIDirect3D9::CheckDeviceType(UINT iAdapter,D3DDEVTYPE DevType,D3DFORMAT DisplayFormat,D3DFORMAT BackBufferFormat,BOOL bWindowed)
{
    return(m_pIDirect3D9->CheckDeviceType(iAdapter,DevType,DisplayFormat,BackBufferFormat,bWindowed));
}

HRESULT __stdcall myIDirect3D9::CheckDeviceFormat(UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT AdapterFormat,DWORD Usage,D3DRESOURCETYPE RType,D3DFORMAT CheckFormat)
{
    return(m_pIDirect3D9->CheckDeviceFormat(Adapter,DeviceType,AdapterFormat,Usage,RType,CheckFormat));
}

HRESULT __stdcall myIDirect3D9::CheckDeviceMultiSampleType(UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT SurfaceFormat,BOOL Windowed,D3DMULTISAMPLE_TYPE MultiSampleType,DWORD* pQualityLevels)
{
    return(m_pIDirect3D9->CheckDeviceMultiSampleType(Adapter,DeviceType,SurfaceFormat,Windowed,MultiSampleType,pQualityLevels));
}

HRESULT __stdcall myIDirect3D9::CheckDepthStencilMatch(UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT AdapterFormat,D3DFORMAT RenderTargetFormat,D3DFORMAT DepthStencilFormat)
{
    return(m_pIDirect3D9->CheckDepthStencilMatch(Adapter,DeviceType,AdapterFormat,RenderTargetFormat,DepthStencilFormat));
}

HRESULT __stdcall myIDirect3D9::CheckDeviceFormatConversion(UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT SourceFormat,D3DFORMAT TargetFormat)
{
    return(m_pIDirect3D9->CheckDeviceFormatConversion(Adapter,DeviceType,SourceFormat,TargetFormat));
}

HRESULT __stdcall myIDirect3D9::GetDeviceCaps(UINT Adapter,D3DDEVTYPE DeviceType,D3DCAPS9* pCaps)
{
    return(m_pIDirect3D9->GetDeviceCaps(Adapter,DeviceType,pCaps));
}

HMONITOR __stdcall myIDirect3D9::GetAdapterMonitor(UINT Adapter)
{
    return(m_pIDirect3D9->GetAdapterMonitor(Adapter));
}

void PostFX_Initialize(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters);

HWND g_nuiHWND;
int g_nuiWidth;
int g_nuiHeight;

HRESULT __stdcall myIDirect3D9::CreateDevice(UINT Adapter,D3DDEVTYPE DeviceType,HWND hFocusWindow,DWORD BehaviorFlags,D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DDevice9** ppReturnedDeviceInterface)
{
    // global var
	extern myIDirect3DDevice9* g_dx;

	// we intercept this call and provide our own "fake" Device Object
	IDirect3DDevice9* device;
	HRESULT hres = m_pIDirect3D9->CreateDevice( Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, &device);

	g_nuiHWND = pPresentationParameters->hDeviceWindow;
	g_nuiDraw->nuiWidth = pPresentationParameters->BackBufferWidth;
	g_nuiDraw->nuiHeight = pPresentationParameters->BackBufferHeight;

	g_nuiDraw->device = device;

    //
	// Create our own Device object and store it in global pointer
	// note: the object will delete itself once Ref count is zero (similar to COM objects)
	g_dx = new myIDirect3DDevice9(device);
	
	// store our pointer (the fake one) for returning it to the calling progam
	*ppReturnedDeviceInterface = g_dx;

	//PostFX_Initialize(g_dx, pPresentationParameters);

	return(hres); 
}
  
#endif