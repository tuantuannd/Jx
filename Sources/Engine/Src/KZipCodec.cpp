//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KZipCodec.cpp
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Zip Decode of inflate method
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KMemBase.h"
#include "KZipCodec.h"
//---------------------------------------------------------------------------
// local file signature
#define LF_SIG				0x0403			// local file header signature
#define FH_SIG				0x0201			// file header signature
#define ED_SIG				0x0605			// end of central dir signature

// general purpose flag
#define LH_FLAG_ENCRYPT 	0x01			// file is encrypted

// for Method 8 - Deflating
#define LF_FLAG_NORM		0x00			// normal compression
#define LF_FLAG_MAX 		0x02			// maximum compression
#define LF_FLAG_FAST		0x04			// fast compression
#define LF_FLAG_SUPER		0x06			// super fast compression
#define LF_FLAG_DDREC		0x08			// use data descriptor record

// compression method
#define LF_CM_STORED		0x00			// stored
#define LF_CM_SHRUNK		0x01			// shrunk
#define LF_CM_REDUCED1		0x02			// reduced with factor 1
#define LF_CM_REDUCED2		0x03			// reduced with factor 2
#define LF_CM_REDUCED3		0x04			// reduced with factor 3
#define LF_CM_REDUCED4		0x05			// reduced with factor 4
#define LF_CM_IMPLODED		0x06			// imploded
#define LF_CM_TOKENIZED 	0x07			// tokenized (not used)
#define LF_CM_DEFLATED		0x08			// deflated

// decode buffer
#define BUFFER_SIZE 		32768			// dictionary/output buffer size
#define COPY_BUFFER 		16384			// copy buffer size

// huffman decode
#define MAX_BITS			16				// maximum bits in code
#define CODE_MAX			288 			// maximum nbr of codes in set
//---------------------------------------------------------------------------
// Inflate Constants
// These tables are taken from PKZIP's appnote.txt.
//---------------------------------------------------------------------------
// bit length code lengths
static UINT bll[] =
{
    16, 17, 18, 0, 8, 7, 9, 6, 10, 5,
	11, 4, 12, 3, 13, 2, 14, 1, 15
};
// copy lengths for literal ..codes 257 thru 285
static UINT cll[] =
{
    3, 4, 5, 6, 7, 8, 9, 10, 11, 13,
	15, 17, 19, 23, 27, 31, 35, 43,
	51, 59, 67, 83, 99, 115, 131, 163,
	195, 227, 258, 0, 0
};
// extra bits for literal ..codes 257 thru 285
static UINT cle[] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
	1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4,
	4, 4, 5, 5, 5, 5, 0, 99, 99
};
// copy distance offsets ..for codes 0 thru 29
static UINT cdo[] =                            
{
    1, 2, 3, 4, 5, 7, 9, 13, 17, 25,
	33, 49, 65, 97, 129, 193, 257, 385,
	513, 769, 1025, 1537, 2049, 3073,
	4097, 6145, 8193, 12289, 16385, 24577
};
// copy extra bits for distance ..codes 0 thru 29
static UINT cde[] =
{
    0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4,
	5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10,
	11, 11, 12, 12, 13, 13
};
//---------------------------------------------------------------------------
// 函数:	KZipCodec
// 功能:	购造函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KZipCodec::KZipCodec()
{
	bhold = 0;
	rsize = 0;
	e_count = 0;
	bsize = 0;
	sb_size = 0;
	sb = NULL;
	sbp = NULL;
	sbe = NULL;
	m_pZipOut = NULL;
	g_MemZero(&m_Lf, sizeof(m_Lf));
}
//---------------------------------------------------------------------------
// 函数:	Decode
// 功能:	Decode a sub file in zip
// 参数:	pBuffer 		Output buffer
//			dwSize			Size of output buffer 
//			Offset			sub file offset
// 返回:	TRUE			Success
//			FALSE			Fail
//---------------------------------------------------------------------------
BOOL KZipCodec::Decode(PVOID pInBuf, PVOID pOutBuf, LF* pLf)
{
	KASSERT(pInBuf);
	KASSERT(pOutBuf);

	m_pZipIn = (PBYTE)pInBuf;
	m_pZipOut = (PBYTE)pOutBuf;

	m_Lf = *pLf;

	extract_init();
	extract_file();
	extract_exit();

	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	extract_init
// 功能:	extract initialize
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KZipCodec::extract_init()
{
	// set up the dictionary size (32k)
	sb_size = (UINT)BUFFER_SIZE;

	// get memory for output buffer
	if (sb == NULL)
		sb = (PBYTE)malloc(sb_size); 
	
	// set up end of buffer address
	sbe = &sb[sb_size];
}
//---------------------------------------------------------------------------
// 函数:	extract_file
// 功能:	extract file from a ZIP file
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KZipCodec::extract_file()
{
	if (m_Lf.lf_flag & LH_FLAG_ENCRYPT)
	{
		g_MessageBox("KZipCodec: The Zip File is Encrypt!");
		return;							
	}
	
	rsize = m_Lf.lf_csize;			// remaining filesize to process
	bsize = 0;						// ..bits in byte to process
	e_count = 2;					// end of data counter/flag
	sbp = sb;						// reset next output pointer
	memset(sb, 0, sb_size); 		// ..and dictionary/output buffer
	
	switch (m_Lf.lf_cm) 			// based on compression method
	{
	case LF_CM_STORED:				// stored file
		extract_stored();			// process stored file
		break;						// ..then do next file
		
	case LF_CM_DEFLATED:			// deflated file
		extract_inflate();			// process deflated file
		break;						// ..then do next file
		
	default:
		g_MessageBox("KZipCodec: Unsuport compress method");
		return; 					// ..and return to caller
	}
	
	extract_flush();				// else .. flush our output buffer
}
//---------------------------------------------------------------------------
// 函数:	extract_exit
// 功能:	extract exit
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KZipCodec::extract_exit()
{
	if (sb != NULL)
	{
		free(sb);
		sb = NULL;
	}
}
//---------------------------------------------------------------------------
// 函数:	extract_flush
// 功能:	write output buffer as needed
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KZipCodec::extract_flush()
{
	// q. local buffer need flushing?
	if (sbp != sb)
	{	
		g_MemCopy(m_pZipOut, sb, (UINT)(sbp - sb));
		m_pZipOut += (UINT)(sbp - sb);
	}
}
//---------------------------------------------------------------------------
// 函数:	store_char
// 功能:	store character in output file
// 参数:	c		character to store
// 返回:	void
//---------------------------------------------------------------------------
void KZipCodec::store_char(char c)
{
	*sbp = c;			// a. yes .. put char in buffer
	if (++sbp >= sbe)	// q. past end of buffer?
	{	
		// a. yes .. write buffer
		g_MemCopy(m_pZipOut, sb, sb_size);
		m_pZipOut += sb_size;						
		sbp = sb;		// ..then back to start again
	}
}
//---------------------------------------------------------------------------
// 函数:	lookat_code
// 功能:	look at the next code from input file
// 参数:	bits		number of bits
// 返回:	UINT
//---------------------------------------------------------------------------
UINT KZipCodec::lookat_code(int bits)
{
	static UINT bit_mask[] = // bit masks
	{
		0x0000,
		0x0001, 0x0003, 0x0007, 0x000f,
		0x001f, 0x003f, 0x007f, 0x00ff,
		0x01ff, 0x03ff, 0x07ff, 0x0fff,
		0x1fff, 0x3fff, 0x7fff, 0xffff
	};
	UINT ch = 0;

	while (bsize < bits)							// get enough bits
	{
		if (rsize > 0)								// q. anything left to process?
		{
			ch = (UINT)*m_pZipIn++;
			bhold |= ch << bsize;			// a. yes .. get a character
			bsize += 8; 							// ..and increment counter
			rsize--;								// ..finally, decrement cnt
		}
		else if (! e_count) 					// q. done messing around?
		{
			extract_flush();						// a. yes .. flush output buffer
			g_MessageBox("KZipCodec::lookat_code() error");
			exit(1);
			break;									// ..then quit with an error msg
		}
		else if (e_count > 0)						// q. down counting?
		{
			e_count--;								// a. yes .. decriment counter
			return(-1); 							// ..and return all done
		}
		else
		{
			e_count++;								// else .. count up
			break;									// ..and exit loop
		}
	}

	return (((UINT)bhold) & bit_mask[bits]);		// return just enough bits
}
//---------------------------------------------------------------------------
// 函数:	get_code
// 功能:	get next code from input file
// 参数:	bits		number of bits in this code
// 返回:	UINT
//---------------------------------------------------------------------------
UINT KZipCodec::get_code(int bits)
{
	UINT a; 					// accumulator

	a = lookat_code(bits);		// prepare return value
	bhold >>= bits; 			// ..shift out requested bits
	bsize -= bits;				// ..decrement remaining bit count
	return(a);					// ..and return value
}
//---------------------------------------------------------------------------
// 函数:	extract_copy()
// 功能:	copy stored data to output stream
// 参数:	len 		length to copy
// 返回:	void
//---------------------------------------------------------------------------
void KZipCodec::extract_copy(UINT len)
{
	char	*b; 								// work buffer pointer
	char	*p; 								// work pointer
	UINT	csize = 0;							// current read size
	
	b = (char *)malloc(COPY_BUFFER);			// get an file buffer
	
	while (len) 								// loop copying file to output
	{
		csize = (len <= COPY_BUFFER) ?			// determine next read size
			len : COPY_BUFFER;

		g_MemCopy(b, m_pZipIn, csize);		// read zip data to buffer
		m_pZipIn += csize;
		
		rsize -= csize; 						// decrement remaining length
		len -= csize;							// ..and loop control length
		
		for (p = b; csize--;)					// for the whole block
			store_char(*p++);					// ..write output data
	}
	free(b);									// ..and release file buffer
}
//---------------------------------------------------------------------------
// 函数:	extract_stored()
// 功能:	extract a stored file
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KZipCodec::extract_stored(void)
{
	extract_copy(rsize);
}
//---------------------------------------------------------------------------
// 函数:	extract_inflate()
// 功能:	extract a inflate file
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KZipCodec::extract_inflate()
{
	UINT	c;								// current read character
	UINT	eoj = 0;						// end of job flag
	
	if (m_Lf.lf_flag & LF_FLAG_DDREC)		// q. need data descriptor rec?
	{
		g_MessageBox("KZipCodec::extract_inflate() error");
		exit(1);
	}
	
	e_count = -e_count; 					// set end count to negative
	
	while (! eoj) 							// loop till end of job
	{
		eoj = get_code(1);					// get the eoj bit
		c = get_code(2);					// ..then get block type
		
		switch (c)							// depending on block type
		{
		case 0: 							// 0: stored block
			bsize = 0;						// flush remaining bits
			c = get_code(16);				// get block length ok

			m_pZipIn += 2;					// skip the ones complement word
			rsize -= 2; 					// ..and its count
			
			extract_copy(c);				// copy bytes to output stream
			break;							// ..and get next block
			
		case 1: 							// 1: fixed Huffman codes
			eoj |= inf_fixed(); 			// process a fixed block
			break;							// ..then get another block
			
		case 2: 							// 2: dynamic Huffman codes
			eoj |= inf_dynamic();			// process the dynamic block
			break;							// ..then get next block
			
		case 3: 							// 3: unknown type
			g_MessageBox("KZipCodec::extract_inflate() error");
			exit(1);						// quit with an error message
		}
	}
}
//---------------------------------------------------------------------------
//
// inf_build_tree() -- build a Huffman tree
//
// Returns: TRUE if error building Huffman tree
//			FALSE if tree built
//
//---------------------------------------------------------------------------
int KZipCodec::inf_build_tree(
					   UINT *b,				// code lengths in bits
					   UINT n,				// number of codes
					   UINT s,				// number of simple codes
					   UINT *d, 			// base values for non-simple
					   UINT *e, 			// list of extra bits
					   HUFF **t,			// resulting table
					   UINT *m) 			// maximum lookup bits
{
	UINT	a,								// code lengths of k
		c[MAX_BITS + 1],					// bit length count table
		f,									// i repeats every f entries
		i, j,								// loop control
		*p, 								// work pointer
		v[CODE_MAX],						// values in order of bit length
		x[MAX_BITS + 1],					// bit offsets
		*xp,								// pointer
		z;									// entries in current table
	int 	g,								// max code length
		h,									// table level
		k,									// loop control
		l,									// max bits length
		w,									// bits before this table
		y;									// number of dummy entries
	HUFF	r,								// work entry
		*q, 								// current table entry
		*u[MAX_BITS];						// table stack
	
	memset(c, 0, sizeof(c));					// clear table to nulls
	
	for (p = b, i = n; i--;)					// loop thru table to generate
		c[*p++]++;								// ..counts for each bit length
	
	if (c[0] == n)								// q. all zero lengths?
	{
		*t = 0; 								// a. yes .. clear result..
		*m = 0; 								// ..pointer and count
		return(FALSE);							// ..and return all ok
	}
	
	// find min and max code lengths
	for (l = *m, j = 1; j <= MAX_BITS; j++) 	// loop to find minimum code len
	{
		if (c[j])								// q. find the min code length?
			break;								// a. yes .. exit the loop
	}
	
	k = j;										// save minimum code length
	
	if ((UINT) l < j)							// q. minimum greater than lookup?
		l = j;									// a. yes .. set up new lookup
	
	for (i = MAX_BITS; i; i--)					// loop to find max code length
	{
		if (c[i])								// q. find a used entry?
			break;								// a. yes .. exit loop
	}
	
	g = i;										// save maximum code length
	
	if ((UINT) l > i)							// q. lookup len greater than max?
		l = i;									// a. yes .. set up new look len
	
	*m = l; 									// return new lookup to caller
	
	
	for (y = 1 << j; j < i; j++, y <<= 1)		// loop to adjust last length codes
	{
		if ((y -= c[j]) < 0)					// q. more codes than bits?
			return(TRUE);						// a. yes .. return w/error flag
	}
	
	if ((y -= c[i]) < 0)						// q. more than max entry's count?
		return(TRUE);							// a. yes .. return w/error flag
	
	c[i] += y;									// adjust last length code
	
	
	x[1] = j = 0;								// initialize code stack
	for (p = c + 1, xp = &x[2], i = g; i--;)	// loop thru generating offsets
	{
		*xp++ = (j += *p++);					// ..into the entry for each length
	}
	
				
	for (p = b, i = 0; i < n; i++)				// make table of value in order ..
	{
		if ((j = *p++) != 0)					// ..by bit lengths
			v[x[j]++] = i;
	}
	
	
	x[0] = i = 0;								// first Huffman code is all zero
	p = v;										// use the values array
	h = -1; 									// no tables yet, level = -1
	w = -l; 									// bits decoded
	q = u[0] = (HUFF *) 0;						// clear stack and pointer
	z = 0;										// ..and number of entries
	
	for (; k <= g; k++) 						// loop from min to max bit lengths
	{
		for (a = c[k]; a--;)					// process entries at this bit len
		{
			while (k > w + l)					// build up tables to k length
			{
				h++;							// increment table level
				w += l; 						// add current nbr of bits
				
				z = (z = g - w) > (UINT) l		// determine the number of entries
					? l : z;				// ..in the current table
				
				if ((f = 1 << (j = k - w))		// q. k-w bit table contain enough?
					> a + 1)
				{
					f -= a + 1; 				// a. too few codes for k-w bit tbl
					xp = c + k; 				// ..deduct codes from patterns left
					
					while (++j < z) 			// loop to build upto z bits
					{
						if ((f <<= 1) <= *++xp) // q. large enough?
							break;				// a. yes .. to use j bits
						
						f -= *xp;				// else .. remove codes
					}
				}
				
				z = 1 << j; 					// entries for j-bit table
				
				q = (HUFF *) malloc(			// get memory for new table
					(z + 1) * sizeof(HUFF));

				*t = q + 1; 					// link to main list
				*(t = &(q->v.table)) = 0;		// clear ptr, save address
				u[h] = ++q; 					// table starts after link
				
				
				if (h)							// q. is there a last table?
				{
					x[h] = i;					// a. yes .. save pattern
					r.blen = (BYTE) l;			// bits to dump before this table
					r.eb = (BYTE) (16 + j); 	// bits in this table
					r.v.table = q;				// pointer to this table
					j = i >> (w - l);			// index to last table
					u[h-1][j] = r;				// connect to last table
				}
			}
			
			
			r.blen = (BYTE) (k - w);			// set up table entry
			
			if (p >= v + n) 					// q. out of values?
				r.eb = 99;						// a. yes .. set up invalid code
			else if (*p < s)				   // q. need extra bits?
			{
				r.eb = (BYTE) (*p < 256 		// a. yes .. set up extra bits
					? 16 : 15); 			// 256 is end-of-block code
				r.v.code = *p++;				// simple code is just the value
			}
			else
			{
				r.eb = (BYTE) e[*p - s];		// non-simple--look up in lists
				r.v.code = d[*p++ - s];
			}
			
			
			f = 1 << (k - w);					// fill code-like entries with r
			
			for (j = i >> w; j < z; j += f)
				q[j] = r;
			
			
			for (j = 1 << (k - 1); i & j; j >>= 1)	// backwards increment the
				i ^= j; 							// ..k-bit code i
			
			i ^= j;
			
			
			while ((i & ((1 << w) - 1)) != x[h])
			{
				h--;							// decrement table level
				w -= l; 						// adjust bits before this table
			}
		}
	}
	
	return(FALSE);								// return FALSE, everything ok
}
//---------------------------------------------------------------------------
//
//  inf_free() -- free malloc'd Huffman tables
//
//---------------------------------------------------------------------------
void KZipCodec::inf_free(HUFF *t)				// base table to free
{
	HUFF   *p;									// work pointer
	
	while (t)									// loop thru freeing memory
	{
		p = (--t)->v.table; 					// get next table address
		free(t);								// free current table
		t = p;									// establish new base pointer
	}
}
//---------------------------------------------------------------------------
//
//  inf_codes() -- inflate the codes using the Huffman trees
//
//---------------------------------------------------------------------------
int KZipCodec::inf_codes(
				  HUFF *tl,					// literal table
				  HUFF *td, 				// distance table
				  int  bl,					// literal bit length
				  int  bd)					// distance bit length
{
	UINT	c;									// current retrieved code
	UINT	e;									// extra bits
	UINT	n, d;								// length and distance
	BYTE   *p;									// work pointer
	HUFF   *t;									// current Huffman tree
	
	for (;;)									// loop till end of block
	{
		c = lookat_code(bl);					// get some bits
		
		if ((e = (t = tl + c)->eb) > 16)		// q. in this table?
		{
			do
			{									// a. no .. loop reading codes
				if (e == 99)					// q. invalid entry?
					return(TRUE);				// a. yes .. return an error
				
				get_code(t->blen);				// read some bits
				e -= 16;						// nbr of bits to get
				c = lookat_code(e); 			// get some bits
			}
			while ((e = (t = t->v.table + c)->eb) > 16);
		}
		
		get_code(t->blen);						// read some processed bits
		
		if (e == 16)							// q. literal code?
			store_char(t->v.code);				// a. yes .. output code
		else
		{
			if (e == 15)						// q. end of block?
				return(FALSE);					// a. yes .. return all ok
			
			n = get_code(e) + t->v.code;		// get length code
			c = lookat_code(bd);				// get some bits
			
			if ((e = (t = td + c)->eb) > 16)	// q. in this table?
			{
				do
				{							// a. no .. loop thru
					if (e == 99)				// q. invalid entry?
						return(TRUE);			// a. yes .. just return
					
					get_code(t->blen);			// read some bits
					e -= 16;					// number of bits to get
					c = lookat_code(e); 		// get some bits
				}
				while ((e = (t = t->v.table + c)->eb) > 16);
			}
			
			get_code(t->blen);					// read some processed bits
			d = t->v.code + get_code(e);		// get distance value
			
			if ((sbp - sb) >= (int)d)				// q. backward wrap?
				p = sbp - d;					// a. no .. just back up a bit
			else
				p = sb_size - d + sbp;			// else .. find at end of buffer
			
			while (n--) 						// copy previously outputed
			{								// ..strings from sliding buffer
				store_char(*p); 				// put out each character
				
				if (++p >= sbe) 				// q. hit the end of the buffer?
					p = sb; 					// a. yes .. back to beginning
			}
		}
	}
}
//---------------------------------------------------------------------------
//
//  inf_fixed() -- inflate a fixed Huffman code block
//
//---------------------------------------------------------------------------
int KZipCodec::inf_fixed()
{
	UINT	i,									// loop control
		bl = 7, 								// bit length for literal codes
		bd = 5, 								// ..and distance codes
		l[288]; 								// length list
	HUFF   *tl, *td;							// literal and distance trees
	
	for (i = 0; i < 144; i++)					// set up literal table
		l[i] = 8;
	
	for (; i < 256; i++)						// ..fixing up ..
		l[i] = 9;
	
	for (; i < 280; i++)						// ..all the
		l[i] = 7;
	
	for (; i < 288; i++)						// ..entries
		l[i] = 8;
	
	if (inf_build_tree(l, 288, 257, cll,		// q. build literal table ok?
		cle, &tl, &bl))
		return(TRUE);							// a. no .. return with error
	
	for (i = 0; i < 30; i++)					// set up the distance list
		l[i] = 5;								// ..to 5 bits
	
	if (inf_build_tree(l, 30, 0, cdo, cde,		// q. build distance table ok?
		&td, &bd))
		return(TRUE);							// a. no .. return with error
	
	if (inf_codes(tl, td, bl, bd))				// q. inflate file ok?
		return(TRUE);							// a. no .. return with error
	
	inf_free(tl);								// free literal trees
	inf_free(td);								// ..and distance trees
	return(FALSE);								// ..and return all ok
}
//---------------------------------------------------------------------------
//
//  inf_dynamic() -- inflate a dynamic Huffman code block
//
//---------------------------------------------------------------------------
int KZipCodec::inf_dynamic()
{
	UINT	c,								// code read from input stream
		i, j,								// loop control
		l,									// last length
		n,									// nbr of lengths to get
		bl, bd, 							// literal and distance bit len
		nl, nd, 							// literal and distance codes
		nb, 								// nbr of bit length codes
		ll[286 + 30];						// literal length and dist codes
	HUFF   *tl, *td;							// literal and distance trees
	
	nl = get_code(5) + 257; 					// get nbr literal len codes
	nd = get_code(5) + 1;						// ..and the nbr dist len codes
	nb = get_code(4) + 4;						// ..and nbr of of bit lengths
	
	for (j = 0; j < nb; j++)					// read in bit length code
		ll[bll[j]] = get_code(3);				// set up bit lengths
	
	for (; j < 19; j++) 						// loop thru clearing..
		ll[bll[j]] = 0; 						// ..other lengths
	
	bl = 7; 									// set literal bit length
	
	if (inf_build_tree(ll, 19, 19,				// q. build decoding table for
		0, 0, &tl, &bl))				// ..trees using 7 bit lookup ok?
		return(TRUE);							// a. no .. return with error
	
	
	n = nl + nd;								// number of lengths to get
	
	for (i = l = 0; i < n;) 					// get literal and dist code lengths
	{
		c = lookat_code(bl);					// get some bits
		j = (td = tl + c)->blen;				// get length code from table
		get_code(j);							// use those bits
		j = td->v.code; 						// ..then get code from table
		
		if (j < 16) 							// q. save length?
			ll[i++] = l = j;					// a. yes .. also save last length
		else if (j == 16)					   // q. repeat last length 3 to 6x?
		{
			j = get_code(2) + 3;				// get repeat length code
			
			if (i + j > n)						// q. past end of array?
				return(TRUE);					// a. yes .. return with error
			
			while (j--) 						// else .. loop filling table
				ll[i++] = l;					// ..with last length
		}
		else if (j == 17)					   // q. 3 to 10 zero length codes?
		{
			j = get_code(3) + 3;				// a. yes .. get repeat code
			
			if (i + j > n)						// q. past end of array?
				return(TRUE);					// a. yes .. return with error
			
			while (j--) 						// else .. loop filling table
				ll[i++] = 0;					// ..with zero length
			
			l = 0;								// ..and save new last length
		}
		else								   // else .. j == 18 and
		{									// ..generate 11 to 138 zero codes
			j = get_code(7) + 11;				// get repeat code
			
			if (i + j > n)						// q. past end of array?
				return(TRUE);					// a. yes .. return with error
			
			while (j--) 						// else .. loop filling table
				ll[i++] = 0;					// ..with zero length
			
			l = 0;								// ..and save new last length
		}
	}
	
	inf_free(tl);								// finally, free literal tree
	
	bl = 9; 									// length of literal bit codes
	
	if (inf_build_tree(ll, nl, 257, 			// q. build literal table ok?
		cll, cle, &tl, &bl))
		return(TRUE);							// a. no .. return with error
	
	bd = 6; 									// length of distance bit codes
	
	if (inf_build_tree(ll + nl, nd, 0,			// q. build distance table ok?
		cdo, cde, &td, &bd))
		return(TRUE);							// a. no .. return with error
	
	if (inf_codes(tl, td, bl, bd))				// q. inflate block ok?
		return(TRUE);							// a. no .. return with error
	
	inf_free(tl);								// free literal trees
	inf_free(td);								// ..and distance trees
	return(FALSE);								// then finally, return all ok
}
//---------------------------------------------------------------------------

