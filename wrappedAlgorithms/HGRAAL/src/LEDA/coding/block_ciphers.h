/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  block_ciphers.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:44:36 $

#ifndef _LEDA_BLOCKCIPHERS_H
#define _LEDA_BLOCKCIPHERS_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500706
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/coding/crypter_base.h>

LEDA_BEGIN_NAMESPACE

/// Blowfish ////////////////////////////////////////////////////////////////////////////////

class __exportC Blowfish : public block_cipher_base {
public:
	typedef block_cipher_base base;
	typedef base::direction direction;
	enum { ID = 0x434200, BlkSz = 8, DefKeyBytes = 16, MinKeyBytes = 4, MaxKeyBytes = 56 };

	enum { Rounds = 16 };
	struct state {
		uint32 P[Rounds + 2];
		uint32 S[4][256];
	};

public:
	Blowfish();
	~Blowfish();

	virtual uint16 get_block_size() const { return BlkSz; }

	virtual uint16 get_accepted_key_size(uint16& min, uint16& max) const 
	{ min = MinKeyBytes; max = MaxKeyBytes; return DefKeyBytes; }

	virtual void set_key(const CryptKey& key, direction which);
			void set_key(const CryptKey& key) { set_key(key, d_both); }

	virtual void encipher(const byte* src, byte* tgt);
	virtual void encipher(byte* blk);
	virtual void decipher(const byte* src, byte* tgt);
	virtual void decipher(byte* blk);

	virtual string  get_description() const { return "Blowfish"; }
	virtual id_type get_id() const { return ID; }

private:
	uint32 F(uint32);
	void encipher_step(uint32* xl, uint32* xr);
	void decipher_step(uint32* xl, uint32* xr);

private:
	state* pState;
};


/// Twofish /////////////////////////////////////////////////////////////////////////////////

class __exportC Twofish : public block_cipher_base {
public:
	typedef block_cipher_base base;
	typedef base::direction direction;
	enum { ID = 0x434201, BlkSz = 16, DefKeyBytes = 16, MinKeyBytes = 16, MaxKeyBytes = 32 };

	struct state {
		uint32 ExpKey[40];
		uint32 FKT[4][256]; // Fully Keyed Table
	};

public:
	Twofish();
	~Twofish();

	virtual uint16 get_block_size() const { return BlkSz; }

	virtual uint16 get_accepted_key_size(uint16& min, uint16& max) const 
	{ min = MinKeyBytes; max = MaxKeyBytes; return DefKeyBytes; }

	virtual void set_key(const CryptKey& key, direction which);
			void set_key(const CryptKey& key) { set_key(key, d_both); }

	virtual void encipher(const byte* src, byte* tgt);
	virtual void encipher(byte* blk);
	virtual void decipher(const byte* src, byte* tgt);
	virtual void decipher(byte* blk);

	virtual string  get_description() const { return "Twofish"; }
	virtual id_type get_id() const { return ID; }

private:
	uint32 poly_mult_mod(uint32 a, uint32 b, uint32 mod);
	uint32 RS_multiply(byte vector[8]);
	uint32 h(uint32 X, uint32 L[4], int k);
	void key_fully(uint32 S[4], int k);

private:
	state* pState;
};


/// Rijndael ////////////////////////////////////////////////////////////////////////////////

class __exportC Rijndael : public block_cipher_base {
public:
	typedef block_cipher_base base;
	typedef base::direction direction;
	enum { ID = 0x434202, BlkSz = 16, DefKeyBytes = 16, MinKeyBytes = 16, MaxKeyBytes = 32 };

public:
	Rijndael();
	~Rijndael();

	virtual uint16 get_block_size() const { return BlkSz; }

	virtual uint16 get_accepted_key_size(uint16& min, uint16& max) const 
	{ min = MinKeyBytes; max = MaxKeyBytes; return DefKeyBytes; }

	virtual void set_key(const CryptKey& key, direction which);

	virtual void encipher(const byte* src, byte* tgt);
	virtual void encipher(byte* blk);
	virtual void decipher(const byte* src, byte* tgt);
	virtual void decipher(byte* blk);

	virtual string  get_description() const { return "Rijndael"; }
	virtual id_type get_id() const { return ID; }

private:
	void key_setup_enc(const CryptKey& key);
	void key_setup_dec(const CryptKey& key);

private:
	int    Nr; // # rounds
	uint32 Rk[4 * (14+1)]; // 240 Bytes
};

typedef Rijndael AES;

#if LEDA_ROOT_INCL_ID == 500706
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif

LEDA_END_NAMESPACE

#endif
