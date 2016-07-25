/**************************************************************************
 *                                                                        *
 * This code has been developed by Adam Li. This software is an           *
 * implementation of a part of one or more MPEG-4 Video tools as          *
 * specified in ISO/IEC 14496-2 standard.  Those intending to use this    *
 * software module in hardware or software products are advised that its  *
 * use may infringe existing patents or copyrights, and any such use      *
 * would be at such party's own risk.  The original developer of this     *
 * software module and his/her company, and subsequent editors and their  *
 * companies (including Project Mayo), will have no liability for use of  *
 * this software or modifications or derivatives thereof.                 *
 *                                                                        *
 * Project Mayo gives users of the Codec a license to this software       *
 * module or modifications thereof for use in hardware or software        *
 * products claiming conformance to the MPEG-4 Video Standard as          *
 * described in the Open DivX license.                                    *
 *                                                                        *
 * The complete Open DivX license can be found at                         *
 * http://www.projectmayo.com/opendivx/license.php                        *
 *                                                                        *
 **************************************************************************/
/**
*  Copyright (C) 2001 - Project Mayo
 *
 * Adam Li
 * Andrea Graziani
 *
 * DivX Advanced Research Center <darc@projectmayo.com>
*
**/
// decore.h //

// This is the header file describing 
// the entrance function of the encoder core
// or the encore ...

#ifdef __cplusplus
extern "C" {
#endif 

#ifndef _DECORE_H_
#define _DECORE_H_

/**
 *
**/

// decore options
#define DEC_OPT_INIT		0x00008000
#define DEC_OPT_RELEASE 0x00010000
#define DEC_OPT_SETPP		0x00020000 // set postprocessing mode
#define DEC_OPT_SETOUT  0x00040000 // set output mode

// decore return values
#define DEC_OK					0
#define DEC_MEMORY			1
#define DEC_BAD_FORMAT	2

// supported output formats
#define YUV12		1
#define RGB32		2 // -biHeight
#define RGB24		3 // -biHeight
#define RGB555	4 // -biHeight
#define RGB565  5 // -biHeight
#define YUV2    6
#define UYVY    7

/**
 *
**/

typedef struct _DEC_PARAM_ 
{
	int x_dim; // x dimension of the frames to be decoded
	int y_dim; // y dimension of the frames to be decoded
	unsigned long color_depth; // leaved for compatibility (new value must be NULL)
	int output_format;
} DEC_PARAM;

typedef struct _DEC_FRAME_
{
	void *bmp; // the 24-bit decoded bitmap 
	void *bitstream; // the decoder buffer
	long length; // the lenght of the decoder stream
	int render_flag;
} DEC_FRAME;

typedef struct _DEC_SET_
{
	int postproc_level; // valid interval are [0..100]
} DEC_SET;

/**
 *
**/

// the prototype of the decore() - main decore engine entrance
//
int decore(
			unsigned long handle,	// handle	- the handle of the calling entity, must be unique
			unsigned long dec_opt, // dec_opt - the option for docoding, see below
			void *param1,	// param1	- the parameter 1 (it's actually meaning depends on dec_opt
			void *param2);	// param2	- the parameter 2 (it's actually meaning depends on dec_opt

#endif // _DECORE_H_
#ifdef __cplusplus
}
#endif 
