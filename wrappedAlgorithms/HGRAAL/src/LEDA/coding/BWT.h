/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  BWT.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:44:35 $

#ifndef _LEDA_BWT_H
#define _LEDA_BWT_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500021
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/coding/coder_base.h>

LEDA_BEGIN_NAMESPACE

/// BWTCoder ////////////////////////////////////////////////////////////////////////////////

/*{\Manpage {BWTCoder} {} {Burrows-Wheeler Transform} {C}}*/

class __exportC BWTCoder : public coder_base {
/*{\Mdefinition
The class |\Mname| applies the Burrows-Wheeler transform
\cite{BWTrafo} (in encoding mode) or its reversal (in decoding mode) to the
input stream. To be more precise, in encoding mode the input stream is 
partitioned in blocks and the transformation is applied to each block.
The size of the blocks, i.e.~the number of characters per block, can be 
specified by the user.

This transformation does not compress the input but it can be used as a 
preprocessing step for other coders. Applying |\Mname| first often improves
the overall compression rates.
The following combinations yield very good compression results:
\begin{itemize}
\item [[CoderPipe3<BWTCoder, MTFCoder, A0Coder>]]
\item [[CoderPipe4<BWTCoder, MTFCoder, RLE0Coder, A0Coder>]]
\end{itemize}
}*/
public:
	typedef coder_base base;
	enum { ID = 0x434c04 };
	enum { DefaultBlockSize = 1000 * 1024 };
public:
/*{\Mcreation}*/

	BWTCoder(streambuf* src_stream = 0, streambuf* tgt_stream = 0, bool own_streams = false, 
		     uint32 block_size = DefaultBlockSize);
	/*{\Mcreate creates an instance |\Mvar| which uses the given source 
	and target streams. If |own_streams| is set, then |\Mvar| is 
	responsible for the destruction of the streams, otherwise the pointers 
	|src_stream| and |tgt_stream| must be valid during the life-time of 
	|\Mvar|. The block size can be controlled via |block_size|.}*/

	BWTCoder(const char* src_file_name, const char* tgt_file_name, 
		     uint32 block_size = DefaultBlockSize);
	/*{\Mcreate creates an instance |\Mvar| which uses file-streams for 
	input and output. The block size can be controlled via |block_size|.}*/

	~BWTCoder();

/*{\Moperations}*/
/*{\Mtext \headerline{Standard Operations}}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	void encode();
*/
	/*{\Mop encodes the source stream and writes the output to the target
	stream.}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	void decode();
*/
	/*{\Mop decodes the source stream and writes the output to the target
	stream.}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	uint32 encode_memory_chunk(const char* in_buf, uint32 in_len, char* out_buf, uint32 out_len);
*/
	/*{\Mop encodes the memory chunk starting at |in_buf| with size |in_len| 
	into the buffer starting at |out_buf| with size |out_len|. 
	The function returns actual length of the encoded chunk which may be 
	smaller than |out_len|. If the output buffer is too small for the encoded
	data the failure flag will be set (see below).}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	uint32 decode_memory_chunk(const char* in_buf, uint32 in_len, char* out_buf, uint32 out_len);
*/
	/*{\Mop decodes a memory chunk. The meaning of the parameters and the 
	return value is the same as in the previous function.}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	streambuf* get_src_stream() const;
*/
	/*{\Mop returns the current source stream.}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	void set_src_stream(streambuf* src_stream, bool own_stream = false);
*/
	/*{\Mop sets the source stream (cf.~constructor).}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	void set_src_file(const char* file_name);
*/
	/*{\Mop sets a file as source stream.}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	streambuf* get_tgt_stream() const;
*/
	/*{\Mop returns the current target stream.}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	void set_tgt_stream(streambuf* tgt_stream, bool own_Stream = false);
*/
	/*{\Mop sets the target stream (cf.~constructor).}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	void set_tgt_file(const char* file_name);
*/
	/*{\Mop sets a file as target stream.}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	void reset(bool keep_block_size = true); 
*/
	/*{\Mop puts |\Mvar| in the same state as the default constructor.
	If |keep_block_size| is false the block size is set to the default
	value.}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	bool failed() const;
*/
	/*{\Mop returns |true| if an error occured.}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	bool finished() const;
*/
	/*{\Mop returns |true| if the coding is finished.}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	virtual string get_description() const;
*/
	/*{\Mop provides a description for |\Mvar|.}*/

/*{\Mtext \headerline{Additional Operations}}*/

	uint32 get_block_size() const { return BlockSizeEnc; }
	/*{\Mop returns the current block size (for encoding).}*/
	void   set_block_size(uint32 block_size);
	/*{\Mop sets the block size.}*/

protected:
	virtual const char* get_class_name() const { return "BWTCoder"; }
	virtual id_type get_id() const { return ID; }

	virtual void init_encoding();
	virtual void encode_stream();

	virtual void init_decoding();
	virtual void decode_stream();

	virtual void do_reset(reset_type t);

private:
	void free_buffers();

	void encode_block();
	bool decode_block(); // returns true iff all bytes could be written
	bool finish_writing_decoded_block(); // dito

private:
	byte*   Buffer;
	sint32* SuffixArray;
	uint32* PosArray;
	uint32  BlockSizeEnc, BlockSizeDec;
	uint32  ByteCount;
	uint32  NextBlockSize;
	uint32  NextPosEob;
	uint32  NextPos;
	uint32  SymbolCount[256];
};

/*{\Mimplementation
Our implementation is based on code for suffix sorting 
by P.~Ferragina and G.~Manzini.}*/

#if LEDA_ROOT_INCL_ID == 500021
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif

LEDA_END_NAMESPACE

#endif
