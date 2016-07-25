
// This file includes the main headers for d3drender.

#ifndef __PRECOMPILE_H__
#define __PRECOMPILE_H__

#define DIRECT3D_VERSION        0x0900

#include <windows.h>
#include <assert.h>

#include <d3d9types.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <d3d9caps.h>

#include "..\..\engine\src\KEngine.h"
#include "..\..\engine\src\KBmpFile24.h"
#include "..\..\engine\src\KWin32Wnd.h"
#include "../iRepresent/KRepresentUnit.h"
#include "..\iRepresent\RepresentUtility.h"

typedef char                int8;
typedef short               int16;
typedef int	                int32;
typedef unsigned char       uint8;
typedef unsigned short      uint16;
typedef unsigned int        uint32;
typedef unsigned __int64    uint64;

#include "BaseInclude.h"

#define FASTCALL __fastcall

// STL
#pragma warning(disable : 4786)

#endif  // __PRECOMPILE_H__



