/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  arithmetic_coding.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:44:35 $

#ifndef _LEDA_ARITHMETIC_CODING_H
#define _LEDA_ARITHMETIC_CODING_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500002
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/coding/coder_base.h>

LEDA_BEGIN_NAMESPACE

/// arithmetic_coder_base ///////////////////////////////////////////////////////////////////

class __exportC arithmetic_coder_base : public coder_base {
public:
	typedef coder_base base;
	enum { EOFSymbol = 0x100 };

protected:
	arithmetic_coder_base(streambuf* src_stream = 0, streambuf* tgt_stream = 0, bool own_streams = false);
	arithmetic_coder_base(const char* src_file_name, const char* tgt_file_name);

protected:
	void ResetCoder();

	void FinishEncoding();
	void StartDecoding();

	void   UpdateEncoder(uint32 lower_range, uint32 upper_range, uint32 total_range);
	void   UpdateDecoder(uint32 lower_range, uint32 upper_range, uint32 total_range);
	uint32 DecodeValueInRange(uint32 total_range);
	
	void   GenOutputBit(byte bit);
	uint32 GetNextBitFromBuf();
	void   FlushBitBuffer();
	
	inline void OutputUnderflowBits(byte b)
	{ while (UnderflowBits > 0) { GenOutputBit(b); --UnderflowBits; } }

	void WriteBiasedUInt32(uint32 val);
	void ReadBiasedUInt32(uint32& val);

protected:
	uint32 L, H;

	union {
		uint32 UnderflowBits;
		uint32 DecodeBuffer;
	};

	uint32 BitsInBuffer;
	byte   BitBuffer;

	enum { 
		CodeBits     = 31,
		TopValue     = 0x7FFFFFFF,
	    FirstQuarter = 0x20000000,
		Half         = 0x40000000,
	    ThirdQuarter = 0x60000000 
	};
};

/// Order0Model ////////////////////////////////////////////////////////////////////////////

class Order0Model : public coder_types {
public:
	Order0Model(uint32 initial_count = 0) { reset(initial_count); }

	// for encoding & decoding
	uint32 get_total() const { return Tree[0]+Tree[256]; } // includes EOF
	uint32 get_sum_of_non_EOFs() const { return get_total()-1; }

	void   get_range(byte symbol, uint32& low, uint32& high) const { int idx = symbol; high = get_cumulative_count(idx); low = high - Counts[idx]; }
	void   get_range_EOF(uint32& low, uint32& high) const { high = get_total(); low = high-1; }
	uint32 get_symbol_and_range(uint32 value, uint32& low, uint32& high) const;

	// for updating statistics globally
	void reset(uint32 initial_count);
	void scale(uint32 scale_divisor);

	// for dynamic coder
	void update(byte symbol);

	// for static coder
	void   raw_increment_count(byte symbol) { ++Counts[symbol]; }
	uint32 raw_get_count(byte symbol) const { return Counts[symbol]; }
	void   raw_set_count(byte symbol, uint32 cnt) { Counts[symbol] = cnt; }
	void   rebuild_from_raw_counts();

private:
	uint32 get_cumulative_count(int idx) const;

private:
	enum { EOFSym = 256, NumSymbols = 257 }; // incl. EOF
	uint32 Counts[NumSymbols-1];
	uint32 Tree[NumSymbols];
};

class Order0StaticModel : public coder_types {
public:
	Order0StaticModel(uint32 initial_count = 0) { reset(initial_count); }

	// for encoding & decoding
	uint32 get_total() const { return Counts[NumSymbols]; } // includes EOF
	uint32 get_sum_of_non_EOFs() const { return Counts[EOFSym]; }

	void   get_range(byte symbol, uint32& low, uint32& high) const { low = Counts[symbol]; high = Counts[symbol+1]; }
	void   get_range_EOF(uint32& low, uint32& high) const { low = Counts[EOFSym]; high = Counts[EOFSym+1]; }
	uint32 get_symbol_and_range(uint32 value, uint32& low, uint32& high) const;

	// for updating statistics globally
	void reset(uint32 initial_count);
	void scale(uint32 scale_divisor);

	// for static coder
	void   raw_increment_count(byte symbol) { ++Counts[symbol]; }
	uint32 raw_get_count(byte symbol) const { return Counts[symbol]; }
	void   raw_set_count(byte symbol, uint32 cnt) { Counts[symbol] = cnt; }
	void   rebuild_from_raw_counts();

private:
	enum { EOFSym = 256, NumSymbols = 257 }; // incl. EOF
	uint32 Counts[NumSymbols+1];
};

/// A0Coder ////////////////////////////////////////////////////////////////////////////////

/*{\Manpage {A0Coder} {} {Adaptive Arithmetic Coder} {C}}*/

class __exportC A0Coder : public arithmetic_coder_base {
/*{\Mdefinition
An instance |\Mvar| of |\Mname| is an adaptive arithmetic coder of order
zero (which explains the ``0'' in the class name).
When the coder encodes a stream it counts the frequencies of the 
characters that have occured so far in order to obtain a model of the 
probabilities for the future characters. The model is called order zero
because it does not take into account that the probability of a character 
|c| may depend on the characters that precede it. (E.g., in an 
English text the probability that the next character is a ``u'' is very 
high if the current character is a ``q''. 
The |PPMIICoder| in Section~\ref{PPMIICoder} takes this into account.)
The advantage of arithmetic coding \cite{WRC:arith-coding} is that a 
character in the source stream can be encoded by a fractional number of 
bits in the target stream, which leads to good results in practice.\\
The coder is called \emph{adaptive} because the model evolves gradually 
while the coder scans its input. In contrast to that a \emph{static} coder 
reads the whole input once before it generates a model. During the actual 
encoding (in a second scan) the model remains fixed. The adaptive coder 
resets and/or scales its model when the number of characters read since the 
last reset/scaling exceeds a certain threshold. Both thresholds can be 
controlled by the user.
}*/
public:
	typedef arithmetic_coder_base base;
	enum { ID = 0x434c00 };
	enum { DefaultResetThreshold = 0, DefaultScaleThreshold = 0x3FFF };
public:
/*{\Mcreation}*/

	A0Coder(streambuf* src_stream = 0, streambuf* tgt_stream = 0, bool own_streams = false);
	/*{\Mcreate creates an instance |\Mvar| which uses the given source 
	and target streams. If |own_streams| is set, then |\Mvar| is 
	responsible for the destruction of the streams, otherwise the pointers 
	|src_stream| and |tgt_stream| must be valid during the life-time of 
	|\Mvar|.}*/

	A0Coder(const char* src_file_name, const char* tgt_file_name);
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
	void reset(bool keep_thresholds = true); 
*/
	/*{\Mop puts |\Mvar| in the same state as the default constructor.
	If |keep_thresholds| is false the thresholds are set to their default
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

/*{\Moptions nextwarning=no }*/
/*	inherited:
	virtual string get_description() const;
*/
	/*{\Mop provides a description for |\Mvar|.}*/

/*{\Mtext \headerline{Additional Operations}}*/

	uint32 get_reset_threshold() const { return ResetThreshold; }
	/*{\Mop returns the current threshold for resetting the model.}*/
	void   set_reset_threshold(uint32 t);
	/*{\Mop sets the threshold for resetting the model.}*/

	uint32 get_scale_threshold() const { return ScaleThreshold; }
	/*{\Mop returns the current threshold for scaling the model.}*/
	uint32 get_max_scale_threshold() const { return FirstQuarter-1; }
	/*{\Mop returns the maximum threshold $S$ for scaling the model.}*/
	void   set_scale_threshold(uint32 t);
	/*{\Mop sets the threshold for scaling the model.\\ \precond $512 \leq t \leq S$}*/

protected:
	virtual const char* get_class_name() const { return "A0Coder"; }
	virtual id_type get_id() const { return ID; }

	virtual void init_encoding();
	virtual void encode_stream();

	virtual void init_decoding();
	virtual void decode_stream();

	virtual void do_reset(reset_type t);

private:
	void UpdateModel(byte symbol);

private:
	uint32 BytesSinceLastReset;
	uint32 ResetThreshold;
	uint32 ScaleThreshold;
	Order0Model Model;
};

/// A0sCoder ///////////////////////////////////////////////////////////////////////////////

/*{\Mtext \newpage}*/
/*{\Manpage {A0sCoder} {} {Static Arithmetic Coder} {C}}*/

class __exportC A0sCoder : public arithmetic_coder_base {
/*{\Mdefinition
An instance |\Mvar| of |\Mname| is a static arithmetic coder of order 
zero. 
(We assume that the reader is familiar with the definition of 
the class |A0Coder| from the previous section.)
|\Mname| is called \emph{static} because it uses a fixed model for 
encoding. 
The input is scanned twice. In the first scan the frequencies of the 
characters are counted. This information is used to precompute the model. 
In the second scan the actual coding takes place.\\
Since the model is fixed the static coder is usually faster than the 
adaptive coder (despite the two scans) but the compression rates are 
often slightly inferior.
}*/
public:
	typedef arithmetic_coder_base base;
	enum { ID = 0x434c02 };

public:
/*{\Mcreation}*/

	A0sCoder(streambuf* src_stream = 0, streambuf* tgt_stream = 0, bool own_streams = false);
	/*{\Mcreate creates an instance |\Mvar| which uses the given source 
	and target streams. If |own_streams| is set, then |\Mvar| is 
	responsible for the destruction of the streams, otherwise the pointers 
	|src_stream| and |tgt_stream| must be valid during the life-time of 
	|\Mvar|.}*/

	A0sCoder(const char* src_file_name, const char* tgt_file_name);
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

	virtual bool is_single_pass_encoder() const { return false; }

protected:
	virtual const char* get_class_name() const { return "A0sCoder"; }
	virtual id_type get_id() const { return ID; }

	virtual void init_encoding();
	virtual void encode_stream();

	virtual void init_decoding();
	virtual void decode_stream();

private:
	void ComputeModelFromStaticCounts();

private:
	Order0StaticModel Model;
};

#if LEDA_ROOT_INCL_ID == 500002
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif

LEDA_END_NAMESPACE

#endif
