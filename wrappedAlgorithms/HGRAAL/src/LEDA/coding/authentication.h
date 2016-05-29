/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  authentication.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:44:35 $

#ifndef _LEDA_AUTHENTICATION_H
#define _LEDA_AUTHENTICATION_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500714
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/coding/block_cipher_modes.h>
#include <LEDA/coding/block_ciphers.h>

LEDA_BEGIN_NAMESPACE

/// OMACCoder ////////////////////////////////////////////////////////////////////////////////

/*{\Manpage {OMACCoder} {BlkCipher} {Authentication} {C}}*/

template <class BlkCipher = Rijndael>
class OMACCoder : public omac_mode {
/*{\Mdefinition
The class |\Mtype| can be used for authentication. It implements the One-Key 
CBC MAC algorithm by Iwata and Kurosawa \cite{IK03}.
A MAC (=\,message authentication code) is a kind of checksum that is generated
for a message $m$. In contrast to a usual checksum a MAC does not only depend
on $m$ but also on a secret key $k$. A MAC can be used if a sender wants to 
transfer $m$ through an insecure channel to a receiver. If the sender also 
transmits the MAC then the receiver can verify that $m$ has not been altered 
and that it originates from someone who knows $k$.\\
{\bf Important:} |\Mtype| does not encipher or decipher any data, it only 
computes and verifies MACs!

The behaviour of this coder depends on the |MAC_in_stream| flag. If it is 
false then encoding and decoding are equivalent: Both methods copy the source 
stream to the target stream and compute a MAC.
If the flag is true then in encoding mode the source stream is copied to the 
target stream, a MAC is computed and appended to the target stream. 
In decoding mode the MAC at the end of source stream is removed and the 
original data is copied to the target stream, a MAC is computed and compared 
with the MAC found in the source stream. If the two MACs differ an error is 
signaled. The method |MAC_is_valid| may be used to check whether the source 
stream is authentic.

The class |\Mname| is parameterized with a block-cipher |BlkCipher|, which can
be one of the following: |Rijndael| (default), |Blowfish|, |Twofish| 
(see Section~\ref{block-ciphers} for more information).
|\Mtype| also supports fast seek operations 
(see Section~\ref{decoding_ifstream}).
}*/
public:
	typedef omac_mode base;
	typedef BlkCipher block_cipher;

public:
/*{\Mcreation}*/

	OMACCoder(streambuf* src_stream = 0, streambuf* tgt_stream = 0, bool own_streams = false)
	 : base(src_stream, tgt_stream, own_streams, new BlkCipher, true) {}
	/*{\Mcreate creates an instance |\Mvar| which uses the given source 
	and target streams. If |own_streams| is set, then |\Mvar| is 
	responsible for the destruction of the streams, otherwise the pointers 
	|src_stream| and |tgt_stream| must be valid during the life-time of 
	|\Mvar|.}*/

	OMACCoder(const char* src_file_name, const char* tgt_file_name)
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
	void reset(bool keep_parameters = true); 
*/
	/*{\Mop puts |\Mvar| in the same state as the default constructor.
	If |keep_parameters| is false the parameters are set to their default
	values and the key is set to the empty key.}*/

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
	uint16 get_accepted_key_size(uint16& min, uint16& max) const;
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
	bool has_key() const { return Key != 0; }
*/
	/*{\Mop queries whether a key has been set.}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	CryptKey       get_key() const;
*/
	/*{\Mop returns the current key.}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	const byte* check();
*/
	/*{\Mop checks the source stream and returns the computed MAC. 
	No target stream needs to be set. (If a target stream is set 
	[[check]] coincides with [[decode]]).}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	bool MAC_is_valid() const;
*/
	/*{\Mop returns whether |\Mvar| has a valid MAC.}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	string      get_MAC_as_hex_string() const;
*/
	/*{\Mop returns the MAC for the stream that has been processed
	last as hexadecimal string.}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	const byte* compute_MAC(string str);
*/
	/*{\Mop computes a MAC for a string. (Sets source and target stream to
	|nil|.)}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	void set_MAC_in_stream_flag(bool mac_in_stream);
*/
	/*{\Mop sets the |MAC_in_stream| flag.}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	bool get_MAC_in_stream_flag() const { return MACInStream; }
*/
	/*{\Mop returns the |MAC_in_stream| flag.}*/ 

	BlkCipher* get_block_cipher() const { return static_cast<BlkCipher*>(base::get_block_cipher()); }

protected:
	virtual const char* get_class_name() const { return "OMACCoder"; }

	virtual void set_block_cipher(block_cipher_base* cipher, bool own_cipher) {} // hide
};

#if LEDA_ROOT_INCL_ID == 500714
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif

LEDA_END_NAMESPACE

#endif
