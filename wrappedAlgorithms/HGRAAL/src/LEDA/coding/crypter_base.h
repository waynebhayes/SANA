/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  crypter_base.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:44:38 $

#ifndef _LEDA_CRYPTER_BASE_H
#define _LEDA_CRYPTER_BASE_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500705
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/coding/coder_base.h>
#include <LEDA/coding/crypt_key.h>

LEDA_BEGIN_NAMESPACE

/// crypter_sym_key_base //////////////////////////////////////////////////////////////////////

class __exportC crypter_sym_key_base : public coder_base {
public:
	typedef coder_base base;

public:
	crypter_sym_key_base(streambuf* src_stream = 0, streambuf* tgt_stream = 0, bool own_streams = false)
	 : base(src_stream, tgt_stream, own_streams) {}
	crypter_sym_key_base(const char* src_file_name, const char* tgt_file_name)
	 : base(src_file_name, tgt_file_name) {}

	virtual bool is_secrecy_sensitive() const { return true; }

	        uint16 get_default_key_size() const { uint16 dummy; return get_accepted_key_size(dummy, dummy); }
	virtual uint16 get_accepted_key_size(uint16& min, uint16& max) const = 0;
	virtual void set_key(const CryptKey& key) = 0;
	virtual bool has_key() const = 0;

	static void wipe_memory(void* mem, uint32 sz) { secure_byte_array::wipe_memory(mem, sz); }

protected:
	virtual void init_encoding() { base::init_encoding(); if (!has_key()) error(); }
	virtual void init_decoding() { base::init_decoding(); if (!has_key()) error(); }
};

/// block_cipher_base ///////////////////////////////////////////////////////////////////////

class __exportC block_cipher_base : public coder_types {
public:
	enum direction { d_encipher = 1, d_decipher = 2, d_both = d_encipher | d_decipher };

public:
	virtual ~block_cipher_base() {}

	virtual uint16 get_block_size() const = 0; // in bytes

	        uint16 get_default_key_size() const { uint16 dummy; return get_accepted_key_size(dummy, dummy); }
	virtual uint16 get_accepted_key_size(uint16& min, uint16& max) const = 0;
	virtual void set_key(const CryptKey& key, direction which) = 0;
	        void set_enc_key(const CryptKey& key) { set_key(key, d_encipher); }
	        void set_dec_key(const CryptKey& key) { set_key(key, d_decipher); }

	virtual void encipher(const byte* src, byte* tgt) = 0;
	virtual void encipher(byte* blk) = 0;
	virtual void decipher(const byte* src, byte* tgt) = 0;
	virtual void decipher(byte* blk) = 0;

	virtual id_type get_id() const = 0;
	virtual string  get_description() const = 0;

	static void wipe_memory(void* mem, uint32 sz) { secure_byte_array::wipe_memory(mem, sz); }
};

#if LEDA_ROOT_INCL_ID == 500705
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif

LEDA_END_NAMESPACE

#endif
