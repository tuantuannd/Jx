// D3D_Shell.h
// 包括 Direct3D 对象和相关信息.
// 在渲染模块中CD3D_Shell对象只有一个. 

#ifndef __D3D_SHELL_H__
#define __D3D_SHELL_H__

#include <vector>
using namespace std;

// Forward declarations...
struct D3DDeviceInfo;
struct D3DModeInfo;

// 包含一个显示适配器信息的结构, 包括此适配器上可用的设备列表
// 除非你有两块适配器，否则只会有一个适配器对象
struct D3DAdapterInfo
{
	uint32					iAdapterNum;	// This is D3D's adapter count number (e.g. 2nd adapter)
    D3DADAPTER_IDENTIFIER9	AdapterID;		// Adapter data
    uint32					iCurrentDevice;	// Device data
    uint32					iNumDevices;
    vector<D3DDeviceInfo>	Devices;
};

// 保存Direct3D设备信息的结构, 包括一个此设备兼容的显示模式的列表
struct D3DDeviceInfo
{
    // Device data
    D3DDEVTYPE				DeviceType;		// Reference, HAL, etc.
    D3DCAPS9				d3dCaps;		// Capabilities of this device
    TCHAR*					strDesc;		// Name of this device
    BOOL					bCanDoWindowed;	// Whether this device can work in windowed mode

    // Mode data
    BOOL					bWindowed;
    BOOL					bStereo;
    D3DMULTISAMPLE_TYPE		MultiSampleType;
    uint32					iCurrentMode;
    vector<D3DModeInfo>		Modes;
};

// 包含显示模式信息的结构
struct D3DModeInfo
{
    uint32					Width;			// Screen width in this mode
	uint32					Height;			// Screen height in this mode
    D3DFORMAT				Format;			// Pixel format in this mode
    bool					bHWTnL;			// Whether to use Hardware or Software vertex processing
	bool operator < (const D3DModeInfo& other) const { 
		if (Format < other.Format) return true; else if (Format > other.Format) return false;
		if (Width  < other.Width) return true;  else if (Width > other.Width) return false;
		if (Height < other.Height) return true; else if (Height > other.Height) return false;
		return false; }
};

class CD3D_Shell {
public:
	CD3D_Shell()			{ Reset(); }
	~CD3D_Shell()			{ FreeAll(); }

	bool					Create();		// Create the Sucka...
	void					Reset();		// Resets back to initial conditions (doesn't try to free anything)...
	void					FreeAll();		// Frees all the member vars and resets afterwards...

    bool					BuildDeviceList();
	D3DDeviceInfo*			PickDefaultDev(D3DAdapterInfo** pAdapterInfo);					// Pick default device from our list...
	D3DModeInfo*			PickDefaultMode(D3DDeviceInfo* pDeviceInfo,uint32 iBitDepth);	// Pick default mode from the passed devices mode list...

	D3DAdapterInfo*			GetAdapterInfo(unsigned int uAdapterID);
	D3DDeviceInfo*			GetDeviceInfo(unsigned int uAdapterID,unsigned int uDeviceID);
	D3DModeInfo*			GetModeInfo(unsigned int uAdapterID,unsigned int uDeviceID,unsigned int uModeID);

	void					ListDevices();	// Go through all the Adapters/Devices and print them out...
	D3DDeviceInfo*			FindDevice(const char* strDesc,D3DAdapterInfo** pAdapterInfo);

	// Note: m_pD3D is public for quick access...
    LPDIRECT3D9				m_pD3D;			// The main D3D object

private:
	D3DDISPLAYMODE			m_DesktopFormat;// Desktop display mode (mainly for window mode)...
	vector<D3DAdapterInfo>	m_AdapterList;	// List of adaptors on this system (probably just one, unless you've got two cards)...
};

extern CD3D_Shell g_D3DShell;				// The global D3D Shell...
#define PDIRECT3D (g_D3DShell.m_pD3D)		// Use for quick access to the D3DDevice...

#endif