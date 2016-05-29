/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  dict_coder.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:44:38 $

#ifndef _LEDA_DICT_CODER_H
#define _LEDA_DICT_CODER_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500068
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/coding/coder_base.h>

LEDA_BEGIN_NAMESPACE

/// DictCoder ///////////////////////////////////////////////////////////////////////////////

/*{\Manpage {DictCoder} {} {Dictionary Based Coder} {C}}*/

class __exportC DictCoder : public coder_base {
/*{\Mdefinition
The class |\Mname| provides a dictionary based coder.
The algorithms for encoding and decoding follow the ideas of 
Lempel and Ziv \cite{LZ77,LZ78}.
}*/
public:
	typedef coder_base base;
	enum { ID = 0x434c06 };

public:
/*{\Mcreation}*/

	DictCoder(streambuf* src_stream = 0, streambuf* tgt_stream = 0, bool own_streams = false);
	/*{\Mcreate creates an instance |\Mvar| which uses the given source 
	and target streams. If |own_streams| is set, then |\Mvar| is 
	responsible for the destruction of the streams, otherwise the pointers 
	|src_stream| and |tgt_stream| must be valid during the life-time of 
	|\Mvar|.}*/

	DictCoder(const char* src_file_name, const char* tgt_file_name);
	/*{\Mcreate creates an instance |\Mvar| which uses file-streams for 
	input and output.}*/

	~DictCoder();

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
	/*{\Mop returns |true| if the coding is finished.}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	virtual string get_description() const;
*/
	/*{\Mop provides a description for |\Mvar|.}*/

protected:
	virtual const char* get_class_name() const { return "DictCoder"; }
	virtual id_type get_id() const { return ID; }

	virtual void init_encoding();
	virtual void encode_stream();

	virtual void init_decoding();
	virtual void decode_stream();

public: // only public because of MSVC 6.0
	typedef uint16 CodeType; // can hold c_MaxCode
	typedef unsigned long IndexType; // can hold Dictionary_Size
	typedef unsigned long CountType;

private:
	void write_code(CodeType code)
	{ write_uint16(code); TotalBytesOut += 2; CurrBytesOut += 2; }

	void read_code(CodeType& code)
	{ read_uint16(code); }

	void load_decode_buffer(unsigned start, CodeType code);

private:
	CountType TotalBytesIn;   // number of data bytes in input file
	CountType TotalBytesOut;  // number of data bytes in output file
	CountType CurrBytesIn;  // number of data bytes in current frame
	CountType CurrBytesOut; // number of data bytes in current frame
	unsigned  BestRatio;    // best ratio so far
	unsigned  LastRatio;    // last ratio seen
	CodeType  CurrCode;

	enum { DecodeBufferSizeStep = 1000 };
	byte*     DecodeBuffer;
	unsigned  DecodeBufferSize;
	bool      NewDictionary;
	byte      PreviousSymbol;
	CodeType  PreviousCode;
	unsigned  PendingDecodedBytesCount;

private:
	class dictionary {
	public:
		enum { Dictionary_Size = 81901L };

		enum { c_PresetMax = 256, c_EndOfInput = c_PresetMax, 
			   c_NewDictionary = c_EndOfInput+1, c_NIL = c_NewDictionary+1, 
			   c_FirstFree = c_NIL+1, c_MaxCode = 65535 };

		struct entry {
			byte symbol;
			CodeType code;
			CodeType parent;
		};
		
		dictionary() {}

		entry& operator[](IndexType idx) { return Entries[idx]; }

		IndexType find(CodeType code, int in);

		void insert_at_idx(IndexType idx, int in, CodeType p)
		{ Entries[idx].symbol = (byte) in; Entries[idx].code = NextFreeCode++; Entries[idx].parent = p; }

		void clear()
		{ quick_clear(); for (IndexType i = 0; i < Dictionary_Size; ++i) Entries[i].code = c_NIL; }

		bool full() const { return NextFreeCode >= c_MaxCode; }

		bool contains(CodeType incode) { return incode < NextFreeCode; }

		void quick_clear() { NextFreeCode = c_FirstFree; }

		void append(byte c, CodeType p)
		{ Entries[NextFreeCode].symbol = c; Entries[NextFreeCode].parent = p; ++NextFreeCode; }

	private:
		entry		Entries[Dictionary_Size];
		CodeType	NextFreeCode;
	};

private:
	dictionary Dictionary;
};

#if LEDA_ROOT_INCL_ID == 500068
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif

LEDA_END_NAMESPACE

#endif
