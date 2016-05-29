/*******************************************************************************
+
+  LEDA 5.0.1
+
+
+  strbuf_helpers.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.3 $  $Date: 2005/04/14 10:44:39 $

#ifndef _LEDA_STRBUF_HELPERS_H
#define _LEDA_STRBUF_HELPERS_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500713
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/coding/coder_base.h>

LEDA_BEGIN_NAMESPACE

// NOTE: None of these classes is exported in the DLL.
//       If you want to export them you have to apply LEDA_COMPRESSION_WA7.

/// eat_all_streambuf ///////////////////////////////////////////////////////////////////////

// eat_all_streambuf consumes all its input without doing anything ...

class eat_all_streambuf : public streambuf {
protected:
	virtual int overflow(int c) { return c; }
	virtual int underflow() { return EOF; } // just a dummy
};


/// get_feeder //////////////////////////////////////////////////////////////////////////////

class get_feeder : public streambuf {
public:
	get_feeder(streamoff to_feed) : ToFeed(to_feed)
	{ setg(0,0,0); }

	bool fed_everything() const { return ToFeed <= 0; }

protected:
	virtual int uflow()
	{ return ToFeed-- != 0 ? 0 : coder_base::StreamBlocked; }

	virtual int underflow()
	{ setg(0,0,0); return uflow(); } // hack (should not be called)

	virtual streamsize xsgetn(char* buffer, streamsize count);

	virtual int overflow(int c) { return EOF; } // just a dummy

private:
	streamoff ToFeed;
};

/// put_eater ///////////////////////////////////////////////////////////////////////////////

/*
	The following class is used to skip a certain amount of put-operations
	on the target stream (in function decode_seekskip below).
	(Since we set the put pointers to zero, this means that every put yields
	an overflow, which we count.)
	NOTE: 
		If ToEat is negative, we will basically never return StreamBlocked,
		i.e. we consume until there is nothing left to consume.
*/
class put_eater : public streambuf {
public:
	put_eater(streamoff to_eat) : ToEat(to_eat), Eaten(0) 
	{ setp(0,0); }

	streamoff get_number_of_puts() const { return Eaten; }

protected:
	virtual int overflow(int c)
	{ ++Eaten; return --ToEat != 0 ? c : coder_base::StreamBlocked; }

	virtual streamsize xsputn(const char* buffer, streamsize count);

	virtual int underflow() { return EOF; } // just a dummy

private:
	streamoff ToEat, Eaten;
};

#if LEDA_ROOT_INCL_ID == 500713
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif

LEDA_END_NAMESPACE

#endif
