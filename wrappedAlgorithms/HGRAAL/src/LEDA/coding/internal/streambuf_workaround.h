/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  streambuf_workaround.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.7 $  $Date: 2005/04/14 10:44:39 $

#ifndef _LEDA_STREAMBUF_WORKAROUND_H
#define _LEDA_STREAMBUF_WORKAROUND_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500789
#include <LEDA/internal/PREAMBLE.h>
#endif

// work-around for Microsoft .NET
#if (_MSC_VER >= 1300)
#define LEDA_COMPRESSION_WA7
#endif

LEDA_BEGIN_NAMESPACE

// work-around for MSVC 6.0 and CXX 5.1 (alpha) and HPUX aCC (see also LEDA/internal/PREAMBLE.h)
#ifdef LEDA_COMPRESSION_WA1
class __exportC local_streambuf : public streambuf {
public:
	virtual streamsize xsgetn(char* buffer, streamsize count);
	virtual streamsize xsputn(const char* buffer, streamsize count);
	virtual int sync() { return 0; }

protected:
	virtual int uflow() { int c = underflow(); return c < 0 ? c : sbumpc(); }
};
#endif

// work-around for Microsoft .NET
#ifdef LEDA_COMPRESSION_WA7
class local_streambuf : public streambuf {};
#endif

#if LEDA_ROOT_INCL_ID == 500789
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif

LEDA_END_NAMESPACE

#endif // ifndef _LEDA_STREAMBUF_WORKAROUND_H

#if defined(LEDA_COMPRESSION_WA1) || defined(LEDA_COMPRESSION_WA7)
#define LEDA_STREAMBUF local_streambuf
#else
#define LEDA_STREAMBUF streambuf
#endif
