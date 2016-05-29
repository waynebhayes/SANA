/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  stream_ciphers.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:44:39 $

#ifndef _LEDA_STREAM_CIPHERS_H
#define _LEDA_STREAM_CIPHERS_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500710
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/coding/block_cipher_modes.h>
#include <LEDA/coding/block_ciphers.h>

LEDA_BEGIN_NAMESPACE

/// ECBCoder ////////////////////////////////////////////////////////////////////////////////

template <class BlkCipher = Twofish>
class ECBCoder : public ecb_mode {
public:
	typedef ecb_mode base;
	typedef BlkCipher block_cipher;

public:
	ECBCoder(streambuf* src_stream = 0, streambuf* tgt_stream = 0, bool own_streams = false)
	 : base(src_stream, tgt_stream, own_streams, new BlkCipher, true) {}

	ECBCoder(const char* src_file_name, const char* tgt_file_name)
	 : base(src_file_name, tgt_file_name, new BlkCipher, true) {}

	BlkCipher* get_block_cipher() const { return static_cast<BlkCipher*>(base::get_block_cipher()); }

protected:
	virtual const char* get_class_name() const { return "ECBCoder"; }
	// inherited from base: virtual id_type get_id() const;

	virtual void set_block_cipher(block_cipher_base* cipher, bool own_cipher) {} // hide
};

/// CBCCoder ////////////////////////////////////////////////////////////////////////////////

/*{\Manpage {CBCCoder} {BlkCipher} {Example for a Stream-Cipher} {C}}*/

template <class BlkCipher = Twofish>
class CBCCoder : public cbc_mode {
/*{\Mdefinition
A stream-cipher is a coder that encrypts and decrypts data streams.
In this section we discuss the interface of such a coder by an
example: |\Mname|. Every stream-cipher in LEDA uses a block-cipher |BlkCipher|
which is specified as a template parameter.
(More information about stream-ciphers and block-ciphers can be found in 
Section~\ref{stream-ciphers}.)
The following stream-ciphers are available: |ECBCoder|, |CBCCoder|, |CFBCoder|
and |OFBCoder|. All of them, except for |OFBCoder|, support fast seek 
operations (see Section~\ref{decoding_ifstream}).
The available block-ciphers are: |Blowfish|, |Twofish| and |Rijndael|.
}*/
public:
	typedef cbc_mode base;
	typedef BlkCipher block_cipher;

public:
/*{\Mcreation}*/

	CBCCoder(streambuf* src_stream = 0, streambuf* tgt_stream = 0, bool own_streams = false)
	 : base(src_stream, tgt_stream, own_streams, new BlkCipher, true) {}
	/*{\Mcreate creates an instance |\Mvar| which uses the given source 
	and target streams. If |own_streams| is set, then |\Mvar| is 
	responsible for the destruction of the streams, otherwise the pointers 
	|src_stream| and |tgt_stream| must be valid during the life-time of 
	|\Mvar|.}*/

	CBCCoder(const char* src_file_name, const char* tgt_file_name)
	 : base(src_file_name, tgt_file_name, new BlkCipher, true) {}
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
	uint32 calculate_length_of_encoded_data(uint32 input_length) const;
*/
	/*{\Mop calculates the length (in bytes) of the output when encoding some
	input of the given length with the current settings.
	(This function is helpful for encoding memory chunks (see below).)}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	uint32 encode_memory_chunk(const char* in_buf, uint32 in_len, char* out_buf, uint32 out_len);
*/
	/*{\Mop encodes the memory chunk starting at |in_buf| with size |in_len| 
	into the buffer starting at |out_buf| with size |out_len|. 
	The function returns actual length of the encoded chunk which may be 
	smaller than |out_len|. If the output buffer is too small for the encoded
	data the failure flag will be set (see below).
	\textbf{Note:} The output data is slightly longer than the input data due
	to padding and header data.}*/

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
	void reset(bool keep_key = true); 
*/
	/*{\Mop puts |\Mvar| in the same state as the default constructor.
	If |keep_key| is false the key is set to the empty key.
	In any case the initialization vector is set to |nil|.}*/

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

/*{\Moptions nextwarning=no }*/
/*	inherited:
	virtual string get_description() const;
*/
	/*{\Mop provides a description for |\Mvar|.}*/

/*{\Mtext \headerline{Additional Operations}}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	virtual uint16 get_default_key_size() const;
*/
	/*{\Mop returns the default key size.}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	virtual uint16 get_accepted_key_size(uint16& min, uint16& max) const;
*/
	/*{\Mop provides the minimum and maximum key size (in bytes). 
	The return value is the default key size.}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	void   set_key(const CryptKey& key);
*/
	/*{\Mop sets the key.}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	virtual bool has_key() const { return Key != 0; }
*/
	/*{\Mop queries whether a key has been set.}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	CryptKey       get_key() const;
*/
	/*{\Mop returns the current key.}*/

	BlkCipher* get_block_cipher() const { return static_cast<BlkCipher*>(base::get_block_cipher()); }
	/*{\Mop returns the underlying block-cipher.}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	void        set_initialization_vector(const byte* iv) { ExternalIV = iv; }
*/
	/*{\Mop sets a user defined initialization vector (IV). 
	The size of |iv| must be at least the block size of |BlkCipher|.
	(If no IV is provided by the user then |\Mvar| generates its own.)}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	const byte* get_initialization_vector() const { return ExternalIV; }
*/
	/*{\Mop returns the user defined IV. (If no IV has been specified then 
	|nil| is returned.)}*/

protected:
	virtual const char* get_class_name() const { return "CBCCoder"; }

	virtual void set_block_cipher(block_cipher_base* cipher, bool own_cipher) {} // hide
};

/// CFBCoder ////////////////////////////////////////////////////////////////////////////////

template <class BlkCipher = Twofish>
class CFBCoder : public cfb_mode {
public:
	typedef cfb_mode base;
	typedef BlkCipher block_cipher;

public:
	CFBCoder(streambuf* src_stream = 0, streambuf* tgt_stream = 0, bool own_streams = false)
	 : base(src_stream, tgt_stream, own_streams, new BlkCipher, true) {}

	CFBCoder(const char* src_file_name, const char* tgt_file_name)
	 : base(src_file_name, tgt_file_name, new BlkCipher, true) {}

	BlkCipher* get_block_cipher() const { return static_cast<BlkCipher*>(base::get_block_cipher()); }

protected:
	virtual const char* get_class_name() const { return "CFBCoder"; }

	virtual void set_block_cipher(block_cipher_base* cipher, bool own_cipher) {} // hide
};

/// OFBCoder ////////////////////////////////////////////////////////////////////////////////

template <class BlkCipher = Twofish>
class OFBCoder : public ofb_mode {
public:
	typedef ofb_mode base;
	typedef BlkCipher block_cipher;

public:
	OFBCoder(streambuf* src_stream = 0, streambuf* tgt_stream = 0, bool own_streams = false)
	 : base(src_stream, tgt_stream, own_streams, new BlkCipher, true) {}

	OFBCoder(const char* src_file_name, const char* tgt_file_name)
	 : base(src_file_name, tgt_file_name, new BlkCipher, true) {}

	BlkCipher* get_block_cipher() const { return static_cast<BlkCipher*>(base::get_block_cipher()); }

protected:
	virtual const char* get_class_name() const { return "OFBCoder"; }

	virtual void set_block_cipher(block_cipher_base* cipher, bool own_cipher) {} // hide
};

#if LEDA_ROOT_INCL_ID == 500710
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif

LEDA_END_NAMESPACE

#endif
