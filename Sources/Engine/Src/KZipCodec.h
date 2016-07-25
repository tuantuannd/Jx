//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KZipCodec.h
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KZipCodec_H
#define KZipCodec_H
//---------------------------------------------------------------------------
#pragma pack(push)
#pragma pack(1)

// local file header
struct lf
{
	UINT	lf_sig;			// signature (0x04034b50)
	WORD	lf_extract;		// vers needed to extract
	WORD	lf_flag;        // general purpose flag
	WORD	lf_cm;          // compression method
	WORD	lf_time;        // file time
	WORD	lf_date;        // ..and file date
	UINT	lf_crc;			// CRC-32 for file
	UINT	lf_csize;       // compressed size
	UINT	lf_size;        // uncompressed size
	WORD	lf_fn_len;		// file name length
	WORD	lf_ef_len;      // extra field length
};

// data descriptor
struct dd
{
	UINT	dd_crc;			// CRC-32 for file
	UINT	dd_csize;       // compressed size
	UINT	dd_size;        // uncompressed size
};

// file header
struct fh
{
	UINT	fh_sig;			// signature (0x02014b50)
	WORD	fh_made;		// version made by
	WORD	fh_extract;     // vers needed to extract
	WORD	fh_flag;        // general purpose flag
	WORD	fh_cm;          // compression method
	WORD	fh_time;        // file time
	WORD	fh_date;        // ..and file date
	UINT	fh_crc;			// CRC-32 for file
	UINT	fh_csize;       // compressed size
	UINT	fh_size;        // uncompressed size
	WORD	fh_fn_len;		// file name length
	WORD	fh_ef_len;      // extra field length
	WORD	fh_fc_len;      // file comment length
	WORD	fh_disk;        // disk number
	WORD	fh_attr;        // internal file attrib
	UINT	fh_eattr;		// external file attrib
	UINT	fh_offset;      // offset of local header
};

// end of central dir record
struct ed
{
	UINT	ed_sig;			// signature (0x06054b50)
	WORD	ed_disk;		// this disk number
	WORD	ed_cdisk;       // disk w/central dir
	WORD	ed_current;     // current disk's dir entries
	WORD	ed_total;       // total dir entries
	UINT	ed_size;		// size of central dir
	UINT	ed_offset;      // offset of central dir
	WORD	ed_zc_len;		// zip file comment length
};

typedef struct lf LF;		// local file header
typedef struct fh FH;		// file header
typedef struct dd DD;		// data descriptor 
typedef struct ed ED;		// end of central dir record

// Huffman Tree Structure
struct  huffman_tree                    // Huffman trees
{
	BYTE eb;                            // extra bits
	BYTE blen;                          // bit length
	union
	{
		int	   code;                    // literal, len or distance
		struct huffman_tree *table;		// chain pointer
	} v;
};
typedef struct huffman_tree HUFF;		// structure shorthand

#pragma pack(pop)
//---------------------------------------------------------------------------
class ENGINE_API KZipCodec
{
public:
	BOOL	Decode(PVOID pInBuf, PVOID pOutBuf, LF* pLf);

public:
	KZipCodec();

private:
	UINT	bhold;						// bits hold area
	long	rsize;						// remaining size to process
	int		e_count;					// end of data counter/flag
	BYTE	bsize;						// bits left to process
	UINT	sb_size;					// sliding buffer size
	PBYTE	sb;							// sliding buffer, ptr, and end
	PBYTE	sbp;						// sliding buffer, ptr, and end
	PBYTE	sbe;						// sliding buffer, ptr, and end
	LF		m_Lf;						// local file header
	PBYTE	m_pZipIn;					// input buffer pointer
	PBYTE	m_pZipOut;					// output buffer pointer

private:
	void	extract_init();
	void	extract_file();
	void	extract_exit();
	void	extract_flush();
	void	extract_copy(UINT len);
	void	store_char(char c);
	UINT	lookat_code(int bits);
	UINT	get_code(int bits);
	void	extract_stored();
	void	extract_inflate();
	int 	inf_fixed();
	int 	inf_dynamic();
	int 	inf_build_tree(
				UINT *b,	// code lengths in bits
				UINT n,		// number of codes
				UINT s,		// number of simple codes
				UINT *d,	// base values for non-simple
				UINT *e,	// list of extra bits
				HUFF **t,	// resulting table
				UINT *m);	// maximum lookup bits
	int     inf_codes(
				HUFF *tl,	// literal table
				HUFF *td,	// distance table
				int  bl,	// literal bit length
				int  bd);	// distance bit length
	void	inf_free(HUFF *t);
};
//---------------------------------------------------------------------------
#endif