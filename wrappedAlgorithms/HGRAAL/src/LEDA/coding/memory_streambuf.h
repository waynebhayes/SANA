/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  memory_streambuf.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.8 $  $Date: 2005/04/14 10:44:38 $

#ifndef _LEDA_MEMORY_STREAMBUF_H
#define _LEDA_MEMORY_STREAMBUF_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500409
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/coding/internal/streambuf_workaround.h>

LEDA_BEGIN_NAMESPACE

/// memory_streambuf ////////////////////////////////////////////////////////////////////////

/*{\Manpage {memory_streambuf} {} {Memory Streambuffer} {mb}}*/

class __exportC memory_streambuf : public LEDA_STREAMBUF {
/*{\Mdefinition
An object |\Mvar| of the class |\Mname| can be used as source or target stream 
for a coder and allows encoding and decoding in memory (i.e.~without file 
accesses).
Every read or write operation is forwarded to a buffer in memory. This buffer 
is directly accessible by the user. The length of this buffer is called the 
\emph{capacity} of |\Mvar|.
The \emph{size} of |\Mvar| is the number of characters that can be read from
or written to the stream before an underflow/overflow occurs. (In that case the
corresponding stream operation returns EOF (end-of-file).)
Thus the capacity is the maximum size that |\Mvar| can have without 
reallocating its buffer.
}*/
public:
	typedef LEDA_STREAMBUF base;

public:
/*{\Mcreation}*/

	memory_streambuf(streamsize buf_sz = 0, bool wipe_buf = false)
	 : Buffer(0), BufferCapacity(0), OwnBuffer(false), WipeBuffer(wipe_buf)
	{ set_size(buf_sz); }
	/*{\Mcreate creates an object |\Mvar| with capacity and size |buf_sz|.
	The parameter |wipe_buf| determines whether |wipe_buffer| is called 
	when the buffer is not needed any more.}*/

	memory_streambuf(char* buf, streamsize buf_sz, bool own_buf = false, bool wipe_buf = false)
	 : Buffer(0), BufferCapacity(0), OwnBuffer(false), WipeBuffer(wipe_buf)
	{ set_buffer(buf, buf_sz, own_buf); }
	/*{\Mcreate creates an object |\Mvar| with the memory buffer |buf| and sets
	the capacity and the size to |buf_sz|. If |own_buf|/|wipe_buf| is true, 
	then |buf| is deleted/wiped by |\Mvar| when it is not used anymore.
	Note that |buf| is not copied but it is used as the internal buffer of 
	|\Mvar|.}*/

	virtual ~memory_streambuf() { free_buffer(); }

/*{\Moperations}*/

	void reset();
	/*{\Mop moves the internal get and put pointers to the beginning of the 
	buffer and clears the underflow/overflow flags.}*/

	streamsize get_capacity() const { return BufferCapacity; }
	/*{\Mop returns the current capacity.}*/

	streamsize get_size() const { return Size; }
	/*{\Mop returns the current size.}*/

	void set_size(streamsize n);
	/*{\Mop changes the size to |n| and calls |reset|. 
	(If |n| exceeds the capacity then a new buffer with sufficient capacity 
	is allocated. The contents of the old buffer are not copied.)}*/

	void truncate(streamsize n);
	/*{\Mop also changes the size to |n|, but never allocates a new buffer.\\
	\precond $n \leq $ current capacity.}*/

	char* get_buffer() const { return Buffer; }
	/*{\Mop returns the memory buffer of |\Mvar|.}*/

	void set_buffer(char* buf, streamsize buf_sz, bool own_buf = false, bool wipe_buf = false);
	/*{\Mop makes |buf| the new memory buffer and changes capacity and size to
	|buf_sz|. (The meaning of |own_buf| and |wipe_buf| is the same as in the 
	second contructor.)}*/

	void wipe_buffer();
	/*{\Mop like reset, in addition all bytes in the memory buffer are set to zero.}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	streamsize in_avail() const;
*/
	/*{\Mop returns how many characters can be read from |\Mvar| before an 
	underflow occurs.}*/

	streamsize in_count() const;
	/*{\Mop returns how many characters have been read from |\Mvar| since 
	the last reset.}*/

	streamsize out_avail() const { return streamsize( epptr()-pptr() ); }
	/*{\Mop returns how many characters can be written to |\Mvar| before an 
	overflow occurs.}*/

	streamsize out_count() const;
	/*{\Mop returns how many characters have been written to |\Mvar| since 
	the last reset.}*/

	char* get_remaining_in_buffer() const { return gptr(); }
	char* get_remaining_out_buffer() const { return pptr(); }

	bool had_underflow() const { return UnderflowOccurred; }
	/*{\Mop returns whether an underflow has occurred since the last reset.}*/

	bool had_overflow() const { return OverflowOccurred; }
	/*{\Mop returns whether an overflow has occurred since the last reset.}*/

protected:
	// the following functions are inherited from streambuf
#ifdef LEDA_COMPRESSION_WA1
// work-around for MSVC 6.0 and CXX 5.1 (alpha) and HPUX aCC (see also LEDA/internal/PREAMBLE.h)
public:
    virtual streampos seekoff(streamoff off, ios::seek_dir way, int mode) { return seekoff(off, way, ios::openmode(mode)); }
	virtual streampos seekpos(streampos sp, int mode) { return seekpos(sp, ios::openmode(mode)); }
#endif
	virtual streampos seekoff(streamoff off, ios::seekdir way, ios::openmode which);
	virtual streampos seekpos(streampos sp, ios::openmode which);

protected:
	virtual streambuf* setbuf(char* buf, streamsize n) { set_buffer(buf, n); return this; }
	virtual int overflow(int c) { OverflowOccurred = true; return EOF; }
		// called when put-buffer buffer full  (puts the character to the stream)
	virtual int underflow() { UnderflowOccurred = true; return EOF; }
		// called when get-buffer buffer empty (gets a character from stream, but doesn' point past it)
	virtual int uflow() { UnderflowOccurred = true; return EOF; }
		// called when get-buffer buffer empty (gets a character from stream, and points past it)

private:
	void free_buffer();
	streampos get_current_pos(ios::openmode which) const;

private:
	char*         Buffer;
	streamsize    BufferCapacity;
	streamsize    Size;
	mutable char* Max_gptr;
	mutable char* Max_pptr;
	bool          OwnBuffer;
	bool          WipeBuffer;
	bool          OverflowOccurred;
	bool          UnderflowOccurred;
};

#undef LEDA_STREAMBUF

#if LEDA_ROOT_INCL_ID == 500409
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif

LEDA_END_NAMESPACE

#endif
