/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  auto_decoder.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:44:36 $

#ifndef _LEDA_AUTO_DECODER_H
#define _LEDA_AUTO_DECODER_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500006
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/coding/coder_util.h>

LEDA_BEGIN_NAMESPACE

/// AutoDecoder /////////////////////////////////////////////////////////////////////////////

/*{\Manpage {AutoDecoder} {} {Automatic Decoder} {C}}*/

class __exportC AutoDecoder : public coder_base {
/*{\Mdefinition
An instance |\Mvar| of |\Mname| can be used for decoding any stream that 
has been encoded with a LEDA coder or a pipe of LEDA coders.
Thus this class is useful if you want to decode a stream and have forgotten
which combination of coders you have used to encode it. 
This class is also helpful if the encoding method is not known at 
compile-time. Then the decoding method cannot be fixed at compile-time 
either but it has to be determined at run-time.
}*/
/*{\Mtypes}*/
/*{\Mtext In order to facilitate the usage of |\Mname| with 
|decoding_istream| we provide the [[typedef]] |autodecoding_istream| as 
a shorthand for |decoding_istream<AutoDecoder>|.}*/
public:
	typedef coder_base base;
	enum { ID = 0x434c03 };

public:
/*{\Mcreation}*/

	AutoDecoder(streambuf* src_stream = 0, streambuf* tgt_stream = 0, bool own_streams = false);
	/*{\Mcreate creates an instance |\Mvar| which uses the given source 
	and target streams. If |own_streams| is set, then |\Mvar| is 
	responsible for the destruction of the streams, otherwise the pointers 
	|src_stream| and |tgt_stream| must be valid during the life-time of 
	|\Mvar|.}*/

	AutoDecoder(const char* src_file_name, const char* tgt_file_name);
	/*{\Mcreate creates an instance |\Mvar| which uses file-streams for 
	input and output.}*/

	~AutoDecoder();

/*{\Moperations}*/
/*{\Mtext \headerline{Standard Operations}}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	void decode();
*/
	/*{\Mop decodes the source stream and writes the output to the target
	stream.}*/
/*{\Moptions nextwarning=no }*/
/*	inherited:
	streambuf* get_src_stream() const;
*/
	/*{\Mop returns the current source stream.}*/
/*{\Moptions nextwarning=no }*/
/*	inherited:
	uint32 decode_memory_chunk(const char* in_buf, uint32 in_len, char* out_buf, uint32 out_len);
*/
	/*{\Mop decodes the memory chunk starting at |in_buf| with size |in_len| 
	into the buffer starting at |out_buf| with size |out_len|. 
	The function returns actual length of the encoded chunk which may be 
	smaller than |out_len|. If the output buffer is too small for the decoded
	data the failure flag will be set (see below).}*/

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
	void reset(); 
*/
	/*{\Mop puts |\Mvar| in exactly the same state as the default 
	constructor.}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	bool failed() const;
*/
	/*{\Mop returns |true| if an error occured.}*/
/*{\Moptions nextwarning=no }*/
/*	inherited:
	bool finished() const;
*/
	/*{\Mop returns |true| if decoding is finished.}*/

	virtual string get_description() const;
	/*{\Mop provides a description for |\Mvar|. After decoding this 
	includes a description of the coder that has been used for encoding the
	stream.}*/

/*{\Moptions nextwarning=no }*/
	virtual bool is_optimizing_seeks() const;
	virtual bool is_secrecy_sensitive() const;
	virtual bool needs_seeks_during_coding() const;

/*{\Mtext \headerline{Additional Operations}}*/

	coder_base* get_coder() const { return pActualCoder; }
	/*{\Mop after decoding this function returns the coder that has actually
	been used to decode the stream. E.g., if the original source stream
	has been encoded with an instance of type |A0Coder| then the function 
	returns an instance of |A0Coder|.}*/

	void        set_coder(coder_base* coder, bool own_coder = false);

protected:
	virtual const char* get_class_name() const { return "AutoDecoder"; }
	virtual id_type get_id() const { return ID; }

	virtual void init_encoding();
	virtual void encode_stream();

	virtual void init_decoding();
	virtual void decode_stream();

	virtual void do_reset(reset_type t);

	virtual streampos encode_seek(streampos current_pos, streampos new_pos);
	virtual streampos decode_seek(streampos current_pos, streampos new_pos);
	virtual streambuf* change_src_stream_temporarily(streambuf* src_stream);
	virtual streambuf* change_tgt_stream_temporarily(streambuf* tgt_stream);

protected:
	// the additional stream is passed to pActualCoder when it is constructed
	// from src_stream (the additional stream may hold secret data for example)
	streambuf* get_additional_stream() const { return pAdditionalStream; }
	void       set_additional_stream(streambuf* additional_stream, bool own_stream);
	void       set_additional_file(const char* file_name, ios::openmode mode = ios::openmode(ios::in | binary));

private:
	coder_base* pActualCoder;
	streambuf*  pAdditionalStream;
	bool        OwnCoder;
	bool		OwnAdditionalStream;
};

// Handy short-hand:
typedef decoding_ifstream<AutoDecoder> autodecoding_ifstream;

#if LEDA_ROOT_INCL_ID == 500006
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif

LEDA_END_NAMESPACE

#endif
