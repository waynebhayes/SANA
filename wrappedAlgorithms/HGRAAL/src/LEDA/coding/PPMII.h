/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  PPMII.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.3 $  $Date: 2005/04/14 10:44:35 $

#ifndef _LEDA_PPMII_H
#define _LEDA_PPMII_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500704
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/coding/coder_base.h>

LEDA_BEGIN_NAMESPACE

/// PPMIICoder //////////////////////////////////////////////////////////////////////////////

/*{\Manpage {PPMIICoder} {} {Prediction by Partial Matching} {C}}*/

class __exportC PPMIICoder : public coder_base {
/*{\Mdefinition
The |\Mtype| is based on the compression scheme 
``Prediction by Partial Matching with Information Inheritance''
by D.~Shkarin \cite{Shk02}.\\
This coder works as follows: Suppose we have processed the first $n-1$ symbols
$x_1 \dots x_{n-1}$ of the stream. Before reading the next symbol $x_n$ we try
to guess it, i.e.~for every symbol $s$ we estimate the probability $p(s)$ for
the event ``$x_n = s$''. This probability distribution determines how the next
symbol is encoded: The higher $p(s)$, the fewer bits are used for encoding $s$.
If our estimation is good, which means that $p(x_n)$ is high, then we obtain 
a good compression rate. \\
In order to predict the probality distribution for the $n$th symbol the PPM 
approach considers the preceding $k$ symbols $x_{n-k} \dots x_{n-1}$. 
We call these symbols the \emph{context} of $x_n$ and $k$ the order of the 
model. (For $k=0$ we obtain the order-0 model from the previous section.)
E.g., if the current context is ``req'', then we should 
predict the letter ``u'' as next symbol with high probability.\\
PPMII is a variant of PPM which usually achieves very accurate estimations.

The |\Mtype| combines very good compression rates with acceptable speed. 
(Shkarin \cite{Shk02} reports that his coder outperforms ZIP and BZIP2 with 
respect to compression rates and speed.)
The only disadvantage of this coder is that it needs a fair amount of main 
memory to store the model. However, the user can set an upper bound on the 
memory usage. And he can specify which model restoration method the coder shall 
apply when it runs out of memory:
\begin{itemize}
\item |mr_restart| (default):\\
The model is deleted completely and rebuilt from scratch. This method is fast.
\item |mr_cut_off|:\\
Parts of the model are freed to gain memory. This method is optimal for 
so-called quasistationary sources. It usually gives better compression but it
is slower.
\item |mr_freeze|:\\
The model is not extended any more. This method is optimal for so-called 
stationary sources. (We want to point out that data streams arising in 
practical applications usually do not behave like a stationary source.)
\end{itemize}
}*/
public:
	typedef coder_base base;
	enum { ID = 0x434c10 };

/*{\Mtypes}*/
	enum mr_method { mr_restart, mr_cut_off, mr_freeze };
	/*{\Menum the different model restoration modes.}*/

	enum defaults { def_max_memory = 10, def_order = 4, def_mrm = mr_restart };

public:
/*{\Mcreation}*/

	PPMIICoder(streambuf* src_stream = 0, streambuf* tgt_stream = 0, bool own_streams = false);
	/*{\Mcreate creates an instance |\Mvar| which uses the given source 
	and target streams. If |own_streams| is set, then |\Mvar| is 
	responsible for the destruction of the streams, otherwise the pointers 
	|src_stream| and |tgt_stream| must be valid during the life-time of 
	|\Mvar|.}*/

	PPMIICoder(const char* src_file_name, const char* tgt_file_name);
	/*{\Mcreate creates an instance |\Mvar| which uses file-streams for 
	input and output.}*/

	~PPMIICoder();

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

	uint16 get_model_memory_bound() const { return MaxMemory; }
	/*{\Mop returns how much memory (in MB) |\Mvar| is allowed to use for 
	storing the model.}*/
	void   set_model_memory_bound(uint16 max_mem);
	/*{\Mop determines the amount of memory available for the model (in MB).}*/

	byte get_model_order() const { return MaxModelOrder; }
	/*{\Mop returns the order of the model.}*/
	void set_model_order(byte order);
	/*{\Mop sets the order of the model.\\ \precond $|order| \in [2..16]$.}*/

	mr_method get_model_restoration_method() const { return ModelRestorationMethod; }
	/*{\Mop returns the model restoration method.}*/
	void      set_model_restoration_method(mr_method method);
	/*{\Mop sets the model restoration method.}*/

protected:
	virtual const char* get_class_name() const { return "PPMIICoder"; }
	virtual id_type get_id() const { return ID; }

	virtual void do_reset(reset_type t);

	virtual void write_header_to_tgt_stream();
	virtual void read_header_from_src_stream();
	virtual uint32 get_header_length(bool& is_exact) const;

	virtual void init_encoding();
	virtual void encode_stream();

	virtual void init_decoding();
	virtual void decode_stream();

private:
	uint16		MaxMemory;
	byte		MaxModelOrder;
	mr_method	ModelRestorationMethod;
};
/*{\Mimplementation
Our implementation encapsulates the code by D.~Shkarin (the implementation of the
PPMII model) and by D.~Subbotin (the implementation of the range coder).
}*/

#if LEDA_ROOT_INCL_ID == 500704
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif

LEDA_END_NAMESPACE

#endif
