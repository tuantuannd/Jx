# Microsoft Developer Studio Project File - Name="OmniCorbaLib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=OmniCorbaLib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "OmniCorbaLib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "OmniCorbaLib.mak" CFG="OmniCorbaLib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "OmniCorbaLib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "OmniCorbaLib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Library/CorbaLib", ZACAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "OmniCorbaLib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "OmniCorbaLib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ  /c
# ADD CPP /nologo /MD /W3 /Gm /GX /ZI /Od /I "..\..\Engine\Src" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "__WIN32__" /D "__x86__" /FD /GZ  /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "OmniCorbaLib - Win32 Release"
# Name "OmniCorbaLib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Src\AssistantServer.idl
# End Source File
# Begin Source File

SOURCE=.\Src\AssistantServerDynSK.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\AssistantServerSK.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\KAssistantServer.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\KCorbaClient.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\KCorbaServer.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\KDataManage.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\KReqServer.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\KSynData.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\KSynDataSet.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\KSynDataSets.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\KSynLock.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\ReqServer.idl
# End Source File
# Begin Source File

SOURCE=.\Src\ReqServerDynSK.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\ReqServerSK.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\SynDataSets.idl
# End Source File
# Begin Source File

SOURCE=.\Src\SynDataSetsDynSK.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\SynDataSetsSK.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Src\AssistantServer.h
# End Source File
# Begin Source File

SOURCE=.\Src\KAssistantServer.h
# End Source File
# Begin Source File

SOURCE=.\Src\KCorbaClient.h
# End Source File
# Begin Source File

SOURCE=.\Src\KCorbaServer.h
# End Source File
# Begin Source File

SOURCE=.\Src\KDataManage.h
# End Source File
# Begin Source File

SOURCE=.\Src\KReqServer.h
# End Source File
# Begin Source File

SOURCE=.\Src\KSynData.h
# End Source File
# Begin Source File

SOURCE=.\Src\KSynDataSet.h
# End Source File
# Begin Source File

SOURCE=.\Src\KSynDataSets.h
# End Source File
# Begin Source File

SOURCE=.\Src\KSynLock.h
# End Source File
# Begin Source File

SOURCE=.\Src\ReqServer.h
# End Source File
# Begin Source File

SOURCE=.\Src\SynDataSets.h
# End Source File
# Begin Source File

SOURCE=.\Src\TServerInfo.h
# End Source File
# End Group
# End Target
# End Project
