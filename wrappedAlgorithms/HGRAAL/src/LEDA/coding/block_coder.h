/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  block_coder.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.4 $  $Date: 2005/04/14 10:44:36 $

#ifndef _LEDA_BLOCK_CODER_H
#define _LEDA_BLOCK_CODER_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500015
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/coding/coder_base.h>
#include <LEDA/coding/memory_streambuf.h>

LEDA_BEGIN_NAMESPACE

/// block_table /////////////////////////////////////////////////////////////////////////////

namespace block_coder_helpers {

	class block_table_cache : public coder_types {
	public:
		enum { MaxNumEntries = 1024 };
	public:
		block_table_cache() { clear(); }

		void clear(uint32 idx_first = 0) { IdxOfFirstEntry = idx_first; NumberOfEntries = 0; }

		void append_entry(uint32 entry) { Entries[NumberOfEntries++] = entry; }
		uint32 get_entry(uint32 idx) const { return Entries[idx - IdxOfFirstEntry]; }

		bool is_empty() const { return NumberOfEntries == 0; }
		bool is_full() const { return NumberOfEntries == MaxNumEntries; }
		bool contains_index(uint32 idx) { return idx >= min_index() && idx < next_index(); }

		bool write_to_stream(streambuf* stream);
		bool read_from_stream(streambuf* stream, uint32 size);

		uint32 min_index() const { return IdxOfFirstEntry; }
		uint32 next_index() const { return IdxOfFirstEntry + NumberOfEntries; }

	private:
		uint16 NumberOfEntries;
		uint32 Entries[MaxNumEntries];
		uint32 IdxOfFirstEntry;
	};

	class block_table : public coder_types {
	public:
		block_table() { clear(); }

		void clear() 
		{ Size = 0; Cache.clear(); pStream = 0; StartInStream = 0; TmpFileName = ""; }

		uint32 get_size() const { return Size; }

		bool append_entry(uint32 entry); // retval indicates success
		uint32 get_entry(uint32 idx, bool cache_successors = false);

		bool write_to_stream(streambuf* stream);
		bool read_from_stream(streambuf* stream, uint32 size);

	private:
		uint32            Size;
		block_table_cache Cache;

		string            TmpFileName;
		streambuf*        pStream;
		streampos         StartInStream;
	};
}

/// block_coder /////////////////////////////////////////////////////////////////////////////

class __exportC block_coder : public coder_base {
public:
	typedef coder_base base;
	enum { ID = 0x434c0c };
	enum { DefaultBlockSize = 100 * 1024 };

public:
	block_coder(streambuf* src_stream = 0, streambuf* tgt_stream = 0, bool own_streams = false,
		        uint32 block_size = DefaultBlockSize);

	block_coder(const char* src_file_name, const char* tgt_file_name,
		        uint32 block_size = DefaultBlockSize);

	virtual ~block_coder();

	virtual string get_description() const;

	virtual bool is_standalone_coder() const { return false; }
	virtual bool is_optimizing_seeks() const { return true; }
	virtual bool needs_seeks_during_coding() const { return true; }

	uint32 get_block_size() const { return BlockSize; }
	void   set_block_size(uint32 block_size);

protected:
	coder_base* get_coder() const { return pCoder; }
	void        set_coder(coder_base* coder, bool own_coder = false);

protected:
	virtual const char* get_class_name() const { return "block_coder"; }
	virtual id_type get_id() const { return ID; }

	virtual void write_id_to_tgt_stream();
	virtual void read_and_check_id_in_src_stream();
	virtual uint32 get_id_length(bool& is_exact) const;
	virtual void finish_construction_from_stream(id_type id_in_stream, streambuf* stream, streambuf* additional_data);

	virtual void write_header_to_tgt_stream();
	virtual void read_header_from_src_stream();
	virtual uint32 get_header_length(bool& is_exact) const;

	virtual void init_encoding();
	virtual void encode_stream();

	virtual void init_decoding();
	virtual void decode_stream();

	virtual void do_reset(reset_type t);

	virtual streambuf* change_tgt_stream_temporarily(streambuf* tgt_stream);

	virtual streampos decode_seek(streampos current_pos, streampos new_pos);

protected:
	streampos get_pos_src_stream() const
	{ return get_pos_src_stream_relative_to_header(); }

	streampos seek_pos_src_stream(streampos pos)
	{ return seek_pos_src_stream_relative_to_header(pos); }

	streampos seek_off_src_stream(streamoff off, ios::seekdir way)
	{ return seek_off_src_stream_relative_to_header(off, way); }

	streampos seek_end_src_stream()
	{ return seek_end_src_stream_relative_to_header(); }

	streampos get_pos_tgt_stream() const
	{ return get_pos_tgt_stream_relative_to_header(); }

private:
	void encode_memory_buffer();

	bool fetch_next_encoded_block();
	bool fetch_encoded_block(uint32 idx_blk); // for seek

private:
	typedef block_coder_helpers::block_table block_table;

	uint32				BlockSize;
	coder_base*			pCoder;
	bool				OwnCoder;
	memory_streambuf	MemoryBuf;
	block_table			BlockTable;
	uint32				SizeOriginalStream;

	uint32				IdxCurrentBlock;    // for decoding
};


/// BlockCoder //////////////////////////////////////////////////////////////////////////////

/*{\Manpage {BlockCoder} {Coder} {Block Coder} {C}}*/

template <class Coder>
class BlockCoder : public block_coder {
/*{\Mdefinition
An instance of type |\Mname| encodes a source stream as follows:
It divides the stream into equally sized blocks and encodes each block 
seperately with an instance of type |Coder|. If you later want to decode 
only parts of the encoded data then you do not have to decode the entire 
data but only the appropriate blocks, which is usually much faster.
(The block size can be specified by the user; please note that the
main memory of your computer has to be large enough to hold one block.)\\
The class |\Mtype| has been designed to speed up the seek operations of 
the class |decoding_ifstream<Coder>| (see Section~\ref{decoding_ifstream}). 
In order to gain the speed up simply replace the template parameter |Coder|
in |decoding_ifstream| by |\Mname| (see also the example below).

There is an important precondition for |\Mtype|: 
In encoding mode the target stream must support seek operations which query the
current position (i.e.~|seek(0, ios::cur, ios::out)|).
In decoding mode the source stream must be capable of random seek operations.\\
These conditions are surely fullfilled if the respective stream is a file. 
Moreover, if |\Mtype| is used within a coder pipe the pipe will make sure that
it operates properly. However, this configuration only makes sense if all 
coders in the pipe optimize seek operations. Then the whole pipe will offer 
fast seek operations.
}*/
public:
	typedef block_coder base;

public:
/*{\Mcreation}*/

	BlockCoder(streambuf* src_stream = 0, streambuf* tgt_stream = 0, bool own_streams = false,
		       uint32 block_size = DefaultBlockSize)
	 : base(src_stream, tgt_stream, own_streams, block_size) { init(); }
	/*{\Mcreate creates an instance |\Mvar| which uses the given source 
	and target streams. If |own_streams| is set, then |\Mvar| is 
	responsible for the destruction of the streams, otherwise the pointers 
	|src_stream| and |tgt_stream| must be valid during the life-time of 
	|\Mvar|. The initial block size is |block_size|.}*/

	BlockCoder(const char* src_file_name, const char* tgt_file_name,
		       uint32 block_size = DefaultBlockSize)
	 : base(src_file_name, tgt_file_name, block_size) { init(); }
	/*{\Mcreate creates an instance |\Mvar| which uses file-streams for 
	input and output. The block size is set to |block_size|.}*/

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
	If |keep_block_size| is false, the block size is set to the default
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

/*{\Moptions nextwarning=no }*/
/*	inherited:
	uint32 get_block_size() const { return BlockSize; }
*/
	/*{\Mop returns the current block size.}*/
/*{\Moptions nextwarning=no }*/
/*	inherited:
	void   set_block_size(uint32 block_size);
*/
	/*{\Mop sets the block size.}*/

	Coder* get_coder() const { return static_cast<Coder*>(base::get_coder()); }
	/*{\Mop returns the instance of |Coder| which is actually used for encoding
	and decoding.}*/

	void set_coder(Coder* coder, bool own_coder = false) { base::set_coder(coder, own_coder); }

private:
	void init() { set_coder(new Coder(), true); }
};

/*{\Mexample
The following example shows how |\Mtype| can be used to speed up seek 
operations. We encode a file of about 3.85 MB, once with |A0Coder| and once
with |BlockCoder<A0Coder>|. We feed each encoded file into an 
|autodecoding_ifstream| and perform random seeks.
The file encoded with |\Mtype| is slightly longer but seek 
operations become much faster.

\begin{verbatim}
#include <LEDA/coding/compress.h>
#include <LEDA/core/random_source.h>
#include <LEDA/system/file.h>
#include <LEDA/system/timer.h>
using namespace LEDA;

void test_random_seek(istream& is)
{
  const int buffer_sz = 100; char buffer[buffer_sz];
  is.seekg(0, ios::end); streamoff size = (streamoff) is.tellg(); 
    // yields the size of the original file

  random_source rs;
  rs.set_seed(0); rs.set_range(0, size - buffer_sz - 1);

  timer t("seek"); t.start();
  for (int i = 0; i < 100; ++i) {
    is.seekg(rs()); is.read(buffer, buffer_sz); // random seek
  }
  t.stop();
}

int main()
{
  typedef A0Coder Coder;
  string in_file = "D:\\ctemp\\bible.txt"; // 3,85MB
  string out_file1 = tmp_file_name(), out_file2 = tmp_file_name();

  Coder c1(in_file, out_file1);
  c1.encode(); c1.reset();
  cout << "encoded size: " << size_of_file(out_file1) << endl;
  autodecoding_ifstream is1(out_file1);
  test_random_seek(is1); is1.close();
  // output on my machine: enc. sz: 2201723 / Timer(seek): 51.36 s

  BlockCoder<Coder> c2(in_file, out_file2);
  c2.encode(); c2.reset();
  cout << "encoded size: " << size_of_file(out_file2) << endl;
  autodecoding_ifstream is2(out_file2);
  test_random_seek(is2); is2.close();
  // output on my machine: enc. sz: 2207892 / Timer(seek): 1.91 s

  delete_file(out_file1); delete_file(out_file2); return 0;
}
\end{verbatim}
}*/

#if LEDA_ROOT_INCL_ID == 500015
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif

LEDA_END_NAMESPACE

#endif
