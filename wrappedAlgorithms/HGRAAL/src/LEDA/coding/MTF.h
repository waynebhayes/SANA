/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  MTF.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:44:35 $

#ifndef _LEDA_MTF_H
#define _LEDA_MTF_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500178
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/coding/coder_base.h>

//#define LEDA_SIMPLE_MTF

LEDA_BEGIN_NAMESPACE

/// MTFCoder ////////////////////////////////////////////////////////////////////////////////

/*{\Manpage {MTFCoder} {} {Move-To-Front Coder} {C}}*/

class __exportC MTFCoder : public coder_base {
/*{\Mdefinition
A Move-To-Front coder is used for preprocessing the input before it is 
fed to the actual compressor. 
Encoding works as follows: The coder maintains a list $L$ containing all 
the 256 characters that can appear in the input. Whenever it receives 
an input character $c$ it looks up the position $i$ of $c$ in $L$, 
outputs $i$ and moves $c$ to the front of $L$.\\
Let us look at an example. Assume that the position of every character in
the initial list is equal to its ASCII-code. When we encode the sequence 
|aaabbbbaab| we obtain the output $97,0,0,98,0,0,0,1,0,1$.
So a |\Mname| has the following properties:
\begin{itemize}
\item If the input contains sequences where one character is repeated many
times then the output will have a lot of consecutive zeros.
\item A sequence in the input where a small set of different characters is
used corresponds to a sequence in the output with (mainly) small integers.
\end{itemize}
Therefore this coder is often used as an intermediate coding step between
a Burrows-Wheeler transform (see Section~\ref{BWTCoder}) and a 
compressing coder.
}*/
public:
	typedef coder_base base;
	enum { ID = 0x434c08 };

public:
/*{\Mcreation}*/

	MTFCoder(streambuf* src_stream = 0, streambuf* tgt_stream = 0, bool own_streams = false);
	/*{\Mcreate creates an instance |\Mvar| which uses the given source 
	and target streams. If |own_streams| is set, then |\Mvar| is 
	responsible for the destruction of the streams, otherwise the pointers 
	|src_stream| and |tgt_stream| must be valid during the life-time of 
	|\Mvar|.}*/

	MTFCoder(const char* src_file_name, const char* tgt_file_name);
	/*{\Mcreate creates an instance |\Mvar| which uses file-streams for 
	input and output.}*/

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
	virtual const char* get_class_name() const { return "MTFCoder"; }
	virtual id_type get_id() const { return ID; }

	virtual void init_encoding();
	virtual void encode_stream();

	virtual void init_decoding();
	virtual void decode_stream();

private:
	void init_table();

private:
#ifdef LEDA_SIMPLE_MTF
	byte Table[256];
#else
	byte CyclicSucc[256];
	byte Tail;
#endif
};


#if LEDA_ROOT_INCL_ID == 500178
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif

LEDA_END_NAMESPACE

#endif
