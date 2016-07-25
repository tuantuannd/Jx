// KLVideo.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "KLVideo.h"
#include <windows.h>
#include <Mmsystem.h>
#include "KLMp4Video.h"
#include "KLMp4Audio.h"

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}


RADEXPFUNC u8 KLVideoSetSoundSystem(void PTR4* dd)
{
	return 0;
}

RADEXPFUNC HKLVIDEO KLVideoOpen(char PTR4* name,u32 flags, void PTR4* hWnd)
{
	HKLVIDEO hVideo = new KLVIDEO;
	char *cc;
	if(!hVideo)
		goto ErrorExit;

	char szPathName[256];
	if(GetModuleFileName(NULL, szPathName, 255) == 0)
		goto ErrorExit;
	
	cc = strrchr(szPathName,'\\');
	if(cc == NULL)
		goto ErrorExit;
	
	*(cc+1) = 0;
	strcat(szPathName,name);
	

	hVideo->pMpeg4Audio = new KLMp4Audio;
	if(!hVideo->pMpeg4Audio)
		goto ErrorExit;
	
	hVideo->pMpeg4Audio->SetWinHandle((HWND) hWnd);
	
	if(!hVideo->pMpeg4Audio->Open(name))
	{
		delete hVideo->pMpeg4Audio;
		hVideo->pMpeg4Audio = NULL;
	}
	else
		hVideo->pMpeg4Audio->Play();



	hVideo->pMpeg4Video = new KLMp4Video;
	if(!hVideo->pMpeg4Video)
		goto ErrorExit;
	
	if(!hVideo->pMpeg4Video->Open(name))
		goto ErrorExit;

	int nWidth, nHeight;
	hVideo->pMpeg4Video->GetVideoSize(nWidth, nHeight);

	hVideo->Width = (u32)nWidth;
	hVideo->Height = (u32)nHeight;
	hVideo->Frames = hVideo->pMpeg4Video->GetVideoFrames();
	hVideo->FrameNum = 0;

	hVideo->pMpeg4Video->Play();
	
	return hVideo;

ErrorExit:
	if(hVideo)
	{
		if(hVideo->pMpeg4Video)
			delete hVideo->pMpeg4Video;

		if(hVideo->pMpeg4Audio)
			delete hVideo->pMpeg4Audio;

		delete hVideo;
	}

	return NULL;
}

RADEXPFUNC void KLVideoClose(HKLVIDEO bnk)
{
	if(!bnk)
		return;

	if(bnk->pMpeg4Video)
	{
		bnk->pMpeg4Video->Close();
		delete bnk->pMpeg4Video;
	}

	if(bnk->pMpeg4Audio)
	{
		bnk->pMpeg4Audio->Close();
		delete bnk->pMpeg4Audio;
	}
		
	delete bnk;
}

RADEXPFUNC u32 KLVideoWait(HKLVIDEO bnk)
{
	if(!bnk || !bnk->pMpeg4Video)
		return false;
	
	return bnk->pMpeg4Video->ShouldWait();
}

RADEXPFUNC void KLVideoCopyToBuffer(HKLVIDEO bnk,void PTR4* buf,u32 left,u32 top,u32 Pitch,u32 destheight,u32 Flags)
{
	if(!bnk || !bnk->pMpeg4Video)
		return;

	bnk->pMpeg4Video->VideoCopyToBuffer(buf, left, top, Pitch, destheight, Flags);
	bnk->FrameNum = bnk->pMpeg4Video->GetCurFrame();
}

RADEXPFUNC u32 KLVideoDoFrame(HKLVIDEO bnk)
{
	return 1;
}

RADEXPFUNC void KLVideoNextFrame(HKLVIDEO bnk)
{
}

RADEXPFUNC void KLVideoSetSoundVolume(HKLVIDEO bnk,u32 nVolume)
{
	if(!bnk || !bnk->pMpeg4Audio)
		return;

	if(nVolume > 100)
		return;

	long n = 100 - (long)nVolume;
	bnk->pMpeg4Audio->SetVolume(-n * n);
}

RADEXPFUNC void KLVideoSetSoundTrack(u32 track)
{
}

RADEXPFUNC u32 KLVideoGetCurrentFrame(HKLVIDEO bnk)
{
	if(!bnk || !bnk->pMpeg4Video)
		return 0;

	return bnk->pMpeg4Video->GetCurFrame();
}
