/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  block_cipher_factory.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:44:36 $

#ifndef _LEDA_BLOCK_CIPHER_FACTORY_H
#define _LEDA_BLOCK_CIPHER_FACTORY_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500708
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/coding/crypter_base.h>
#include <LEDA/core/list.h>
#include <LEDA/core/h_array.h>

LEDA_BEGIN_NAMESPACE

/// block_cipher_factory ////////////////////////////////////////////////////////////////////

class __exportC block_cipher_factory {
public:
	typedef block_cipher_base::id_type id_type;
	typedef block_cipher_base* (*cipher_construction_func)(id_type id);
		// id: id of the cipher to construct (-> same function can be used for several ciphers)

public:
	block_cipher_factory();

	block_cipher_base* construct_cipher_by_id(id_type id);

	void register_id(id_type id, cipher_construction_func ctor);

	list<block_cipher_base*> get_all_ciphers();

	void print_registered_ciphers(ostream& out = cout); // for debugging

private:
	void init();
	void destroy();

private:
	h_array<id_type, cipher_construction_func>* pCipherConstructorMap;

private:
#if defined(LEDA_MULTI_THREAD)
	static leda_mutex mutex;
	void lock()   { mutex.lock(); }
	void unlock() { mutex.unlock(); }
#else
	void lock()   {}
	void unlock() {}
#endif

	friend class block_cipher_factory_init;
};

extern __exportD block_cipher_factory BlockCipherFactory;

/// block_cipher_factory_init ///////////////////////////////////////////////////////////////

class __exportC block_cipher_factory_init {
	static unsigned Count;
public:
	block_cipher_factory_init();
	~block_cipher_factory_init();
};

static block_cipher_factory_init _init_block_cipher_factory_;

#if LEDA_ROOT_INCL_ID == 500708
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif

LEDA_END_NAMESPACE

#endif
