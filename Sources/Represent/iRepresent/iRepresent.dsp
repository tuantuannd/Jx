# Microsoft Developer Studio Project File - Name="iRepresent" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=iRepresent - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "iRepresent.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "iRepresent.mak" CFG="iRepresent - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "iRepresent - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "iRepresent - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/SwordOnline/Sources/Represent/iRepresent", VXGAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "iRepresent - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ""
# PROP Intermediate_Dir ""
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "IREPRESENT_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 /nologo /dll /machine:I386
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "iRepresent - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ""
# PROP Intermediate_Dir ""
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "IREPRESENT_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "IREPRESENT_EXPORTS" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 engine.lib /nologo /dll /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /debug

!ENDIF 

# Begin Target

# Name "iRepresent - Win32 Release"
# Name "iRepresent - Win32 Debug"
# Begin Group "Font"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Font\FontDef.h
# End Source File
# Begin Source File

SOURCE=.\Font\iFont.h
# End Source File
# Begin Source File

SOURCE=.\Font\KCharSet.cpp
# End Source File
# Begin Source File

SOURCE=.\Font\KCharSet.h
# End Source File
# Begin Source File

SOURCE=.\Font\KFont2.cpp
# End Source File
# Begin Source File

SOURCE=.\Font\KFont2.h
# End Source File
# Begin Source File

SOURCE=.\Font\KFont3.cpp
# End Source File
# Begin Source File

SOURCE=.\Font\KFont3.h
# End Source File
# Begin Source File

SOURCE=.\Font\KFontData.cpp
# End Source File
# Begin Source File

SOURCE=.\Font\KFontData.h
# End Source File
# Begin Source File

SOURCE=.\Font\KFontRes.cpp
# End Source File
# Begin Source File

SOURCE=.\Font\KFontRes.h
# End Source File
# Begin Source File

SOURCE=.\Font\KMRU.cpp
# End Source File
# Begin Source File

SOURCE=.\Font\KMRU.h
# End Source File
# End Group
# Begin Group "Image"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Image\ImageOperation.cpp
# End Source File
# Begin Source File

SOURCE=.\Image\ImageOperation.h
# End Source File
# End Group
# Begin Group "Text"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Text\TextProcess.cpp
# End Source File
# Begin Source File

SOURCE=.\Text\TextProcess.h
# End Source File
# Begin Source File

SOURCE=.\Text\TextProcessDef.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\iRepresentShell.h
# End Source File
# Begin Source File

SOURCE=.\KRepresentUnit.h
# End Source File
# End Target
# End Project
