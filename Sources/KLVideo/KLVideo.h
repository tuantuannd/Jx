#ifndef __KLVIDEO_H__
#define __KLVIDEO_H__

#define KLVIDEOVERSION "1.0h"
#define KLVIDEODATE    "2002-10-11"

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the KLVIDEO_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// KLVIDEO_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef KLVIDEO_EXPORTS
#define KLVIDEO_API __declspec(dllexport)
#else
#define KLVIDEO_API __declspec(dllimport)
#endif

#ifndef __RADRES__

#include "rad.h"

RADDEFSTART

#define KLVIDEONOSOUND			0xffffffff
#define KLVIDEOSNDTRACK			0x00004000L // Set the track number to play

#define KLVIDEOSURFACE8P          0
#define KLVIDEOSURFACE24          1
#define KLVIDEOSURFACE24R         2
#define KLVIDEOSURFACE32          3
#define KLVIDEOSURFACE32R         4
#define KLVIDEOSURFACE32A         5
#define KLVIDEOSURFACE32RA        6
#define KLVIDEOSURFACE4444        7
#define KLVIDEOSURFACE5551        8
#define KLVIDEOSURFACE555         9
#define KLVIDEOSURFACE565        10
#define KLVIDEOSURFACE655        11
#define KLVIDEOSURFACE664        12
#define KLVIDEOSURFACEYUY2       13
#define KLVIDEOSURFACEUYVY       14
#define KLVIDEOSURFACEYV12       15
#define KLVIDEOSURFACEMASK       15

class KLMp4Video;
class KLMp4Audio;

struct KLVIDEO {
	KLVIDEO(){memset(this,0,sizeof(KLVIDEO));}
	KLMp4Video PTR4* pMpeg4Video;
	KLMp4Audio PTR4* pMpeg4Audio;
	u32 Width;             // Width (1 based, 640 for example)
	u32 Height;            // Height (1 based, 480 for example)
	u32 Frames;            // Number of frames (1 based, 100 = 100 frames)
	u32 FrameNum;          // Frame to *be* displayed (1 based)
};

typedef struct KLVIDEO PTR4* HKLVIDEO;

RADEXPFUNC u8 KLVideoSetSoundSystem(void PTR4* dd);
RADEXPFUNC HKLVIDEO KLVideoOpen(char PTR4* name,u32 flags, void PTR4* hWnd);
RADEXPFUNC void KLVideoClose(HKLVIDEO bnk);
RADEXPFUNC u32 KLVideoWait(HKLVIDEO bnk);
RADEXPFUNC void KLVideoCopyToBuffer(HKLVIDEO bnk,void PTR4* buf,u32 left,u32 top,u32 Pitch,u32 destheight,u32 Flags);
RADEXPFUNC u32 KLVideoDoFrame(HKLVIDEO bnk);
RADEXPFUNC void KLVideoNextFrame(HKLVIDEO bnk);
RADEXPFUNC void KLVideoSetSoundVolume(HKLVIDEO bnk,u32 nVolume);
RADEXPFUNC void KLVideoSetSoundTrack(u32 track);
RADEXPFUNC u32 KLVideoGetCurrentFrame(HKLVIDEO bnk);

RADDEFEND

#endif

#endif


