/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  deflate.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:44:38 $

#ifndef _LEDA_DEFLATE_H
#define _LEDA_DEFLATE_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500066
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/coding/coder_base.h>

LEDA_BEGIN_NAMESPACE

/// DeflateCoder ////////////////////////////////////////////////////////////////////////////

/*{\Manpage {DeflateCoder} {} {Deflation/Inflation Coder} {C}}*/

class __exportC DeflateCoder : public coder_base {
/*{\Mdefinition
The class |\Mname| encapsulates the algorithms ``deflation'' and 
``inflation'' from the zlib-library by J.-L.~Gilly and M.~Adler 
(see {\tt www.zlib.org}).

Both algorithms use two buffers (for encoded and decoded data). The user
can control the size of each buffer.
In addition he can fine-tune some parameters of the algorithm, which gives 
him a trade-off between compression rate and speed.\\
This coder is fast, gives good compression rates and has moderate memory
requirements.
}*/
public:
	typedef coder_base base;
	enum { ID = 0x434c05 };
	enum { DefaultBufferSize = 8 * 1024 };

public:
/*{\Mcreation}*/

	DeflateCoder(streambuf* src_stream = 0, streambuf* tgt_stream = 0, bool own_streams = false, 
	             uint32 enc_buffer_sz = DefaultBufferSize, uint32 dec_buffer_sz = DefaultBufferSize);
	/*{\Mcreate creates an instance |\Mvar| which uses the given source 
	and target streams. If |own_streams| is set, then |\Mvar| is 
	responsible for the destruction of the streams, otherwise the pointers 
	|src_stream| and |tgt_stream| must be valid during the life-time of 
	|\Mvar|.
	(In addition the sizes of the two buffers may be specified.)}*/

	DeflateCoder(const char* src_file_name, const char* tgt_file_name,
	             uint32 enc_buffer_sz = DefaultBufferSize, uint32 dec_buffer_sz = DefaultBufferSize);
	/*{\Mcreate creates an instance |\Mvar| which uses file-streams for 
	input and output.
	(In addition the sizes of the two buffers may be specified.)}*/

	~DeflateCoder();

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
	void reset(bool keep_parameters = true); 
*/
	/*{\Mop puts |\Mvar| in the same state as the default constructor.
	If |keep_parameters| is false the parameters are set to their default
	values.}*/

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

	virtual string get_description() const;
	/*{\Mop provides a description for |\Mvar|.}*/

/*{\Mtext \headerline{Additional Operations}}*/

	int  get_compression_level() const { return CompressionLevel; }
	/*{\Mop returns the current compression level.}*/

	void set_compression_level(int level);
	/*{\Mop sets the compression level (between 0 and 9), whereas 0 means no
	compression, 1 gives the best speed and 9 yields the highest 
	compression.}*/

	int  get_window_size() const { return Log2OfWindowSize; }
	/*{\Mop returns the base two logarithm of the size of the sliding window
	used by the deflation algorithm.}*/

	void set_window_size(int log2_of_sz);
	/*{\Mop sets the window size (|log2_of_sz| must be between 8 and 15).}*/

	int  get_memory_level() const { return MemLevel; }
	/*{\Mop returns the current memory level.}*/

	void set_memory_level(int level);
	/*{\Mop sets the memory level (between 1 and 9).
	The higher this number, the higher the memory consumption.}*/

	int  get_strategy() const { return Strategy; }
	/*{\Mop returns the current strategy for the deflation algorithm.}*/

	void set_strategy(int strategy);
	/*{\Mop sets the compression strategy (between 0 and 3): 0=default, 
	1=optimization for filtered data, i.e. small values with random 
	distribution, 2=Huffman only (no string matching), 3=RLE (for image 
	data).}*/

	void get_memory_consumption(uint32& mem_for_enc, uint32& mem_for_dec);
	/*{\Mop returns the (approximate) memory consumption (in bytes) needed
	for encoding and decoding repectively.}*/

	uint32 get_enc_buffer_size() const { return EncBufferSize; }
	/*{\Mop returns the size of the buffer for encoded data.}*/

	void   set_enc_buffer_size(uint32 buffer_size);
	/*{\Mop sets the size of the buffer for encoded data.}*/

	uint32 get_dec_buffer_size() const { return DecBufferSize; }
	/*{\Mop returns the size of the buffer for decoded data.}*/

	void   set_dec_buffer_size(uint32 buffer_size);
	/*{\Mop sets the size of the buffer for decoded data.}*/

	static const char* get_zlib_version();

protected:
	virtual const char* get_class_name() const { return "DeflateCoder"; }
	virtual id_type get_id() const { return ID; }

	virtual void init_encoding();
	virtual void encode_stream();

	virtual void init_decoding();
	virtual void decode_stream();

	virtual void encode_memory(memory_streambuf& in, memory_streambuf& out);
	virtual void decode_memory(memory_streambuf& in, memory_streambuf& out);

	virtual void do_reset(reset_type t);

private:
	int		CompressionLevel;
	int		Log2OfWindowSize;
	int		MemLevel;
	int		Strategy;
	void*	p_zlib_stream;
	int		State; // Z_NO_FLUSH, Z_FINISH or Z_STREAM_END
	int		EncBufferSize;
	int		DecBufferSize;
	byte*	pEncBuffer;
	byte*	pDecBuffer;
	byte*	pCurrent;
};


#if LEDA_ROOT_INCL_ID == 500066
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif

LEDA_END_NAMESPACE

#endif
