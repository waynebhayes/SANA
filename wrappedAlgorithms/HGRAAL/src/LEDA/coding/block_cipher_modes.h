/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  block_cipher_modes.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:44:36 $

#ifndef _LEDA_BLOCK_CIPHER_MODES_H
#define _LEDA_BLOCK_CIPHER_MODES_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500707
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/coding/crypter_base.h>
#include <LEDA/core/b_queue.h> // for ReadAhead

LEDA_BEGIN_NAMESPACE

/// block_cipher_mode_base //////////////////////////////////////////////////////////////////

class __exportC block_cipher_mode_base : public crypter_sym_key_base {
public:
	typedef crypter_sym_key_base base;

public:
	block_cipher_mode_base(streambuf* src_stream = 0, streambuf* tgt_stream = 0, bool own_streams = false,
		                   block_cipher_base* cipher = 0, bool own_cipher = false);
	block_cipher_mode_base(const char* src_file_name, const char* tgt_file_name,
		                   block_cipher_base* cipher = 0, bool own_cipher = false);
	~block_cipher_mode_base();

	virtual bool is_standalone_coder() const { return false; }

	virtual string get_description() const;

	virtual uint16 get_accepted_key_size(uint16& min, uint16& max) const;
	virtual void   set_key(const CryptKey& key);
	virtual bool   has_key() const { return !Key.is_empty(); }
	CryptKey       get_key() const { return Key; }

	block_cipher_base* get_block_cipher() const { return pBlockCipher; }
	virtual void set_block_cipher(block_cipher_base* cipher, bool own_cipher);

	uint32 calculate_length_of_encoded_data(uint32 input_length) const;

public:
	static void memxor(byte* tgt, const byte* src, size_t sz); // tgt ^= src
	
protected:
	virtual void encoding_finished();
	virtual void decoding_finished();
	virtual void do_reset(reset_type t);

	virtual void write_id_to_tgt_stream();
	virtual void read_and_check_id_in_src_stream();
	virtual uint32 get_id_length(bool& is_exact) const;
	virtual void finish_construction_from_stream(id_type id_in_stream, streambuf* stream, streambuf* additional_data);

protected:
	byte*  get_block() { return Block; }

	virtual uint32 calculate_length_of_encoded_data_without_prolog(uint32 input_length, bool& is_exact) const = 0;
		// prolog = id & header

private:
	block_cipher_base* pBlockCipher;
	bool               OwnBlockCipher;
	CryptKey           Key;
	secure_byte_array  Block;
};

/// bcm_stream_cipher_base //////////////////////////////////////////////////////////////////
// base class for all block cipher modes which turn a block cipher into a stream cipher

class __exportC bcm_stream_cipher_base : public block_cipher_mode_base {
public:
	typedef block_cipher_mode_base base;
	typedef block_cipher_base::direction bc_direction;

public:
	bcm_stream_cipher_base(streambuf* src_stream = 0, streambuf* tgt_stream = 0, bool own_streams = false,
						   block_cipher_base* cipher = 0, bool own_cipher = false);
	bcm_stream_cipher_base(const char* src_file_name, const char* tgt_file_name,
						   block_cipher_base* cipher = 0, bool own_cipher = false);

protected: // inherited from coder_base
	virtual void init_encoding();
	virtual void encode_stream();

	virtual void init_decoding();
	virtual void decode_stream();
	        void prepare_resume_decoding(); // for use in decode_seek

protected: // inherited from block_cipher_mode_base
	virtual uint32 calculate_length_of_encoded_data_without_prolog(uint32 input_length, bool& is_exact) const;

protected: // new virtual functions
	virtual bc_direction block_cipher_dir_for_enciphering() const
	{ return block_cipher_base::d_encipher; }
	virtual bc_direction block_cipher_dir_for_deciphering() const
	{ return block_cipher_base::d_decipher; }

	virtual void encipher_block() = 0;
	virtual void decipher_block() = 0;

private:
	bool  LastBlockFetched;
	byte* pCurrent;
	byte* pStop;
};

/// bcm_stream_cipher_with_iv_base //////////////////////////////////////////////////////////

class __exportC bcm_stream_cipher_with_iv_base : public bcm_stream_cipher_base {
public:
	typedef bcm_stream_cipher_base base;

public:
	bcm_stream_cipher_with_iv_base(streambuf* src_stream = 0, streambuf* tgt_stream = 0, bool own_streams = false,
								   block_cipher_base* cipher = 0, bool own_cipher = false);
	bcm_stream_cipher_with_iv_base(const char* src_file_name, const char* tgt_file_name,
								   block_cipher_base* cipher = 0, bool own_cipher = false);

	void        set_initialization_vector(const byte* iv) { ExternalIV = iv; }
	const byte* get_initialization_vector() const { return ExternalIV; }

protected: // inherited from coder_base
	virtual void init_encoding();
	virtual void encoding_finished();
	virtual void init_decoding();
	virtual void decoding_finished();

	virtual void do_reset(reset_type t);

protected: // inherited from block_cipher_mode_base
	virtual uint32 calculate_length_of_encoded_data_without_prolog(uint32 input_length, bool& is_exact) const;

protected:
	virtual void finish_initialization_vector(byte* iv) {}
		// derived modes may for example encrypt the raw iv before its use

	byte* get_feedback_blk() { return FeedbackBlock; }
	byte* get_tmp_blk() { return TempBlock; }

private:
	secure_byte_array FeedbackBlock;
	secure_byte_array TempBlock;
	const byte*       ExternalIV;
};

/// bcm_str_cipher_with_cipher_feedb_base ///////////////////////////////////////////////////

class __exportC bcm_str_cipher_with_cipher_feedb_base : public bcm_stream_cipher_with_iv_base {
public:
	typedef bcm_stream_cipher_with_iv_base base;

public:
	bcm_str_cipher_with_cipher_feedb_base(streambuf* src_stream = 0, streambuf* tgt_stream = 0, bool own_streams = false,
	                                      block_cipher_base* cipher = 0, bool own_cipher = false);
	bcm_str_cipher_with_cipher_feedb_base(const char* src_file_name, const char* tgt_file_name, 
	                                      block_cipher_base* cipher = 0, bool own_cipher = false);

	virtual bool is_optimizing_seeks() const { return true; }

protected:
	virtual streampos decode_seek(streampos current_pos, streampos new_pos);
};

/// ecb_mode ////////////////////////////////////////////////////////////////////////////////

class __exportC ecb_mode : public bcm_stream_cipher_base {
public:
	typedef bcm_stream_cipher_base base;
	enum { ID = 0x434c11 };

public:
	ecb_mode(streambuf* src_stream = 0, streambuf* tgt_stream = 0, bool own_streams = false,
		     block_cipher_base* cipher = 0, bool own_cipher = false)
	 : base(src_stream, tgt_stream, own_streams, cipher, own_cipher) {}
	ecb_mode(const char* src_file_name, const char* tgt_file_name,
		     block_cipher_base* cipher = 0, bool own_cipher = false)
	 : base(src_file_name, tgt_file_name, cipher, own_cipher) {}

	virtual bool is_optimizing_seeks() const { return true; }

protected: // inherited from coder_base
	virtual const char* get_class_name() const { return "ecb_mode"; }
	virtual id_type get_id() const { return ID; }

	virtual streampos decode_seek(streampos current_pos, streampos new_pos);

protected: // inherited from base
	virtual void encipher_block() { get_block_cipher()->encipher(get_block()); }
	virtual void decipher_block() { get_block_cipher()->decipher(get_block()); }
};

/// cbc_mode ////////////////////////////////////////////////////////////////////////////////

class __exportC cbc_mode : public bcm_str_cipher_with_cipher_feedb_base {
public:
	typedef bcm_str_cipher_with_cipher_feedb_base base;
	enum { ID = 0x434c12 };

public:
	cbc_mode(streambuf* src_stream = 0, streambuf* tgt_stream = 0, bool own_streams = false,
		     block_cipher_base* cipher = 0, bool own_cipher = false)
	 : base(src_stream, tgt_stream, own_streams, cipher, own_cipher) {}
	cbc_mode(const char* src_file_name, const char* tgt_file_name,
		     block_cipher_base* cipher = 0, bool own_cipher = false)
	 : base(src_file_name, tgt_file_name, cipher, own_cipher) {}

protected: // inherited from coder_base
	virtual const char* get_class_name() const { return "cbc_mode"; }
	virtual id_type get_id() const { return ID; }

protected: // inherited from base
	virtual void finish_initialization_vector(byte* iv);
	virtual void encipher_block();
	virtual void decipher_block();
};

/// cfb_mode ////////////////////////////////////////////////////////////////////////////////

class __exportC cfb_mode : public bcm_str_cipher_with_cipher_feedb_base {
public:
	typedef bcm_str_cipher_with_cipher_feedb_base base;
	enum { ID = 0x434c13 };

public:
	cfb_mode(streambuf* src_stream = 0, streambuf* tgt_stream = 0, bool own_streams = false,
		     block_cipher_base* cipher = 0, bool own_cipher = false)
	 : base(src_stream, tgt_stream, own_streams, cipher, own_cipher) {}
	cfb_mode(const char* src_file_name, const char* tgt_file_name,
		     block_cipher_base* cipher = 0, bool own_cipher = false)
	 : base(src_file_name, tgt_file_name, cipher, own_cipher) {}

protected: // inherited from coder_base
	virtual const char* get_class_name() const { return "cfb_mode"; }
	virtual id_type get_id() const { return ID; }

protected: // inherited from base
	virtual bc_direction block_cipher_dir_for_deciphering() const
	{ return block_cipher_base::d_encipher; } // uses block_cipher in enciphering mode!

	virtual void finish_initialization_vector(byte* iv);
	virtual void encipher_block();
	virtual void decipher_block();
};

/// ofb_mode ////////////////////////////////////////////////////////////////////////////////

class __exportC ofb_mode : public bcm_stream_cipher_with_iv_base {
public:
	typedef bcm_stream_cipher_with_iv_base base;
	enum { ID = 0x434c14 };

public:
	ofb_mode(streambuf* src_stream = 0, streambuf* tgt_stream = 0, bool own_streams = false,
		     block_cipher_base* cipher = 0, bool own_cipher = false)
	 : base(src_stream, tgt_stream, own_streams, cipher, own_cipher) {}
	ofb_mode(const char* src_file_name, const char* tgt_file_name,
		     block_cipher_base* cipher = 0, bool own_cipher = false)
	 : base(src_file_name, tgt_file_name, cipher, own_cipher) {}

protected: // inherited from coder_base
	virtual const char* get_class_name() const { return "ofb_mode"; }
	virtual id_type get_id() const { return ID; }

protected: // inherited from base
	virtual bc_direction block_cipher_dir_for_deciphering() const
	{ return block_cipher_base::d_encipher; } // uses block_cipher in enciphering mode!

	virtual void encipher_block();
	virtual void decipher_block();
};

/// omac_mode ///////////////////////////////////////////////////////////////////////////////

class __exportC omac_mode : public block_cipher_mode_base {
public:
	typedef block_cipher_mode_base base;
	typedef block_cipher_base::direction bc_direction;
	enum { MaxBlkSz = 256/8 };
	enum { ID = 0x434c15 };
	
public:
	omac_mode(streambuf* src_stream = 0, streambuf* tgt_stream = 0, bool own_streams = false,
			  block_cipher_base* cipher = 0, bool own_cipher = false);
	omac_mode(const char* src_file_name, const char* tgt_file_name,
			  block_cipher_base* cipher = 0, bool own_cipher = false);

	virtual bool is_optimizing_seeks() const { return true; }
	virtual bool is_secrecy_sensitive() const { return false; }
		// it's authentication, no ciphering

	const byte* check();
	const byte* compute_MAC(string str);
	string      get_MAC_as_hex_string() const;

	void set_MAC_in_stream_flag(bool mac_in_stream);
	bool get_MAC_in_stream_flag() const { return MACInStream; }

	bool MAC_is_valid() const { return (MACState == mac_okay) && !failed(); }

public: // inherited from base
	virtual void set_block_cipher(block_cipher_base* cipher, bool own_cipher);

protected: // inherited from coder_base
	virtual const char* get_class_name() const { return "omac_mode"; }
	virtual id_type get_id() const { return ID; }

	virtual void init_encoding();
	virtual void encode_stream();

	virtual void init_decoding();
	virtual void decode_stream();

	virtual void do_reset(reset_type t);

	virtual streampos decode_seek(streampos current_pos, streampos new_pos);

protected: // output length calculations
	virtual uint32 get_prolog_length(bool& is_exact) const;
	virtual uint32 calculate_length_of_encoded_data_without_prolog(uint32 input_length, bool& is_exact) const;

private:
	void init_processing(bool read_ahead);
	bool process_stream();
	void multiply_blk_by_u();

private:
	enum mac_state { mac_invalid, mac_okay, mac_computing };

private:
	secure_byte_array MAC;
	byte*             pCurrent;
	byte*             pStop;
	bool              MACInStream;
	mac_state         MACState;
	b_queue<byte>     ReadAheadBuffer;
};

#if LEDA_ROOT_INCL_ID == 500707
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif

LEDA_END_NAMESPACE

#endif
