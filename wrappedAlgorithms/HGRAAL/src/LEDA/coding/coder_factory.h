/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  coder_factory.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:44:37 $

#ifndef _LEDA_CODER_FACTORY_H
#define _LEDA_CODER_FACTORY_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500395
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/coding/coder_base.h>
#include <LEDA/core/list.h>
#include <LEDA/core/h_array.h>

#if (defined(_MSC_VER) && _MSC_VER < 1300) || defined(__DECCXX)
// work-around for MSVC 6.0 and CXX 5.1 (alpha)
#define LEDA_COMPRESSION_WA3
#endif

LEDA_BEGIN_NAMESPACE

class __exportC block_cipher_mode_base;

/// coder_factory ///////////////////////////////////////////////////////////////////////////

class __exportC coder_factory {
public:
	typedef coder_base::id_type id_type;
	typedef coder_base* (*coder_construction_func)(id_type id, bool as_standalone);
		// id: id of the coder to construct (-> same function can be used for several coders)
		// as_standalone: states if coder will be used as standalone coder
		// (The function may return NIL, if the coder cannot be constructed (as standalone).)

public:
	coder_factory();

	coder_base* construct_coder_by_id(id_type id);

	void register_id(id_type id, coder_construction_func ctor);

	list<coder_base*>             get_all_coders();
	list<coder_base*>             get_standalone_coders();
	list<block_cipher_mode_base*> get_block_cipher_modes();

	void print_registered_coders(ostream& out = cout); // for debugging

private:
	void init();
	void destroy();

private:
	h_array<id_type, coder_construction_func>* pCoderConstructorMap;

private:
#if defined(LEDA_MULTI_THREAD)
	static leda_mutex mutex;
	void lock()   { mutex.lock(); }
	void unlock() { mutex.unlock(); }
#else
	void lock()   {}
	void unlock() {}
#endif

	friend class coder_factory_init;
};

extern __exportD coder_factory CoderFactory;

#ifndef LEDA_COMPRESSION_WA3
template <class Coder>
coder_base* coder_constructor(coder_factory::id_type id, bool as_standalone)
{
	coder_base* p_coder = new Coder;
	if (as_standalone && !p_coder->is_standalone_coder()) { delete p_coder; p_coder = 0; }
	return p_coder;
}

template <class Coder>
class register_coder {
public:
	register_coder() { CoderFactory.register_id(Coder::ID, &coder_constructor<Coder>); }
};
#endif

class __exportC coder_factory_init {
	static unsigned Count;
public:
	coder_factory_init();
	~coder_factory_init();
};

static coder_factory_init _init_coder_factory_;

#if LEDA_ROOT_INCL_ID == 500395
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif

LEDA_END_NAMESPACE

#endif
