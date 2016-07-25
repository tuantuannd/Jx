/////////////////////////////////////////////////////////////////////////////////
// BASEPCH0.H - Borland C++ Builder pre-compiled header file
// $Revision:   1.0.1.0.1.2  $
// Copyright (c) 1997, 2002 Borland Software Corporation
//
// BASEPCH0.H is the core header that includes VCL/CLX headers. The headers
// included by BASEPCH0.H are governed by the following macros:
//
//  MACRO                   DESCRIPTION                                 DEFAULT
// =======                 =============                                =======
//
//  NO_WIN32_LEAN_AND_MEAN  When this macro is defined, BASEPCH.H does    OFF
//                          not define WIN32_LEAN_AND_MEAN.
//
//  INC_VCL                 This macro is defined by VCL.H to             OFF
//                          include the base set of VCL headers
//
//  VCL_FULL                Same as NO_WIN32_LEAN_AND_MEAN                OFF
//                          (NOTE: This macro is for BCB v1.0 backward
//                          compatibility)
//
//  INC_VCLDB_HEADERS       When this macro is defined, VCL.H includes
//    requires INC_VCL      the core Database headers of VCL.             OFF
//                          (Defining this macro is functionally
//                          equivalent to including VCLDB.H)
//
//  INC_VCLEXT_HEADERS      When this macro is defined, VCL.H includes
//    requires INC_VCL      all VCL headers.                              OFF
//                          (Defining this macro is functionally
//                          equivalent to including VCLMAX.H)
//
//  INC_CLX                 This macro is defined by CLX.H to include     OFF
//                          the base set of CLX headers
//
//  INC_CLXDB_HEADERS       When this macro is defined, CLX.H includes
//    requires INC_CLX      the core Database headers of CLX.             OFF
//
//  INC_CLXEXT_HEADERS      When this macro is defined, CLX.H includes
//    requires INC_CLX      all CLX headers.                              OFF
//
/////////////////////////////////////////////////////////////////////////////////
#ifndef __BASEPCH0_H__
#define __BASEPCH0_H__

// v1.0 of BCB checked for VCL_FULL to avoid LEAN_AND_MEAN support
//
#if defined(VCL_FULL)                 // BCB v1.0 compatible
#define NO_WIN32_LEAN_AND_MEAN
#endif

#if !defined(_WINDOWS_)               // Don't optimize if WINDOWS.H has already been included
#if !defined(NO_WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN           // Enable LEAN_AND_MEAN support
#define  _VCL_LEAN_AND_MEAN           // BCB v1.0 compatible
#endif                                // NO_WIN32_LEAN_AND_MEAN
#endif                                // _WINDOWS_

#if !defined(COM_NO_WINDOWS_H)        // Make sure COM Headers don't include WINDOWS.H/OLE2.H
#define COM_NO_WINDOWS_H
#define UNDEF_COM_NO_WINDOWS_H
#endif

#if !defined(RPC_NO_WINDOWS_H)        // Make sure RPC Headers don't include WINDOWS.H
#define RPC_NO_WINDOWS_H
#define UNDEF_RPC_NO_WINDOWS_H
#endif

// Core (minimal) Delphi RTL headers
//
#include <System.hpp>
#include <Types.hpp>
#include <Windows.hpp>
#include <Messages.hpp>
#include <SysUtils.hpp>
#include <Classes.hpp>


// Core (minimal) VCL headers
//
#if defined(INC_VCL)
#include <Controls.hpp>
#include <Graphics.hpp>
#include <Forms.hpp>
#include <Dialogs.hpp>
#include <StdCtrls.hpp>
#include <ExtCtrls.hpp>

// VCL Database related headers
//
#if defined(INC_VCLDB_HEADERS)
#include <DBCtrls.hpp>
#include <DB.hpp>
#include <DBTables.hpp>
#endif  //  INC_VCLDB_HEADERS

// Full set of VCL headers
//
#if defined(INC_VCLEXT_HEADERS)
#include <Buttons.hpp>
#include <ChartFX.hpp>
#include <ComCtrls.hpp>
#include <DBCGrids.hpp>
#include <DBGrids.hpp>
#include <DBLookup.hpp>
#include <DdeMan.hpp>
#include <FileCtrl.hpp>
#include <GraphSvr.hpp>
#include <Grids.hpp>
#include <MPlayer.hpp>
#include <Mask.hpp>
#include <Menus.hpp>
#include <OleCtnrs.hpp>
#include <OleCtrls.hpp>
#include <Outline.hpp>
#include <Tabnotbk.hpp>
#include <Tabs.hpp>
#include <VCFImprs.hpp>
#include <VCFrmla1.hpp>
#include <VCSpell3.hpp>
#endif  // INC_VCLEXT_HEADERS

#endif  //  INC_VCL

// Core (minimal) CLX headers
//
#if defined(INC_CLX)
#include <Qt.hpp>
#include <BindHelp.hpp>
#include <QConsts.hpp>
#include <QStdActns.hpp>
#include <QStyle.hpp>
#include <QControls.hpp>
#include <QTypes.hpp>
#include <QGraphics.hpp>
#include <QForms.hpp>
#include <QDialogs.hpp>
#include <QStdCtrls.hpp>
#include <QExtCtrls.hpp>
#include <QMenus.hpp>
#include <QClipbrd.hpp>
#include <QMask.hpp>
#include <QButtons.hpp>
#include <QComCtrls.hpp>

// CLX Database related headers
//
#if defined(INC_CLXDB_HEADERS)
#include <QDBCtrls.hpp>
#include <DB.hpp>
#include <DBTables.hpp>
#endif  //  INC_CLXDB_HEADERS

// Full set of CLX headers
//
#if defined(INC_CLXEXT_HEADERS)
#include <QDBGrids.hpp>
#include <QDBCtrls.hpp>
#include <QDBActns.hpp>
#include <QGrids.hpp>
#include <QImgList.hpp>
#include <QSearch.hpp>
#include <QActnList.hpp>
#include <QCheckLst.hpp>
#include <QPrinters.hpp>
#endif  // INC_CLXEXT_HEADERS

#endif  //  INC_CLX

#if defined(INC_OLE_HEADERS)
#include <cguid.h>
#include <dir.h>
#include <malloc.h>
#include <objbase.h>
#include <ole2.h>
#include <shellapi.h>
#include <stddef.h>
#include <tchar.h>
#include <urlmon.h>
#include <AxCtrls.hpp>
#include <DataBkr.hpp>
#include <OleCtnrs.hpp>
#include <OleCtrls.hpp>
#include <Mtx.hpp>
#endif

// Using ATLVCL.H
//
#if defined(INC_ATL_HEADERS)
#include <atl\atlvcl.h>
#endif

#if defined(INC_CORBA_HEADERS)
#include <corbapch.h>
#endif

#if defined(UNDEF_COM_NO_WINDOWS_H)   // Clean up MACRO to prevent inclusion of WINDOWS.H/OLE2.H
#undef COM_NO_WINDOWS_H
#undef UNDEF_COM_NO_WINDOWS_H
#endif

#if defined(UNDEF_RPC_NO_WINDOWS_H)   // Clean up MACRO to prevent inclusion of WINDOWS.H
#undef RPC_NO_WINDOWS_H
#undef UNDEF_RPC_NO_WINDOWS_H
#endif

#endif  // __BASEPCH0_H__
