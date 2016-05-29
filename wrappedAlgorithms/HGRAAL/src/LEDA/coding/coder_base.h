/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  coder_base.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:44:36 $

#ifndef _LEDA_CODER_BASE_H
#define _LEDA_CODER_BASE_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500394
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/system/basic.h>

#if defined(_MSC_VER) && _MSC_VER < 1300
#define LEDA_COMPRESSION_WA6
#endif

#if !defined(LEDA_STD_HEADERS) && (defined(__DECCXX) || defined (__HP_aCC))
#define LEDA_COMPRESSION_WA8
#define LEDA_COMPRESSION_WA9
#endif

LEDA_BEGIN_NAMESPACE

/// coder_types /////////////////////////////////////////////////////////////////////////////

#include <LEDA/internal/std/limits.h>

// determine 16 bits integers
#undef LEDA_CODER_EXACT_SIZE_INT16
#undef LEDA_UINT16
#undef LEDA_SINT16
#if   (UINT_MAX == 0xFFFFUL)
#define LEDA_UINT16 unsigned int
#define LEDA_SINT16 signed   int
#define LEDA_CODER_EXACT_SIZE_INT16
#elif (ULONG_MAX == 0xFFFFUL)
#define LEDA_UINT16 unsigned long
#define LEDA_SINT16 signed   long
#define LEDA_CODER_EXACT_SIZE_INT16
#elif (USHRT_MAX == 0xFFFFUL)
#define LEDA_UINT16 unsigned short
#define LEDA_SINT16 signed   short
#define LEDA_CODER_EXACT_SIZE_INT16
#elif (USHRT_MAX >= 0xFFFFUL)
#define LEDA_UINT16 unsigned short
#define LEDA_SINT16 signed   short
#elif (UINT_MAX >= 0xFFFFUL)
#define LEDA_UINT16 unsigned int
#define LEDA_SINT16 signed   int
#elif (ULONG_MAX >= 0xFFFFUL)
#define LEDA_UINT16 unsigned long
#define LEDA_SINT16 signed   long
#endif

// determine 32 bits integers
#undef LEDA_CODER_EXACT_SIZE_INT32
#undef LEDA_UINT32
#undef LEDA_SINT32
#if   (UINT_MAX == 0xFFFFFFFFUL)
#define LEDA_UINT32 unsigned int
#define LEDA_SINT32 signed   int
#define LEDA_CODER_EXACT_SIZE_INT32
#elif (ULONG_MAX == 0xFFFFFFFFUL)
#define LEDA_UINT32 unsigned long
#define LEDA_SINT32 signed   long
#define LEDA_CODER_EXACT_SIZE_INT32
#elif (USHRT_MAX == 0xFFFFFFFFUL)
#define LEDA_UINT32 unsigned short
#define LEDA_SINT32 signed   short
#define LEDA_CODER_EXACT_SIZE_INT32
#elif (USHRT_MAX >= 0xFFFFFFFFUL)
#define LEDA_UINT32 unsigned short
#define LEDA_SINT32 signed   short
#elif (UINT_MAX >= 0xFFFFFFFFUL)
#define LEDA_UINT32 unsigned int
#define LEDA_SINT32 signed   int
#elif (ULONG_MAX >= 0xFFFFFFFFUL)
#define LEDA_UINT32 unsigned long
#define LEDA_SINT32 signed   long
#endif

// LEDA_CODER_EXACT_SIZE_INT16 should be defined if sizeof(uint16)==sizeof(sint16)==2
// LEDA_CODER_EXACT_SIZE_INT32 should be defined if sizeof(uint32)==sizeof(sint32)==4

#if !defined(LEDA_UINT32)
#error "no integer type with at least 32 bits on this platform"
#endif

#ifndef LEDA_CODER_TEST_64BIT
class __exportC coder_types {
public:
	typedef unsigned char byte;   //  exactly  8 bits (and unsigned)
	typedef LEDA_UINT16   uint16; // at least 16 bits (and unsigned) (faster if exactly 16)
	typedef LEDA_SINT16   sint16; // at least 16 bits (and signed)
	typedef LEDA_UINT32   uint32; // at least 32 bits (and unsigned) (faster if exactly 32)
	typedef LEDA_SINT32   sint32; // at least 32 bits (and signed)
	typedef uint32        id_type;// at least 32 bits (and unsigned)
};
#else
// use types with non-exact size (for testing purposes)
class __exportC coder_types {
public:
	typedef unsigned char    byte;
	typedef unsigned __int32 uint16;
	typedef signed   __int32 sint16;
	typedef unsigned __int64 uint32;
	typedef signed   __int64 sint32;
	typedef unsigned int     id_type;
};

#undef LEDA_CODER_EXACT_SIZE_INT16
#undef LEDA_CODER_EXACT_SIZE_INT32
#endif

#undef LEDA_UINT16
#undef LEDA_SINT16
#undef LEDA_UINT32
#undef LEDA_SINT32

/// coder_base //////////////////////////////////////////////////////////////////////////////

class __exportC memory_streambuf;

#ifndef LEDA_COMPRESSION_WA8
#define LC_READ_EOF(c) c
#define LC_WRITE_EOF(c) c
#else
#define LC_READ_EOF(c) check_eof(c, get_src_stream())
#define LC_WRITE_EOF(c) check_eof(c, get_tgt_stream())
#endif

class __exportC coder_base : public coder_types {
public:
	enum { StreamBlocked = -2 };

#ifndef __DECCXX
	enum { binary = ios::binary };
#else
	enum { binary = 0 };
#endif
	enum { bin_in = ios::in | binary, bin_out = ios::out | binary };

public:
	coder_base(streambuf* src_stream = 0, streambuf* tgt_stream = 0, bool own_streams = false);
	coder_base(const char* src_file_name, const char* tgt_file_name);
	virtual ~coder_base();

	void encode();
	void decode();

	uint32 encode_memory_chunk(const char* in_buf, uint32 in_len, char* out_buf, uint32 out_len); // returns #used bytes in out_buf
	uint32 decode_memory_chunk(const char* in_buf, uint32 in_len, char* out_buf, uint32 out_len); // dito

	streambuf* get_src_stream() const { return pSrcStream; }
	void set_src_stream(streambuf* src_stream, bool own_stream = false);
	void set_src_file(const char* file_name, ios::openmode mode = ios::openmode(ios::in | binary));

	streambuf* get_tgt_stream() const { return pTgtStream; }
	void set_tgt_stream(streambuf* tgt_stream, bool own_Stream = false);
	void set_tgt_file(const char* file_name, ios::openmode mode = ios::openmode(ios::out | ios::trunc | binary));

	void reset(bool keep_parameters = true) { do_reset(keep_parameters ? r_soft : r_hard); }

	bool failed() const { return (State & s_error) != 0; }
	bool finished() const { return (State & s_finished) != 0; }
	bool running() const { return (State & (s_init | s_finished)) == 0; }
	bool encoding() const { return (State & s_encoding) != 0; }
	bool decoding() const { return (State & s_decoding) != 0; }

	virtual string get_description() const { return string(get_class_name()); }

	// coder properties
	virtual bool is_standalone_coder() const { return true; }
	virtual bool is_single_pass_encoder() const { return true; }
	virtual bool is_optimizing_seeks() const { return false; }
	virtual bool needs_seeks_during_coding() const { return false; }
	virtual bool is_secrecy_sensitive() const { return false; }

	static ios::openmode binary_in() { return ios::openmode(bin_in); }
	static ios::openmode binary_out() { return ios::openmode(bin_out); }

	// for backward compatibility ...
	bool fail() const { return failed(); }

protected:
// the following functions should be overriden in a derived class
	virtual const char* get_class_name() const = 0;
	virtual id_type get_id() const = 0; 

	        void do_encode();
	virtual void init_encoding(); // see also write_header_to_tgt_stream below!
	virtual void encode_stream() { encoding_finished(); }
	virtual void encoding_finished(); 

	        void do_decode();
	virtual void init_decoding(); // see also read_header_from_src_stream below!
	virtual void decode_stream() { decoding_finished(); }
	virtual void decoding_finished(); 

	virtual void write_header_to_tgt_stream();  // overide only if needed
		// write global parameters (like block size) (automatically called by coder_base::init_encoding)
	virtual void read_header_from_src_stream(); // overide only if needed
		// read global parameters (automatically called by coder_base::init_decoding)
	virtual uint32 get_header_length(bool& is_exact) const; // overide only if needed
		// returns the length of the header (in byte) (if exact is false, it is only an approximation)

// the following functions could be overriden in a derived class
	enum reset_type { r_hard = 0, r_soft = 1, r_seek = 2, r_seek_without_rewind = 3, r_block = r_seek_without_rewind };
	virtual void do_reset(reset_type t); // overide only if needed
		// hard  = put coder in exactly the same state as the default ctor
		// soft  = put coder in the same state as default ctor but keep parameters like buffer sizes
		// the other values can be treated as soft reset in derived classes (see coder_base.cpp for details)

	virtual void encode_memory(memory_streambuf& in, memory_streambuf& out);
	virtual void decode_memory(memory_streambuf& in, memory_streambuf& out);

protected:
// helpers for derived classes
	enum coder_state { 
		s_init = 1, s_encoding = 2, s_decoding = 4, s_finished = 8, s_error = 16, s_seek = 32,
		s_init_encoding = s_init | s_encoding, s_init_decoding = s_init | s_decoding,
		s_encoding_finished = s_finished | s_encoding, s_decoding_finished = s_finished | s_decoding
	};

	coder_state get_state() const { return State; }
	void set_state(coder_state state) { State = state; }

	bool can_change_parameters() { return !running(); }

	void error(const char* err_msg = 0);

// id and header processing
	// id: everything that is needed in finish_construction_from_stream belongs to the id!
	// header: all other global parameters written to stream (MUST REMAIN CONSTANT while running)
	enum { IdLength = 4 };
	void process_id_in_streams(bool enable) { ProcessIdInStreams = enable; }
	void process_no_id_in_streams() { ProcessIdInStreams = false; }
	bool look_for_id_in_streams() const { return ProcessIdInStreams; }
	virtual void write_id_to_tgt_stream();
	virtual void read_and_check_id_in_src_stream();
	virtual void check_id(id_type id_to_check);
	virtual uint32 get_id_length(bool& is_exact) const; // length of the id (in byte) (if exact is false, it is only an approximation)

	void process_header_in_streams(bool enable) { ProcessHeaderInStreams = enable; }
	void process_no_header_in_streams() { ProcessHeaderInStreams = false; }
	bool look_for_header_in_streams() const { return ProcessHeaderInStreams; }

	virtual uint32 get_prolog_length(bool& is_exact) const; // length of the prolog = id & header

// basic I/O
	int read_byte() { return LC_READ_EOF(pSrcStream->sbumpc()); }
		// read one character, returns the character or EOF (=failure)
	int read_bytes(byte* buffer, int buf_size) { return pSrcStream->sgetn((char*)buffer, buf_size); }
		// read maximum buf_size chars into buffer, returns number of chars read
		// (or a negative number if stream is blocked -> see simple_coder.c)
	void read_uint16(uint16& val);
		// read uint16 (calls error() if it fails)
	void read_uint32(uint32& val);
		// read uint32 (calls error() if it fails)
	void read_id(id_type& val) { if (! read_id(val, pSrcStream)) error(); }
		// read id_type (calls error() if it fails)

#ifndef LEDA_COMPRESSION_WA6
	int write_byte(byte b) { return LC_WRITE_EOF(pTgtStream->sputc((char) b)); }
		// write one character to stream, returns the character or EOF (=failure)
#else
	int write_byte(byte b) { return LC_WRITE_EOF(pTgtStream->sputc((int) b)); }
#endif
	int write_bytes(const byte* buffer, int buf_size) { return pTgtStream->sputn((char*)buffer, buf_size); }
		// write buf_size chars from buffer to stream, returns number of chars written
		// (or a negative number if stream is blocked -> see simple_coder.c)
	void write_uint16(uint16 val);
		// write uint16 (calls error() if it fails)
	void write_uint32(uint32 val);
		// write uint32 (calls error() if it fails)
	void write_id(id_type val) { if (! write_id(val, pTgtStream)) error(); }
		// write id_type (calls error() if it fails)

	void rewind_src_stream() { if (pSrcStream->pubseekpos(0) != streampos(0)) error(); }
	void rewind_tgt_stream() { if (pTgtStream->pubseekpos(0) != streampos(0)) error(); }

	static bool read_id(id_type& val, streambuf* stream);
	static bool write_id(id_type val, streambuf* stream);

// support for autodecoding
	static coder_base* construct_from_stream(streambuf* stream, streambuf* additional_data = 0);
	virtual void finish_construction_from_stream(id_type id_in_stream, streambuf* stream, streambuf* additional_data);
		// additional_data: data needed for the construction which is not stored in stream
		// (this could be the key for deciphering)

// support for seek
	virtual streampos encode_seek(streampos current_pos, streampos new_pos);
		// moves coder from current_pos to new_pos in the src stream
		// (So far only forward seeks are supported, i.e. new_pos must be
		// greater than current_pos. So current_pos is the position behind
		// the character that has been read last, the characters that are
		// skipped are treated as zeros, which is standard.)
	virtual streampos decode_seek(streampos current_pos, streampos new_pos);
		// moves coder from current_pos to new_pos in the tgt stream
		// (If new_pos is -1, we seek till the end of the file)

	virtual streambuf* change_src_stream_temporarily(streambuf* src_stream);
		// needed by encode_seek (returns the old src stream)
	virtual streambuf* change_tgt_stream_temporarily(streambuf* tgt_stream);
		// needed by decode_seek (returns the old tgt stream)

	streamoff get_pos_behind_header() const { return PosBehindHeader; }

	streampos get_pos_tgt_stream_relative_to_header() const
	{ return pTgtStream->pubseekoff(0, ios::cur, ios::out) - PosBehindHeader; }
	streampos seek_pos_tgt_stream_relative_to_header(streampos pos)
	{ return pTgtStream->pubseekpos(pos + PosBehindHeader, ios::out) - PosBehindHeader; }
	streampos seek_off_tgt_stream_relative_to_header(streamoff off, ios::seekdir way)
	{ return pTgtStream->pubseekoff(off, way, ios::out) - PosBehindHeader; }
	streampos seek_end_tgt_stream_relative_to_header()
	{ return pTgtStream->pubseekoff(0, ios::end, ios::out) - PosBehindHeader; }
		// for encoding

	streampos get_pos_src_stream_relative_to_header() const
	{ return pSrcStream->pubseekoff(0, ios::cur, ios::in) - PosBehindHeader; }
	streampos seek_pos_src_stream_relative_to_header(streampos pos)
	{ return pSrcStream->pubseekpos(pos + PosBehindHeader, ios::in) - PosBehindHeader; }
	streampos seek_off_src_stream_relative_to_header(streamoff off, ios::seekdir way)
	{ return pSrcStream->pubseekoff(off, way, ios::in) - PosBehindHeader; }
	streampos seek_end_src_stream_relative_to_header()
	{ return pSrcStream->pubseekoff(0, ios::end, ios::in) - PosBehindHeader; }
		// for decoding

	friend class __exportC coder_factory;
	friend class __exportC coder_pipe;
	friend class __exportC block_coder;
	friend class __exportC AutoDecoder; // for change_src/tgt_stream_temporarily
	friend class __exportC coder_strbuf_adapter; // for reset
	friend class __exportC coder_strbuf_adapter_with_seek; // for seek

#ifdef LEDA_COMPRESSION_WA8
	static int check_eof(int c, streambuf* stream);
#endif

public:
	inline static void swap_bytes_uint16(uint16& i);
	inline static void swap_bytes_uint32(uint32& i);

	static bool read_uint16(streambuf* stream, uint16& val);
	static bool read_uint32(streambuf* stream, uint32& val);
	static bool write_uint16(streambuf* stream, uint16 val);
	static bool write_uint32(streambuf* stream, uint32 val);

private:
	coder_base(const coder_base&); // forbidden
	const coder_base& operator=(const coder_base&); // forbidden

private:
	coder_state State;
	streambuf*  pSrcStream;
	streambuf*  pTgtStream;
	bool		OwnSrcStream;
	bool		OwnTgtStream;
	bool		ProcessIdInStreams;
	bool		ProcessHeaderInStreams;
	streamoff   PosBehindHeader;
};

inline 
void coder_base::swap_bytes_uint16(uint16& i)
{
	uint16 b1 = (i << 8) & 0xFF00; // take byte 0 and shift it into byte 1
	uint16 b0 = (i >> 8) & 0x00FF; // take byte 1 and shift it into byte 0
	i = b0 | b1;
}

inline 
void coder_base::swap_bytes_uint32(uint32& i)
{
	uint32 b3 = (i << 24) & 0xFF000000;
	uint32 b2 = (i << 8)  & 0x00FF0000;
	uint32 b1 = (i >> 8)  & 0x0000FF00;
	uint32 b0 = (i >> 24) & 0x000000FF;

	i = b0 | b1 | b2 | b3;
}

/// coder_base_debug ////////////////////////////////////////////////////////////////////////

//#define LEDA_CODER_DEBUG
#ifdef LEDA_CODER_DEBUG

class __exportC coder_base_debug : public coder_base {
public:
	typedef coder_base base;

public:
	coder_base_debug(streambuf* src_stream = 0, streambuf* tgt_stream = 0, bool own_streams = false);
	coder_base_debug(const char* src_file_name, const char* tgt_file_name);

	static coder_base_debug* (streambuf* stream, streambuf* additional_data = 0);

protected:
	int read_byte();
	int read_bytes(byte* buffer, int buf_size);

	int write_byte(byte b);
	int write_bytes(byte* buffer, int buf_size);

private:
	int log_read(int v);
	int log_write(int v);

private:
	uint32      ReadLogCount;
	int         ReadLog[2048];
	uint32		WriteLogCount;
	int         WriteLog[2048];
};

#define coder_base coder_base_debug

#endif // ifdef LEDA_CODER_DEBUG

#if LEDA_ROOT_INCL_ID == 500394
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif

LEDA_END_NAMESPACE

#endif
