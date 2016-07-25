//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KDInput.cpp
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Direct Input Related Functions
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KMemBase.h"
#include "KWin32Wnd.h"
#include "KDError.h"
#include "KDInput.h"
//---------------------------------------------------------------------------
ENGINE_API KDirectInput* g_pDirectInput = NULL;
//---------------------------------------------------------------------------
// 函数:	KDirectInput
// 功能:	购造函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KDirectInput::KDirectInput()
{
	g_pDirectInput		= this;
	m_pDirectInput		= NULL;
	m_pMouseDevice		= NULL;
	m_pKeyboardDevice	= NULL;
}
//---------------------------------------------------------------------------
// 函数:	~KDirectInput
// 功能:	析造函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KDirectInput::~KDirectInput()
{
	Exit();
}
//---------------------------------------------------------------------------
// 函数:	Init
// 功能:	初始化DirectInput
// 参数:	void
// 返回:	TRUE－成功 FALSE－失败
//---------------------------------------------------------------------------
BOOL KDirectInput::Init()
{
	// free prior obj
	if (m_pDirectInput != NULL)
		Exit();

	if (!CreateDirectInput())
	{
		g_MessageBox("DirectInput : DirectInputCreate() Failed");
		return FALSE;
	}

	// create mouse device
	if (!CreateMouseDevice())
	{
		g_MessageBox("DirectInput : CreateMouseDevice() Failed");
		return FALSE;
	}

	// create keyboard device
	if (!CreateKeyboardDevice())
	{
		g_MessageBox("DirectInput : CreateKeyboardDevice() Failed");
		return FALSE;
	}

	g_DebugLog("DirectInput init ok");
	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	Exit
// 功能:	释放DirectInput
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KDirectInput::Exit()
{
	if (m_pMouseDevice)
	{
		m_pMouseDevice->Unacquire();
		m_pMouseDevice->Release();
		m_pMouseDevice = NULL;
	}
	if (m_pKeyboardDevice)
	{
		m_pKeyboardDevice->Unacquire();
		m_pKeyboardDevice->Release();
		m_pKeyboardDevice = NULL;
	}
	if (m_pDirectInput)
	{
		m_pDirectInput->Release();
		m_pDirectInput = NULL;
	}
	g_DebugLog("DirectInput release ok");
}
//---------------------------------------------------------------------------
// 函数:	CreateDirectInput
// 功能:	创建DirectInput
// 参数:	void
// 返回:	TRUE－成功 FALSE－失败
//---------------------------------------------------------------------------
BOOL KDirectInput::CreateDirectInput()
{
	HRESULT  hres;

	hres = DirectInput8Create(
		GetModuleHandle(NULL),
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		(void**)&m_pDirectInput,
		NULL);
#ifndef _DEBUG		
	if (hres != DI_OK)
	{
		g_DIError(hres);
		return FALSE;
	}
#endif
	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	CreateMouseDevice
// 功能:	创建 Mouse Device
// 参数:	void
// 返回:	TRUE－成功 FALSE－失败
//---------------------------------------------------------------------------
BOOL KDirectInput::CreateMouseDevice()
{
	DWORD    Flags;
	HRESULT  hres;

	// check direct input
	if (m_pDirectInput == NULL)
		return FALSE;

	// creating the DirectInput Mouse Device
	hres = m_pDirectInput->CreateDevice(
		GUID_SysMouse,
		&m_pMouseDevice,
		NULL);
	if (hres != DI_OK)
	{
		g_DIError(hres);
		return FALSE;
	}

	// setting the Mouse Data Format
	hres = m_pMouseDevice->SetDataFormat(&c_dfDIMouse);
	if (hres != DI_OK)
	{
		g_DIError(hres);
		return FALSE;
	}

	// setting the Mouse Behavior flag
	Flags = DISCL_FOREGROUND | DISCL_NONEXCLUSIVE;
	hres = m_pMouseDevice->SetCooperativeLevel(g_GetMainHWnd(), Flags);

#ifndef _DEBUG	
	if (hres != DI_OK)
	{
		g_DIError(hres);
		return FALSE;
	}
#endif
	// acquire obtains access to the input device
	hres = m_pMouseDevice->Acquire();
//Question 不知为什么用_DEBUG系统为未发现_DEBUG，固在此在一下

#ifndef _DEBUG		
	if (hres != DI_OK)
	{
		g_DIError(hres);
		return FALSE;
	}
#endif
	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	CreateKeyboardDevice
// 功能:	创建键盘设备
// 参数:	void
// 返回:	TRUE－成功 FALSE－失败
//---------------------------------------------------------------------------
BOOL KDirectInput::CreateKeyboardDevice()
{
	DWORD   Flags;
	HRESULT hres;

	// check direct input
	if (m_pDirectInput == NULL)
		return FALSE;

	// creating the DirectInput Keyboard Device
	hres = m_pDirectInput->CreateDevice(
		GUID_SysKeyboard,
        &m_pKeyboardDevice,
		NULL);
	if (hres != DI_OK)
	{
		g_DIError(hres);
		return FALSE;
	}

	// setting the Keyboard Data Format
	hres = m_pKeyboardDevice->SetDataFormat(&c_dfDIKeyboard);
	if (hres != DI_OK)
	{
		g_DIError(hres);
		return FALSE;
	}

	// setting the Keyboard Behavior
	Flags = DISCL_FOREGROUND | DISCL_NONEXCLUSIVE;
	hres = m_pKeyboardDevice->SetCooperativeLevel(g_GetMainHWnd(), Flags);

#ifndef _DEBUG		
	if (hres != DI_OK)
	{
		g_DIError(hres);
		return FALSE;
	}
#endif

	// acquire obtains access to the input device
	hres = m_pKeyboardDevice->Acquire();
#ifndef _DEBUG	
	if (hres != DI_OK)
	{
		g_DIError(hres);
		return FALSE;
	}
#endif

	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	SetMouseBehavior
// 功能:	设置 Mouse 协作模式
// 参数:	bExclusive ＝ TRUE 为独占模式
// 返回:	TRUE－成功 FALSE－失败
//---------------------------------------------------------------------------
BOOL KDirectInput::SetMouseBehavior(BOOL bExclusive)
{
	DWORD    Flags;
	HRESULT  hres;

	if (bExclusive)
	{
		Flags = DISCL_FOREGROUND | DISCL_EXCLUSIVE;
	}
	else
	{
		Flags = DISCL_FOREGROUND | DISCL_NONEXCLUSIVE;
	}
	hres = m_pMouseDevice->SetCooperativeLevel(g_GetMainHWnd(), Flags);
	if (hres != DI_OK)
	{
		g_DIError(hres);
		return FALSE;
	}
	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	GetMouseState
// 功能:	取得鼠标状态
// 参数:	pDx		水平偏移
//			pDy		垂直偏移
//			pLb		左键状态
//			pRb		右键状态
// 返回:	void
//---------------------------------------------------------------------------
BOOL KDirectInput::GetMouseState(PINT pDx, PINT pDy, PBYTE pLb, PBYTE pRb)
{
	DIMOUSESTATE  dims;
	HRESULT       hres;

	if (m_pMouseDevice == NULL)
		return FALSE;
	hres = m_pMouseDevice->GetDeviceState(sizeof(dims), &dims);
	if (hres == DI_OK)
	{
		*pDx = dims.lX;
		*pDy = dims.lY;
		*pLb = dims.rgbButtons[0];
		*pRb = dims.rgbButtons[1];
	}
	else
	{
		m_pMouseDevice->Acquire();
		return FALSE;
	}
	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	GetKeyboardState
// 功能:	取得键盘状态
// 参数:	KeyBuf	状态缓存
// 返回:	void
//---------------------------------------------------------------------------
BOOL KDirectInput::GetKeyboardState(PBYTE KeyBuffer)
{
	if (m_pKeyboardDevice == NULL)
		return FALSE;
	if (m_pKeyboardDevice->GetDeviceState(256, KeyBuffer) != DI_OK)
	{
		m_pKeyboardDevice->Acquire();
		return FALSE;
	}
	return TRUE;
}
//---------------------------------------------------------------------------

