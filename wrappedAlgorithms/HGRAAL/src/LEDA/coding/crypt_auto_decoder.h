/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  crypt_auto_decoder.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:44:37 $

#ifndef _LEDA_CRYPT_AUTO_DECODER_H
#define _LEDA_CRYPT_AUTO_DECODER_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500709
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/coding/auto_decoder.h>
#include <LEDA/coding/memory_streambuf.h>
#include <LEDA/coding/crypt_key.h>
#include <LEDA/core/list.h>

LEDA_BEGIN_NAMESPACE

/// CryptAutoDecoder ////////////////////////////////////////////////////////////////////////

/*{\Manpage {CryptAutoDecoder} {} {Automatic Decoder supporting Cryptography} {C}}*/

class __exportC CryptAutoDecoder : public AutoDecoder {
/*{\Mdefinition
The class |\Mtype| is an extension of |AutoDecoder|. It can be used for
decoding encrypted streams (in particular if the coder used for encoding is 
unknown). In order to decrypt a stream a key must be provided by the user 
because the key is -- of course -- not stored in the encrypted stream. 
There are three possibilities to specify a key: as a |CryptKey|, as a key 
stream or as a key file. 
Sometimes it may be necessary to provide several keys to |\Mtype|. 
E.g., if you use |CoderPipe2< OMACCoder<>, OFBCoder<> >| to 
encode a stream two keys are needed for decoding. 
Therefore, |\Mtype| maintains a list of keys for decoding.
}*/
public:
	typedef AutoDecoder base;
	enum { ID = 0x434c16 };

public:
/*{\Mcreation}*/

	CryptAutoDecoder(streambuf* src_stream = 0, streambuf* tgt_stream = 0, bool own_streams = false);
	/*{\Mcreate creates an instance |\Mvar| which uses the given source 
	and target streams. If |own_streams| is set, then |\Mvar| is 
	responsible for the destruction of the streams, otherwise the pointers 
	|src_stream| and |tgt_stream| must be valid during the life-time of 
	|\Mvar|.}*/

	CryptAutoDecoder(const char* src_file_name, const char* tgt_file_name);
	/*{\Mcreate creates an instance |\Mvar| which uses file-streams for 
	input and output.}*/

/*{\Moperations 3.4 3.5}*/
/*{\Mtext \headerline{Standard Operations}}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	void decode();
*/
	/*{\Mop decodes the source stream and writes the output to the target
	stream.}*/
/*{\Moptions nextwarning=no }*/
/*	inherited:
	uint32 decode_memory_chunk(const char* in_buf, uint32 in_len, char* out_buf, uint32 out_len);
*/
	/*{\Mop decodes the memory chunk starting at |in_buf| with size |in_len| 
	into the buffer starting at |out_buf| with size |out_len|. 
	The function returns actual length of the encoded chunk which may be 
	smaller than |out_len|. If the output buffer is too small for the decoded
	data the failure flag will be set (see below).}*/

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
	void reset(); 
*/
	/*{\Mop puts |\Mvar| in exactly the same state as the default 
	constructor.}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	bool failed() const;
*/
	/*{\Mop returns |true| if an error occured.}*/
/*{\Moptions nextwarning=no }*/
/*	inherited:
	bool finished() const;
*/
	/*{\Mop returns |true| if decoding is finished.}*/
/*{\Moptions nextwarning=no }*/
/*	inherited:
	virtual string get_description() const;
*/
	/*{\Mop provides a description for |\Mvar|. After decoding this 
	includes a description of the coder that has been used for encoding the
	stream.}*/

/*{\Mtext \headerline{Additional Operations}}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	coder_base* get_coder() const { return pActualCoder; }
*/
	/*{\Mop after decoding this function returns the coder that has actually
	been used to decode the stream.}*/

	bool has_key() const { return ! Keys.empty(); }
	/*{\Mop returns whether the key list is not empty.}*/

	list<CryptKey> get_keys() const { return Keys; }
	/*{\Mop returns the current list of keys.}*/

	void set_key(const CryptKey& key)
	{ clear_keys(); add_key(key); }
	/*{\Mop sets the key that should be used for decoding.}*/

	void add_key(const CryptKey& key);
	/*{\Mop adds a key to the key list.}*/

	void set_keys_in_stream(streambuf* key_stream) 
	{ clear_keys(); add_keys_in_stream(key_stream); }
	/*{\Mop makes the key list equal to the key(s) in |key_stream|.}*/

	void add_keys_in_stream(streambuf* key_stream);
	/*{\Mop adds the key(s) in |key_stream| to the key list.}*/

	void set_keys_in_file(const char* file_name)
	{ clear_keys(); add_keys_in_file(file_name); }
	/*{\Mop makes the key list equal to the key(s) in the given file.}*/

	void add_keys_in_file(const char* file_name);
	/*{\Mop adds the key(s) in the given file to the key list.}*/

	void clear_keys() { Keys.clear(); }
	/*{\Mop removes all keys.}*/

protected:
	virtual const char* get_class_name() const { return "CryptAutoDecoder"; }
	virtual id_type get_id() const { return ID; }

	virtual void init_encoding();
	virtual void init_decoding();

	virtual void do_reset(reset_type t);

private:
	void convert_keys_to_memory_stream();

private:
	list<CryptKey>    Keys;
	secure_byte_array KeyMemory; // will wipe itself out after use
};

/*{\Mexample
Example on how to use |\Mtype| with a key file:
\begin{verbatim}
  typedef OFBCoder<> Cipher;

  // generate key ...
  CryptByteString passphrase("secret phrase");
  CryptByteString dummy_salt; // will be ignored
  CryptKey key 
    = CryptKey::generate_key_and_salt(16, 16, passphrase, dummy_salt, true);
    // 8*16 = 128 bit salt and key

  // write key
  ofstream key_out("secret.key");
  key_out << key;
  key_out.close();

  // encipher data
  encoding_ofstream<Cipher> data_out("data");
  data_out.get_coder()->set_key(key);
  data_out << "my secret text ..." << endl;
  data_out.close();

  // much later ...

  // decipher data
  decoding_ifstream<CryptAutoDecoder> data_in("data");
  data_in.get_coder()->set_keys_in_file("secret.key");
  string data; data.read_line(data_in); cout << data << endl;
  data_in.close();
\end{verbatim}
}*/

#if LEDA_ROOT_INCL_ID == 500709
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif

LEDA_END_NAMESPACE

#endif
