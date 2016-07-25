// D3D_Shell.h
// 包括 Direct3D 对象和相关信息.
// 在渲染模块中CD3D_Shell对象只有一个.

#include "precompile.h"
#include "d3d_shell.h"
#include <algorithm>

CD3D_Shell g_D3DShell;						// The global D3D Shell...
D3DFORMAT g_PixelFormat[3] = {D3DFMT_X1R5G5B5, D3DFMT_R5G6B5, D3DFMT_X8R8G8B8};

// Create the Sucka...
bool CD3D_Shell::Create()
{
	FreeAll();								// Make sure everything is all clean before we start...

	// 创建 D3D 对象(通过它可以查询和创建D3D设备)...
	m_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (!m_pD3D) return false;

	// 取得桌面显示模式...
	if (FAILED(m_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &m_DesktopFormat))) { FreeAll(); return false; }

	// 创建设备列表 (适配器, 设备, 显示模式)...
	if (!BuildDeviceList()) { FreeAll(); return false; }
	
	return true;
}

// Resets back to initial conditions (doesn't try to free anything)...
void CD3D_Shell::Reset()
{
	m_pD3D = NULL;

	m_AdapterList.clear();
}

// Frees all the member vars and resets afterwards...
void CD3D_Shell::FreeAll()
{
	if (m_pD3D) {
		uint32 iRefCnt = m_pD3D->Release(); } // assert(iRefCnt==0);

	Reset();
}

// 创建设备列表 - 下面是基本步骤:
//	1. 循环遍历系统中的所有适配器 (通常只有一个),
//	2. 列举此设备的所有显示模式和象素格式.
//	3. 将所有设备加入适配器结构.
//	4. 选择一个缺省适配器和设备和显示模式...
bool CD3D_Shell::BuildDeviceList()
{
	if (!m_pD3D) return false;
	m_AdapterList.clear();				// Clear the Adapter List First...

    // 循环遍历系统中的所有适配器 (通常只有一个)
    for (UINT iAdapter = 0; iAdapter < m_pD3D->GetAdapterCount(); ++iAdapter)
	{
        D3DAdapterInfo AdapterInfo;		// Fill in adapter info
		AdapterInfo.iAdapterNum		= iAdapter;
        m_pD3D->GetAdapterIdentifier(iAdapter,0,&AdapterInfo.AdapterID);
        AdapterInfo.iNumDevices		= 0;
        AdapterInfo.iCurrentDevice	= 0;

		// Enumerate all display modes and formats on this adapter...
        vector<D3DModeInfo>		modes;	
        vector<D3DFORMAT>		formats;
		for(int i=0; i<3; i++)
		{
			uint32 iNumAdapterModes = m_pD3D->GetAdapterModeCount(iAdapter, g_PixelFormat[i]);

			D3DDISPLAYMODE DesktopMode;		// 将当前桌面象素格式加入列表
			m_pD3D->GetAdapterDisplayMode(iAdapter, &DesktopMode);
			formats.push_back(DesktopMode.Format);

			// 遍历此适配器的所有显示模式
			for (uint32 iMode = 0; iMode < iNumAdapterModes; iMode++)
			{
				uint32 m;
				D3DDISPLAYMODE d3dDisplayMode; 
				// 取得显示模式属性
				m_pD3D->EnumAdapterModes(iAdapter, g_PixelFormat[i],iMode, &d3dDisplayMode);
				D3DModeInfo DisplayMode; 
				DisplayMode.Width	 = d3dDisplayMode.Width; 
				DisplayMode.Height	 = d3dDisplayMode.Height;
				DisplayMode.Format	 = d3dDisplayMode.Format;

				// 过滤掉太低的分辨率
				if (DisplayMode.Width < 640 || DisplayMode.Height < 480) continue;

				// 检查模式是否已经存在 (过滤掉因为刷新率不同而重复的模式)
				for ( m=0L; m<modes.size(); ++m) {
					if ((modes[m].Width == DisplayMode.Width) && (modes[m].Height == DisplayMode.Height) && (modes[m].Format == DisplayMode.Format)) break; }

				// 如果发现了一个新的模式，将它加入列表

				if (m == modes.size()) 
				{
					// 检查此模式的象素格式是否已经存在，如果不存在则加入列表
					uint32 f;
					for ( f=0; f<formats.size(); ++f)
					{
						if (DisplayMode.Format == formats[f]) break; 
					}

					if (f==formats.size()) formats.push_back(DisplayMode.Format);

					modes.push_back(DisplayMode); 
				}
			}
		}

        sort(modes.begin(),modes.end());		// 将显示模式排序

        // 将设备加入适配器设备列表 (只加入我们感兴趣的设备)
		uint32		iNumDeviceTypes = 2L;
		TCHAR*		strDeviceDescs[] = { "HAL", "REF" };
		D3DDEVTYPE	DeviceTypes[]    = { D3DDEVTYPE_HAL, D3DDEVTYPE_REF };

        for (UINT iDevice = 0; iDevice < iNumDeviceTypes; ++iDevice)
		{
            D3DDeviceInfo Device;		// Fill in device info
            Device.DeviceType		 = DeviceTypes[iDevice];
            m_pD3D->GetDeviceCaps(iAdapter, DeviceTypes[iDevice], &Device.d3dCaps);
            Device.strDesc			 = strDeviceDescs[iDevice];
            Device.iCurrentMode		 = 0;
            Device.bCanDoWindowed	 = FALSE;
            Device.bWindowed		 = FALSE;
            Device.bStereo			 = FALSE;
            Device.MultiSampleType	 = D3DMULTISAMPLE_NONE;

            // 遍历所有象素格式, 看是否可以渲染和做硬件T&L加速
			vector<bool> bConfirmedFormats;
			vector<bool> bCanDoHWTnL;

            for (DWORD f=0; f<formats.size(); ++f)
			{
				bool bConfirmed = false; bool bHWTnL = false;

                // 检查是否可以渲染
                if (FAILED(m_pD3D->CheckDeviceType(iAdapter,Device.DeviceType,formats[f],formats[f],FALSE)))
				{
					bConfirmed = false; 
				}
				else
				{
					bConfirmed = true;
					// Confirm the device for HW vertex processing
					if (Device.d3dCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
						bHWTnL = true; 
					else 
						bHWTnL = false;
				}

				bConfirmedFormats.push_back(bConfirmed);
				bCanDoHWTnL.push_back(bHWTnL);
			}

            // 将所有象素格式符合要求的显示模式加入到设备显示模式列表
            for (uint32 m=0; m<modes.size(); ++m)
			{
                for (uint32 f=0; f<formats.size(); ++f)
				{
                    if (modes[m].Format == formats[f]) 
					{
                        if (bConfirmedFormats[f])
						{
							modes[m].bHWTnL = bCanDoHWTnL[f];
                            Device.Modes.push_back(modes[m]); 
						}
					}
				}
			}

            // 选择一个800×600模式为缺省模式 (希望选择一个16位模式)
         uint32 m;
			for (m=0; m<Device.Modes.size(); ++m)
			{
                if (Device.Modes[m].Width==800 && Device.Modes[m].Height==600)
				{
                    Device.iCurrentMode = m;
                    if (Device.Modes[m].Format == D3DFMT_R5G6B5 || Device.Modes[m].Format == D3DFMT_X1R5G5B5 || Device.Modes[m].Format == D3DFMT_A1R5G5B5)
						break;
				}
			}

            // 检查设备是否兼容于桌面显示模式
            // (桌面显示模式保存在formats[0]中)
            if (bConfirmedFormats[0])
			{
                Device.bCanDoWindowed = TRUE;
                Device.bWindowed      = TRUE;
			}

            // 发现了合适的设备, 将它加入设备列表
			AdapterInfo.Devices.push_back(Device);
		}
    
        // 发现了合适的设备, 将这个适配器加入列表
        m_AdapterList.push_back(AdapterInfo); }

    // Return an error if no compatible devices were found
    if (!m_AdapterList.size()) return false;

    return true;
}

// 由初始化函数调用，选择和创建设备 (g_Device)...
//	Pick Device Named szDevName if there's one of this name, otherwise, will pick what it thinks is best...
D3DDeviceInfo* CD3D_Shell::PickDefaultDev(D3DAdapterInfo** pAdapterInfo)
{
	for (vector<D3DAdapterInfo>::iterator itAdapter = m_AdapterList.begin(); itAdapter != m_AdapterList.end(); ++itAdapter)
	{
		for (vector<D3DDeviceInfo>::iterator itDevice = itAdapter->Devices.begin(); itDevice != itAdapter->Devices.end(); ++itDevice)
		{
			if (g_bRefRast && itDevice->DeviceType != D3DDEVTYPE_REF) continue;
			if (!g_bRefRast && itDevice->DeviceType != D3DDEVTYPE_HAL) continue;
			if (g_bRunWindowed && !itDevice->bWindowed) continue;

			*pAdapterInfo = &(*itAdapter); 
            return &(*itDevice);
		}
	} 
	return NULL;
}

D3DModeInfo* CD3D_Shell::PickDefaultMode(D3DDeviceInfo* pDeviceInfo,uint32 iBitDepth)
{
	for (vector<D3DModeInfo>::iterator itMode = pDeviceInfo->Modes.begin(); itMode != pDeviceInfo->Modes.end(); ++itMode)
	{
		if (itMode->Width  != g_nScreenWidth)  continue;
		if (itMode->Height != g_nScreenHeight) continue;
		if (g_bRunWindowed && itMode->Format != m_DesktopFormat.Format) continue; 
		if (!g_bRunWindowed) {
			switch (iBitDepth) {
			case 32 : if (itMode->Format != D3DFMT_X8R8G8B8 && itMode->Format != D3DFMT_R8G8B8)   continue; break;
			case 24 : if (itMode->Format != D3DFMT_X8R8G8B8 && itMode->Format != D3DFMT_R8G8B8)   continue; break;
			case 16 : if (itMode->Format != D3DFMT_R5G6B5   && itMode->Format != D3DFMT_X1R5G5B5) continue; break; } }

		return &(*itMode);
	}
	return NULL;
}

// 遍历所有的适配器/设备，将他们打印出来
void CD3D_Shell::ListDevices()
{
	for (vector<D3DAdapterInfo>::iterator itAdapter = m_AdapterList.begin(); itAdapter != m_AdapterList.end(); ++itAdapter)
	{
		for (vector<D3DDeviceInfo>::iterator itDevice = itAdapter->Devices.begin(); itDevice != itAdapter->Devices.end(); ++itDevice)
		{
			g_DebugLog("Device: %s", itDevice->strDesc);
		}
	}
}

D3DDeviceInfo* CD3D_Shell::FindDevice(const char* strDesc,D3DAdapterInfo** pAdapterInfo)
{
	for (vector<D3DAdapterInfo>::iterator itAdapter = m_AdapterList.begin(); itAdapter != m_AdapterList.end(); ++itAdapter)
	{
		for (vector<D3DDeviceInfo>::iterator itDevice = itAdapter->Devices.begin(); itDevice != itAdapter->Devices.end(); ++itDevice)
		{
			if (strcmp(strDesc,itDevice->strDesc)==0)
			{
				*pAdapterInfo = &(*itAdapter); return &(*itDevice); 
			}
		}
	}
	return NULL;
}

D3DAdapterInfo*	CD3D_Shell::GetAdapterInfo(unsigned int uAdapterID)
{
	if (uAdapterID < m_AdapterList.size())
		return &m_AdapterList[uAdapterID];
	return NULL;
}

D3DDeviceInfo*	CD3D_Shell::GetDeviceInfo(unsigned int uAdapterID, unsigned int uDeviceID)
{
	D3DAdapterInfo* pAdapter = GetAdapterInfo(uAdapterID);
	if (pAdapter)
	{
		if (uDeviceID < pAdapter->Devices.size())
			return &pAdapter->Devices[uDeviceID];
	}
	return NULL;
}

D3DModeInfo* CD3D_Shell::GetModeInfo(unsigned int uAdapterID,unsigned int uDeviceID,unsigned int uModeID)
{
	D3DDeviceInfo* pDevice = GetDeviceInfo(uAdapterID, uDeviceID);
	if (pDevice)
	{
		if (uModeID < pDevice->Modes.size())
			return &pDevice->Modes[uModeID];
	}
	return NULL;
}