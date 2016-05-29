/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  coder_util.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.4 $  $Date: 2005/04/14 10:44:37 $

#ifndef _LEDA_CODER_UTIL_H
#define _LEDA_CODER_UTIL_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500024
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/coding/coder_strbuf_adapter.h>
#include <LEDA/coding/coder_pipe.h>

LEDA_BEGIN_NAMESPACE

/// CoderAdapter ////////////////////////////////////////////////////////////////////////////

template <class Coder>
class CoderAdapter : public coder_strbuf_adapter_with_seek {
public:
	typedef coder_strbuf_adapter_with_seek base;

public:
	CoderAdapter(streambuf* strbuf = 0, bool own_strbuf = false, uint32 buf_size = 0)
	 : base(new Coder(), true, strbuf, own_strbuf, buf_size) {} 

	Coder* get_coder() const { return static_cast<Coder*>(base::get_coder()); }
	void set_coder(Coder* coder, bool own_coder = false) { base::set_coder(coder, own_coder); }
};

/// encoding_ostream ////////////////////////////////////////////////////////////////////////

/*{\Manpage {encoding_ostream} {Coder} {Encoding Output Stream} {os}}*/

template <class Coder>
class encoding_ostream : public ostream {
/*{\Mdefinition
The type |\Mname| is the encoding counterpart of the type |ostream| from 
the \CC{} iostream library. 
Each instance |\Mvar| of type |\Mname| is attached to a |streambuf| |sb|.
Every data that is written to |\Mvar| is encoded on-the-fly by an 
instance of type |Coder| and then written to |sb|.
All operations and all operators ([[<<]]) defined for \CC{} |ostreams| 
can be applied to |\Mtype| as well.}*/
public:
/*{\Mcreation}*/

	explicit 

        encoding_ostream(streambuf* encoded_stream, bool own_stream = false)
	 : ostream(new CoderAdapter<Coder>()) 
	{
		pStreamBuf = static_cast<CoderAdapter<Coder>*>(rdbuf());
		if (pStreamBuf) pStreamBuf->set_encoded_stream(encoded_stream, own_stream); 
		if (!pStreamBuf || pStreamBuf->failed()) clear(ios::badbit);
	}
	/*{\Mcreate creates an instance |\Mvar| and attaches it to the given |streambuf|
	object.}*/

	virtual ~encoding_ostream() 
	{
		// observe that possibly rdbuf() != pStreamBuf (the user may have changed it)
		if (pStreamBuf) { delete pStreamBuf; pStreamBuf = 0; }
	}

/*{\Moperations}*/
	Coder* get_coder() const { return pStreamBuf ? static_cast<Coder*>(pStreamBuf->get_coder()) : 0; }
	/*{\Mop returns the instance of |Coder| which is used for encoding.}*/

	CoderAdapter<Coder>* get_adapter() const { return pStreamBuf; }

	void close()
	{
		CoderAdapter<Coder>* adapter = get_adapter();
		if (adapter) { 
			adapter->sputEOF(); 
			if (adapter->failed()) clear(ios::badbit);
			adapter->reset();
		}
	}
	/*{\Mop detaches |\Mvar| from its |streambuf| object.}*/

private:
	CoderAdapter<Coder>* pStreamBuf;
		// it would be nicer to avoid the pointer, but Borland 6.0 does not support ostream(&StreamBuf)!
};

/// decoding_istream ////////////////////////////////////////////////////////////////////////

/*{\Manpage {decoding_istream} {Coder} {Decoding Input Stream} {is}}*/

template <class Coder>
class decoding_istream : public istream {
/*{\Mdefinition
The type |\Mname| is the decoding counterpart of the type |istream| from 
the \CC{} iostream library. 
Each instance |\Mvar| of type |\Mname| is attached to a |streambuf| |sb|.
Whenever data is requested from |\Mvar|, it reads some data from |sb| and 
then decodes it on-the-fly with an intance of type |Coder|.
All operations and all operators ([[>>]]) defined for \CC{} |istreams| 
can be applied to |\Mtype| as well.
}*/
public:
/*{\Mcreation}*/

	explicit 

        decoding_istream(streambuf* encoded_stream, bool own_stream = false)
	 : istream(new CoderAdapter<Coder>()) 
	{
		pStreamBuf = static_cast<CoderAdapter<Coder>*>(rdbuf());
		if (pStreamBuf) pStreamBuf->set_encoded_stream(encoded_stream, own_stream);
		if (!pStreamBuf || pStreamBuf->failed()) clear(ios::badbit);
	}
	/*{\Mcreate creates an instance |\Mvar| and attaches it to the given |streambuf|
	object.}*/

	virtual ~decoding_istream()
	{
		// observe that possibly rdbuf() != pStreamBuf (the user may have changed it)
		delete pStreamBuf; pStreamBuf = 0;
	}

/*{\Moperations}*/
	Coder* get_coder() const { return pStreamBuf ? static_cast<Coder*>(pStreamBuf->get_coder()) : 0; }
	/*{\Mop returns the instance of |Coder| which is used for encoding.}*/

	CoderAdapter<Coder>* get_adapter() const { return pStreamBuf; }

	void close()
	{
		CoderAdapter<Coder>* adapter = get_adapter();
		if (adapter) {
			if (good()) peek(); // read EOF (possibly)
			if (adapter->failed()) clear(ios::badbit);
			adapter->reset();
		}
	}
	/*{\Mop detaches |\Mvar| from its |streambuf| object.}*/

	void finish()
	{
		while (good() && peek() != EOF) get();
		close();
	}
	/*{\Mop reads till the end of the stream and then closes |\Mvar|.
	(This is useful if |Coder| is a checksummer; the checksum is only verified
	after EOF has been read.)}*/

private:
	CoderAdapter<Coder>* pStreamBuf;
};

/// encoding_ofstream ///////////////////////////////////////////////////////////////////////

/*{\Manpage {encoding_ofstream} {Coder} {Encoding File Stream} {os}}*/

template <class Coder>
class encoding_ofstream : public encoding_ostream<Coder> {
/*{\Mdefinition
The type |\Mname| is the encoding counterpart of the type |ofstream| from 
the \CC{} iostream library. 
Each instance |\Mvar| of type |\Mname| is associated with a file.
Every data that is written to |\Mvar| is encoded on-the-fly by an 
instance of type |Coder| and then written to the associated file.
All operations and all operators ([[<<]]) defined for \CC{} |ostreams| 
can be applied to |\Mtype| as well.}*/
public:
	typedef encoding_ostream<Coder> base;
public:
/*{\Mcreation}*/

	encoding_ofstream(const char* file_name = 0, ios::openmode mode = ios::openmode(ios::out | ios::trunc))
	 : base(0) { open(file_name, mode); }
	/*{\Mcreate creates an instance |\Mvar|. If |file_name| is specified, 
	the stream is attached to the file with the given name.}*/

	virtual ~encoding_ofstream() { base::close(); }

/*{\Moperations}*/
	bool is_open() const { return base::get_adapter() && (base::get_adapter()->get_encoded_stream() != 0); }
	/*{\Mop returns if |\Mvar| is attached to an open file.}*/

	void open(const char* file_name = 0, ios::openmode mode = ios::openmode(ios::out | ios::trunc))
	{
		CoderAdapter<Coder>* adapter = base::get_adapter();
		if (adapter) {
			adapter->reset(); base::clear();
			adapter->set_encoded_file(file_name, mode);
		}
		if (!adapter || adapter->failed()) base::clear(ios::badbit);
	}
	/*{\Mop opens a file and attaches |\Mvar| to it.}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	void close();
*/
	/*{\Mop closes the attached file and detaches |\Mvar| from it.}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	streampos tellp();
*/
	/*{\Mop queries the position of the (internal) put pointer.
	This pointer determines the position in the (original) stream whereto
	the next character is written.}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	ostream& seekp(streampos pos);
*/
	/*{\Mop sets the position of the put pointer to |pos|.
	|pos| must be greater than the current put pointer.  
	The skipped characters are assumed to be zero.}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	ostream& seekp(streamoff off, ios::seekdir dir);
*/
	/*{\Mop moves the put pointer by |off| relative to the position determined
	by |dir|.
	|dir| can be |ios::beg| (beginning), |ios::cur| (current position) or
	|ios::end| (last position).
	The new position of the put pointer has to be greater than its current 
	position. The skipped characters are assumed to be zero.}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	Coder* get_coder() const { return pStreamBuf ? static_cast<Coder*>(pStreamBuf->get_coder()) : 0; }
*/
	/*{\Mop returns the instance of |Coder| which is used for encoding.}*/
};

/// decoding_ifstream ///////////////////////////////////////////////////////////////////////

/*{\Manpage {decoding_ifstream} {Coder} {Decoding File Stream} {is}}*/

template <class Coder>
class decoding_ifstream : public decoding_istream<Coder> {
/*{\Mdefinition
The type |\Mname| is the decoding counterpart of the type |ifstream| from 
the \CC{} iostream library. 
Each instance |\Mvar| of type |\Mname| is associated with a file.
Whenever data is requested from an instance |\Mvar| of type |\Mname|, 
|\Mvar| reads some data from the associated file and then decodes 
it on-the-fly with an intance of type |Coder|.
All operations and all operators ([[>>]]) defined for \CC{} |istreams| 
can be applied to |\Mtype| as well.

|\Mtype| supports random access read operations by providing seek operations.
For this purpose it maintains a so-called \emph{get pointer}. This pointer 
specifies the position in the decoded data stream from which the next read 
operation will extract its data. After the operation the get pointer refers to
the position immediately behind the last extracted character. 
E.g., if you are not interested in the first 5000 characters and you want to 
extract the 10 characters in positions 5000 -- 5009, you can perform a seek 
operation which moves the get pointer to position 5000 and then read 10 
characters. (After that the get pointer will refer to position 5010.)
This seek operation will usually be faster than skipping 5000 characters by 
hand. \\
Seek operations are supported no matter which coder is plugged into |\Mtype|.
However, some coders (like checksummers) provide fast seek operations. This 
manual mentions these coders explicitly. 
Moreover, we provide a class called |BlockCoder| (in Section~\ref{Block Coder})
which allows you to speed up seeks for any coder.}*/
public:
	typedef decoding_istream<Coder> base;
public:
/*{\Mcreation}*/

	decoding_ifstream(const char* file_name = 0, ios::openmode mode = ios::in)
	 : base(0) { open(file_name, mode); }
	/*{\Mcreate creates an instance |\Mvar|. If |file_name| is specified, 
	the stream is attached to the file with the given name.}*/

/*{\Moperations}*/
	bool is_open() const { return base::get_adapter() && (base::get_adapter()->get_encoded_stream() != 0); }
	/*{\Mop returns if |\Mvar| is attached to an open file.}*/

	void open(const char* file_name = 0, ios::openmode mode = ios::in)
	{
		CoderAdapter<Coder>* adapter = base::get_adapter();
		if (adapter) {
			adapter->reset(); base::clear();
			adapter->set_encoded_file(file_name, mode);
		}
		if (!adapter || adapter->failed()) base::clear(ios::badbit);
	}
	/*{\Mop opens a file and attaches |\Mvar| to it.}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	void close();
*/
	/*{\Mop closes the attached file and detaches |\Mvar| from it.}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	void finish();
*/
	/*{\Mop reads till the end of the stream and then closes |\Mvar|.
	(This is useful if |Coder| is a checksummer; the checksum is only verified
	after EOF has been read.)}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	streampos tellg();
*/
	/*{\Mop queries the position of the (internal) get pointer.
	This pointer determines the position in the (decoded) stream from which
	the next character is read.}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	istream& seekg(streampos pos);
*/
	/*{\Mop sets the position of the get pointer to |pos|.}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	istream& seekg(streamoff off, ios::seekdir dir);
*/
	/*{\Mop moves the get pointer by |off| relative to the position determined
	by |dir|.
	|dir| can be |ios::beg| (beginning), |ios::cur| (current position) or
	|ios::end| (last position).}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	Coder* get_coder() const { return pStreamBuf ? static_cast<Coder*>(pStreamBuf->get_coder()) : 0; }
*/
	/*{\Mop returns the instance of |Coder| which is used for decoding.}*/
};

/// CoderPipe2 //////////////////////////////////////////////////////////////////////////////

/*{\Mtext \clearpage}*/
/*{\Manpage {CoderPipe2} {Coder1, Coder2} {Coder Pipes} {C}}*/

template <class Coder1, class Coder2>
class CoderPipe2 : public coder_pipe {
/*{\Mdefinition
The type |\Mname| can be used to combine two coders of type |Coder1| and
|Coder2| into one single coder. 
This works in an analogous way as a pipe of the operating system: In encoding 
mode the original input is processed by |Coder1|. Its output is fed into 
|Coder2|. The output of |Coder2| becomes the output of the pipe.
In decoding mode the situation is reversed: The data is sent through 
|Coder2| first and then through |Coder1|.

We also provide pipes for combining more than two coders (up to six): 
$|CoderPipe3|, \dots, |CoderPipe6|$. 
(Since these classes have a similar interface as |\Mtype|, we do not include
manual pages for them.)
}*/
public:
	typedef coder_pipe base;
/*{\Mtypes}*/
	typedef Coder1     coder1;
	/*{\Mtypemember the type |Coder1|.}*/
	typedef Coder2     coder2;
	/*{\Mtypemember the type |Coder2|.}*/

public:
/*{\Mcreation}*/

	CoderPipe2(streambuf* src_stream = 0, streambuf* tgt_stream = 0, bool own_streams = false)
	 : base(2)
	{ 
		base::set_src_stream(src_stream, own_streams); base::set_tgt_stream(tgt_stream, own_streams);
		set_coder1(new Coder1(), true);
		set_coder2(new Coder2(), true);
	}
	/*{\Mcreate creates an instance |\Mvar| which uses the given source 
	and target streams. If |own_streams| is set, then |\Mvar| is 
	responsible for the destruction of the streams, otherwise the pointers 
	|src_stream| and |tgt_stream| must be valid during the life-time of 
	|\Mvar|.}*/

	CoderPipe2(const char* src_file_name, const char* tgt_file_name)
	 : base(2)
	{ 
		base::set_src_file(src_file_name); base::set_tgt_file(tgt_file_name);
		set_coder1(new Coder1(), true);
		set_coder2(new Coder2(), true);
	}
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
	void reset(bool keep_parameters = true); 
*/
	/*{\Mop puts |\Mvar| in the same state as the default constructor.
	If |keep_parameters| is false the parameters of all coders in the pipe are 
	set to their default values.}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	bool failed() const;
*/
	/*{\Mop returns |true| if an error occured or a checksum in the 
	stream does not match the computed checksum.}*/

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

	Coder1* get_coder1() const { return static_cast<Coder1*>(base::get_coder(1)); }
	/*{\Mop returns the currently used instance of |Coder1|.}*/
	void    set_coder1(Coder1* c1, bool own_coder = false) { base::set_coder(1, c1, own_coder); }
	/*{\Mop sets the instance of |Coder1|. If |own_coder| is |true|, then the
	|c1| is deleted by |\Mvar|. Otherwise |c1| is not deleted by |\Mvar| 
	and the pointer |c1| must be valid during the life-time of |\Mvar|.}*/

	Coder2* get_coder2() const { return static_cast<Coder2*>(base::get_coder(2)); }
	/*{\Mop returns the currently used instance of |Coder2|.}*/
	void    set_coder2(Coder2* c2, bool own_coder = false) { base::set_coder(2, c2, own_coder); }
	/*{\Mop sets the instance of |Coder2|. The paramter |own_coder| is explained
	above.}*/

protected:
	virtual const char* get_class_name() const { return "CoderPipe2"; }
};

/// CoderPipe3 //////////////////////////////////////////////////////////////////////////////

template <class Coder1, class Coder2, class Coder3>
class CoderPipe3 : public coder_pipe {
public:
	typedef coder_pipe base;
	typedef Coder1     coder1;
	typedef Coder2     coder2;
	typedef Coder3     coder3;

public:
	CoderPipe3(streambuf* src_stream = 0, streambuf* tgt_stream = 0, bool own_streams = false)
	 : base(3)
	{ 
		base::set_src_stream(src_stream, own_streams); base::set_tgt_stream(tgt_stream, own_streams);
		set_coder1(new Coder1(), true);
		set_coder2(new Coder2(), true);
		set_coder3(new Coder3(), true);
	}

	CoderPipe3(const char* src_file_name, const char* tgt_file_name)
	 : base(3)
	{ 
		base::set_src_file(src_file_name); base::set_tgt_file(tgt_file_name);
		set_coder1(new Coder1(), true);
		set_coder2(new Coder2(), true);
		set_coder3(new Coder3(), true);
	}

	Coder1* get_coder1() const { return static_cast<Coder1*>(base::get_coder(1)); }
	void    set_coder1(Coder1* c1, bool own_coder = false) { base::set_coder(1, c1, own_coder); }

	Coder2* get_coder2() const { return static_cast<Coder2*>(base::get_coder(2)); }
	void    set_coder2(Coder2* c2, bool own_coder = false) { base::set_coder(2, c2, own_coder); }

	Coder3* get_coder3() const { return static_cast<Coder3*>(base::get_coder(3)); }
	void    set_coder3(Coder3* c3, bool own_coder = false) { base::set_coder(3, c3, own_coder); }

protected:
	virtual const char* get_class_name() const { return "CoderPipe3"; }
};

/// CoderPipe4 //////////////////////////////////////////////////////////////////////////////

template <class Coder1, class Coder2, class Coder3, class Coder4>
class CoderPipe4 : public coder_pipe {
public:
	typedef coder_pipe base;
	typedef Coder1     coder1;
	typedef Coder2     coder2;
	typedef Coder3     coder3;
	typedef Coder4     coder4;

public:
	CoderPipe4(streambuf* src_stream = 0, streambuf* tgt_stream = 0, bool own_streams = false)
	 : base(4)
	{ 
		base::set_src_stream(src_stream, own_streams); base::set_tgt_stream(tgt_stream, own_streams);
		set_coder1(new Coder1(), true);
		set_coder2(new Coder2(), true);
		set_coder3(new Coder3(), true);
		set_coder4(new Coder4(), true);
	}

	CoderPipe4(const char* src_file_name, const char* tgt_file_name)
	 : base(4)
	{ 
		base::set_src_file(src_file_name); base::set_tgt_file(tgt_file_name);
		set_coder1(new Coder1(), true);
		set_coder2(new Coder2(), true);
		set_coder3(new Coder3(), true);
		set_coder4(new Coder4(), true);
	}

	Coder1* get_coder1() const { return static_cast<Coder1*>(base::get_coder(1)); }
	void    set_coder1(Coder1* c1, bool own_coder = false) { base::set_coder(1, c1, own_coder); }

	Coder2* get_coder2() const { return static_cast<Coder2*>(base::get_coder(2)); }
	void    set_coder2(Coder2* c2, bool own_coder = false) { base::set_coder(2, c2, own_coder); }

	Coder3* get_coder3() const { return static_cast<Coder3*>(base::get_coder(3)); }
	void    set_coder3(Coder3* c3, bool own_coder = false) { base::set_coder(3, c3, own_coder); }

	Coder4* get_coder4() const { return static_cast<Coder4*>(base::get_coder(4)); }
	void    set_coder4(Coder4* c4, bool own_coder = false) { base::set_coder(4, c4, own_coder); }

protected:
	virtual const char* get_class_name() const { return "CoderPipe4"; }
};

/// CoderPipe5 //////////////////////////////////////////////////////////////////////////////

template <class Coder1, class Coder2, class Coder3, class Coder4, class Coder5>
class CoderPipe5 : public coder_pipe {
public:
	typedef coder_pipe base;
	typedef Coder1     coder1;
	typedef Coder2     coder2;
	typedef Coder3     coder3;
	typedef Coder4     coder4;
	typedef Coder5     coder5;

public:
	CoderPipe5(streambuf* src_stream = 0, streambuf* tgt_stream = 0, bool own_streams = false)
	 : base(5)
	{ 
		base::set_src_stream(src_stream, own_streams); base::set_tgt_stream(tgt_stream, own_streams);
		set_coder1(new Coder1(), true);
		set_coder2(new Coder2(), true);
		set_coder3(new Coder3(), true);
		set_coder4(new Coder4(), true);
		set_coder5(new Coder5(), true);
	}

	CoderPipe5(const char* src_file_name, const char* tgt_file_name)
	 : base(5)
	{ 
		base::set_src_file(src_file_name); base::set_tgt_file(tgt_file_name);
		set_coder1(new Coder1(), true);
		set_coder2(new Coder2(), true);
		set_coder3(new Coder3(), true);
		set_coder4(new Coder4(), true);
		set_coder5(new Coder5(), true);
	}

	Coder1* get_coder1() const { return static_cast<Coder1*>(base::get_coder(1)); }
	void    set_coder1(Coder1* c1, bool own_coder = false) { base::set_coder(1, c1, own_coder); }

	Coder2* get_coder2() const { return static_cast<Coder2*>(base::get_coder(2)); }
	void    set_coder2(Coder2* c2, bool own_coder = false) { base::set_coder(2, c2, own_coder); }

	Coder3* get_coder3() const { return static_cast<Coder3*>(base::get_coder(3)); }
	void    set_coder3(Coder3* c3, bool own_coder = false) { base::set_coder(3, c3, own_coder); }

	Coder4* get_coder4() const { return static_cast<Coder4*>(base::get_coder(4)); }
	void    set_coder4(Coder4* c4, bool own_coder = false) { base::set_coder(4, c4, own_coder); }

	Coder5* get_coder5() const { return static_cast<Coder5*>(base::get_coder(5)); }
	void    set_coder5(Coder5* c5, bool own_coder = false) { base::set_coder(5, c5, own_coder); }

protected:
	virtual const char* get_class_name() const { return "CoderPipe5"; }
};

/// CoderPipe6 //////////////////////////////////////////////////////////////////////////////

template <class Coder1, class Coder2, class Coder3, class Coder4, class Coder5, class Coder6>
class CoderPipe6 : public coder_pipe {
public:
	typedef coder_pipe base;
	typedef Coder1     coder1;
	typedef Coder2     coder2;
	typedef Coder3     coder3;
	typedef Coder4     coder4;
	typedef Coder5     coder5;
	typedef Coder6     coder6;

public:
	CoderPipe6(streambuf* src_stream = 0, streambuf* tgt_stream = 0, bool own_streams = false)
	 : base(6)
	{ 
		base::set_src_stream(src_stream, own_streams); base::set_tgt_stream(tgt_stream, own_streams);
		set_coder1(new Coder1(), true);
		set_coder2(new Coder2(), true);
		set_coder3(new Coder3(), true);
		set_coder4(new Coder4(), true);
		set_coder5(new Coder5(), true);
		set_coder6(new Coder6(), true);
	}

	CoderPipe6(const char* src_file_name, const char* tgt_file_name)
	 : base(6)
	{ 
		base::set_src_file(src_file_name); base::set_tgt_file(tgt_file_name);
		set_coder1(new Coder1(), true);
		set_coder2(new Coder2(), true);
		set_coder3(new Coder3(), true);
		set_coder4(new Coder4(), true);
		set_coder5(new Coder5(), true);
		set_coder6(new Coder6(), true);
	}

	Coder1* get_coder1() const { return static_cast<Coder1*>(base::get_coder(1)); }
	void    set_coder1(Coder1* c1, bool own_coder = false) { base::set_coder(1, c1, own_coder); }

	Coder2* get_coder2() const { return static_cast<Coder2*>(base::get_coder(2)); }
	void    set_coder2(Coder2* c2, bool own_coder = false) { base::set_coder(2, c2, own_coder); }

	Coder3* get_coder3() const { return static_cast<Coder3*>(base::get_coder(3)); }
	void    set_coder3(Coder3* c3, bool own_coder = false) { base::set_coder(3, c3, own_coder); }

	Coder4* get_coder4() const { return static_cast<Coder4*>(base::get_coder(4)); }
	void    set_coder4(Coder4* c4, bool own_coder = false) { base::set_coder(4, c4, own_coder); }

	Coder5* get_coder5() const { return static_cast<Coder5*>(base::get_coder(5)); }
	void    set_coder5(Coder5* c5, bool own_coder = false) { base::set_coder(5, c5, own_coder); }

	Coder6* get_coder6() const { return static_cast<Coder6*>(base::get_coder(6)); }
	void    set_coder6(Coder6* c6, bool own_coder = false) { base::set_coder(6, c6, own_coder); }

protected:
	virtual const char* get_class_name() const { return "CoderPipe6"; }
};

/// GenericCoderPipe ////////////////////////////////////////////////////////////////////////

// no __exportC since all members in header file
class GenericCoderPipe : public coder_pipe {
public:
	typedef coder_pipe base;

public:
	GenericCoderPipe(unsigned num_coders = 1) : base(num_coders) {}

	// these functions are protected in the base class, the only reason for 
	// creating GenericCoderPipe is to make them public
	coder_base* get_coder(unsigned coder_idx) const 
	{ return base::get_coder(coder_idx); }
	void set_coder(unsigned coder_idx, coder_base* coder, bool own_coder = false) 
	{ base::set_coder(coder_idx, coder, own_coder); }

protected:
	virtual const char* get_class_name() const { return "GenericCoderPipe"; }
	// inherited from base: virtual id_type get_id() const;
};

#if LEDA_ROOT_INCL_ID == 500024
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif

LEDA_END_NAMESPACE

#endif
