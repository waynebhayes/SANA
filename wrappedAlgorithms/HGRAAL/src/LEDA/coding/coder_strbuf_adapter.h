/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  coder_strbuf_adapter.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.5 $  $Date: 2005/04/14 10:44:37 $

#ifndef _LEDA_CODER_STRBUF_ADAPTER_H
#define _LEDA_CODER_STRBUF_ADAPTER_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500397
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/system/basic.h>
#include <LEDA/coding/coder_base.h>
#include <LEDA/coding/internal/streambuf_workaround.h>

LEDA_BEGIN_NAMESPACE

/// coder_strbuf_adapter ////////////////////////////////////////////////////////////////////

class __exportC coder_strbuf_adapter : public LEDA_STREAMBUF {
public:
	typedef LEDA_STREAMBUF     base;
	typedef coder_base::byte   byte;
	typedef coder_base::uint32 uint32;
	enum { DefaultBufSize = 4, StreamBlocked = coder_base::StreamBlocked };

public:
	coder_strbuf_adapter(coder_base* coder = 0, bool own_coder = false, 
						 streambuf* strbuf = 0, bool own_strbuf = false, 
						 uint32 buf_size = DefaultBufSize);
	virtual ~coder_strbuf_adapter();

	uint32 get_buffer_size() const { return BufferSize; }
	void   set_buffer_size(uint32 buf_size);

	coder_base* get_coder() const { return pCoder; }
	bool        owns_coder() const { return OwnCoder; }
	void        set_coder(coder_base* coder, bool own_coder = false);
	coder_base* release_ownership_of_coder();

	streambuf* get_encoded_stream() const { return pEncodedStream; }
	void set_encoded_stream(streambuf* stream, bool own_stream = false);
	void set_encoded_file(const char* file_name, ios::openmode mode);

	void sputEOF();

	void reset(bool keep_coder_parameters = true) 
	{ do_reset(keep_coder_parameters ? coder_base::r_soft : coder_base::r_hard); }
	// same state as the default ctor, but keeps coder and BufferSize

	bool failed() const { return (State & s_error) != 0; }
	bool finished() const { return (State & s_finished) != 0; }
	bool running() const { return (State & (s_init | s_finished | s_finalized)) == 0; }
	bool encoding() const { return (State & s_encoding) != 0; }
	bool decoding() const { return (State & s_decoding) != 0; }

protected:
	enum state { 
		s_init=1, s_encoding=2, s_decoding=4, s_finished=8, s_finalized=16, s_error=32, 
		s_first_pass = 0, s_later_pass = 64,
		s_init_encoding = s_init | s_encoding, s_init_decoding = s_init | s_decoding,
		s_encoding_finished = s_finished | s_encoding, s_decoding_finished = s_finished | s_decoding,
		s_encoding_first_pass = s_encoding | s_first_pass, s_encoding_later_pass = s_encoding | s_later_pass
	};

	bool later_pass() const { return (State & s_later_pass) != 0; }

protected:
	// the following function should be called in the destructor of a derived class
	// (It has to be called if the virtual function sync() is overwritten!)
	void finalize();

	friend class __exportC coder_pipe;
	virtual void do_reset(coder_base::reset_type t); 

	void error(const char* err_msg = 0);

	virtual streambuf* create_internal_stream(string filename);
	virtual void destroy_internal_stream(streambuf* int_stream, string filename);
	streambuf* get_stream_for_buffering() const { return pStreamForBuffering; }

	virtual void observe_state_change(state new_state) {}
	state get_state() const { return State; }

	// the following functions are inherited from streambuf
	virtual int overflow(int c); // called when put-buffer buffer full  (puts the character to the stream)
	virtual int underflow();     // called when get-buffer buffer empty (gets a character from stream, but doesn' point past it)
	virtual int uflow();		 // called when get-buffer buffer empty (gets a character from stream, and points past it)
	virtual streampos seekoff(streamoff off, ios::seekdir way, ios::openmode which);
	virtual streampos seekpos(streampos sp, ios::openmode which);
	virtual streambuf* setbuf(char*, streamsize) { return 0; } // do not allow to change buffer
	virtual streamsize xsgetn(char* buffer, streamsize count);
	virtual streamsize xsputn(const char* buffer, streamsize count);

	virtual void start_encoding();
	void encode_internal_buffer();

	virtual void start_decoding();
	int  decode_into_internal_buffer();

#ifdef LEDA_COMPRESSION_WA1
// work-around for MSVC 6.0 and CXX 5.1 (alpha) and HPUX aCC (see also LEDA/internal/PREAMBLE.h)
public:
    virtual streampos seekoff(streamoff off, ios::seek_dir way, int mode) { return seekoff(off, way, ios::openmode(mode)); }
	virtual streampos seekpos(streampos sp, int mode) { return seekpos(sp, ios::openmode(mode)); }
// Observe: sync must be public below!
#endif
	virtual int sync(); // used to flush buffers

private:
	void set_state(state new_state);

private:
	state       State;
	char*		InternalBuffer;
	uint32		BufferSize;
	char		TinyBuffer[4];
	coder_base*	pCoder;
	streambuf*	pEncodedStream;
	bool        OwnCoder;
	bool        OwnEncodedStream;
	bool		SignalEof;
	int			NumOverFlowChars;

	streambuf*  pStreamForBuffering;
	string		TmpFileName;
};

#undef LEDA_STREAMBUF

/// coder_strbuf_adapter_with_seek //////////////////////////////////////////////////////////

class __exportC coder_strbuf_adapter_with_seek : public coder_strbuf_adapter {
public:
	typedef coder_strbuf_adapter base;

public:
	coder_strbuf_adapter_with_seek(coder_base* coder = 0, bool own_coder = false, 
								   streambuf* strbuf = 0, bool own_strbuf = false, 
								   uint32 buf_size = DefaultBufSize);

protected: // inherited from base
	virtual void do_reset(coder_base::reset_type t); 

	virtual int underflow();
	virtual int overflow(int c);
	virtual int sync();

	virtual streampos seekoff(streamoff off, ios::seekdir way, ios::openmode which);
	virtual streampos seekpos(streampos sp, ios::openmode which);

protected:
	// common for encoding/decoding
	streampos get_current_pos() const
	{ return encoding() ? encode_get_current_pos() : decode_get_current_pos(); }
		// NOTE: this works also if state == s_init

	streampos get_pos_EOF(ios::openmode which);

	// encoding
	streampos encode_seekpos(streampos sp, ios::openmode which);

	streampos encode_get_current_pos() const
	{ return PosCorrespToIntBuffer + streamoff(pptr() - pbase()); }

	// decoding
	streampos decode_seekpos(streampos sp, ios::openmode which);

	streampos decode_get_current_pos() const
	{ return PosCorrespToIntBuffer + streamoff(gptr() - eback()); }

	streampos decode_get_current_pos_of_coder() const
	{ return PosCorrespToIntBuffer + streamoff(egptr() - eback()); }

private:
	streampos PosCorrespToIntBuffer;
	streampos PosEOF;
};

#if LEDA_ROOT_INCL_ID == 500397
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif

LEDA_END_NAMESPACE

#endif
