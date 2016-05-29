/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  secure_secure_socket_streambuf.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.3 $  $Date: 2005/04/14 10:44:38 $

#ifndef _LEDA_SECURE_SOCKET_STREAMBUF_H
#define _LEDA_SECURE_SOCKET_STREAMBUF_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 450717
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/system/socket.h>
#include <LEDA/coding/coder_util.h>
#include <LEDA/coding/deflate.h>
#include <LEDA/coding/memory_streambuf.h>
#include <LEDA/coding/authentication.h>
#include <LEDA/coding/stream_ciphers.h>
#include <LEDA/coding/internal/streambuf_workaround.h>

LEDA_BEGIN_NAMESPACE

/// secure_socket_streambuf /////////////////////////////////////////////////////////////////

/*{\Manpage {secure_socket_streambuf} {} {Secure Socket Streambuffer} {sb}}*/

class __exportC secure_socket_streambuf : public LEDA_STREAMBUF {
/*{\Mdefinition
An instance |\Mvar| of class |\Mname| can be used as an adapter: It turns a 
|leda_socket| $s$ into a \CC{}-streambuf object. This object can be used in 
standard \CC{} |ostreams| and |istreams|, which makes the communication through 
the socket easier. 
Moreover, |\Mname| uses cryptography to secure the communication.
Every piece of data is authenticated, (possibly) compressed and encrypted before
it is sent.

If two parties want to use the class |\Mname| to exchange data they have to 
do the following. First they establish a connection through |leda_sockets|. 
Then each party contructs an instance of the class |\Mname| which is attached 
to its socket. (An example showing how to this can be found at the end of 
Section~\ref{socket_streambuf}; simply replace ``|socket_streambuf|'' by 
``|\Mname|'' and add the passphrase(s).)

The communication between two instances of the class |\Mname| can be divided 
in two phases. In the first phase the two parties negotiate session parameters
like packet sizes and they also agree on a so-called session-seed which will 
be explained later. In the second phase the actual user data is exchanged.
Each phases is protected with cryptography. The authentication and encryption 
keys for the first phase are generated in a deterministic way from the 
user-supplied passphrase(s). They are called master keys because they remain 
the same as long as the passphrases are not changed.
In order to protect the master keys we use them only during the first phase, 
which consists of two messages from each party. After that we use the 
random session seed (in addition to the passphrases) to compute session keys
which are used during the second phase.
}*/
public:
	typedef LEDA_STREAMBUF base;
	enum { DefaultBufferSize = 1024 };

	enum { ProtocolVersion = 0x03 };
	typedef CoderPipe2< OMACCoder<Rijndael>, CBCCoder<Twofish> > crypt_coder;
	typedef CoderPipe3< OMACCoder<Rijndael>, DeflateCoder, CBCCoder<Twofish> > crypt_and_compress_coder;
		// if one of the coders above changes, also change ProtocolVersion!

	typedef coder_strbuf_adapter crypt_streambuf;
	typedef coder_types::byte byte;
	typedef coder_types::uint32 uint32;

public:
/*{\Mcreation}*/

	secure_socket_streambuf(leda_socket& s, 
	                        const CryptByteString& auth_passphrase, 
	                        const CryptByteString& cipher_passphrase, 
	                        uint32 out_buf_sz = DefaultBufferSize, 
							uint32 in_buf_sz  = DefaultBufferSize, 
							bool enable_compression = false,
							bool send_acknowledge = false);
	/*{\Mcreate creates a |\Mvar| and attaches it to the socket |s|. 
	The two passphrases are used to generate the keys for authentication and 
	encryption.
	The parameters |out_buf_sz| and |in_buf_sz| determine the maximum size of 
	the out-buffer and the in-buffer. 
	The paramter |enable_compression| determines whether compression is used.
	|send_acknowledge| specifies whether an acknowledgement is sent for every 
	received packet.
	\\ \precond The connection between the server and the client must have 
	been established when |\Mvar| is created.}*/

	secure_socket_streambuf(leda_socket& s, 
	                        const CryptByteString& passphrase, 
	                        uint32 out_buf_sz = DefaultBufferSize, 
							uint32 in_buf_sz  = DefaultBufferSize, 
							bool enable_compression = false,
							bool send_acknowledge = false);
	/*{\Mcreate as above, but the same |passphrase| is used for generating both
	the authentication and the encryption key. However, it is still guaranteed 
	that two different keys are used.}*/

	virtual ~secure_socket_streambuf();

/*{\Moperations}*/
/*{\Mtext
The class |\Mname| inherits most of its operations from the class |streambuf| 
that belongs to the \CC{} standard library. Usually there is no need to call 
these operations explicitly. 
(You can find documentation for |streambuf| at 
\htmladdnormallink{\texttt{http://www.cplusplus.com}}.)
}*/

	bool failed() const { return Failed; }
	/*{\Mop returns whether a failure has occured.}*/

	string get_error() const { return ErrorMsg; }
	/*{\Mop returns an error message (if available).}*/

	void sputEOF();
	/*{\Mop signals the end of the transmission to the receiving socket, 
	so that it does not wait for further data.
	(This function is called automatically in the destructor unless it has been
	called explicitly by the user. If |\Mvar| is not immediately destroyed 
	after the end of the transmission then you should call |sputEOF| 
	explicitly, otherwise the receiving party might incur a timeout error.)}*/

	bool has_put_EOF() const { return HasPutEOF; }
	/*{\Mop returns whether EOF has already been sent.}*/

	bool has_got_EOF() const { return HasGotEOF; }
	/*{\Mop returns whether EOF has already been received.}*/

	leda_socket& get_socket() { return Socket; }
	/*{\Mop returns the socket to which |\Mvar| is attached.}*/

	uint32 get_outgoing_packet_size() const { return OutgoingPacketSize; }
	/*{\Mop returns the (actual) outgoing packet size.}*/

	uint32 get_incoming_packet_size() const { return IncomingPacketSize; }
	/*{\Mop returns the (actual) incoming packet size.}*/

	bool uses_compression() const { return CompressionEnabled; }
	/*{\Mop returns whether |\Mvar| compresses outgoing packets before 
	sending.}*/

	bool waits_for_acknowledge() const { return WaitForAcknowlegdeOfOutgoingPackets; }
	/*{\Mop returns whether |\Mvar| expects an acknowledgement for outgoing 
	packets.}*/

	bool sends_acknowledge() const { return SendAcknowlegdeForIncomingPackets; }
	/*{\Mop returns whether |\Mvar| sends an acknowledgement for incoming 
	packets.}*/

protected:
	// the following functions are inherited from streambuf
	virtual int overflow(int c);
		// called when put-buffer buffer full  (puts the character to the stream)
	virtual int underflow();
		// called when get-buffer buffer empty (gets a character from stream, but doesn' point past it)
	virtual int uflow() { int c = underflow(); return c < 0 ? c : sbumpc(); }
		// called when get-buffer buffer empty (gets a character from stream, and points past it)

#ifdef LEDA_COMPRESSION_WA1
public:
#endif
	virtual int sync();

private:
	void init(const CryptByteString& auth_phrase, const CryptByteString& cipher_phrase, bool enable_compression);
	void allocate_buffers();
	void free_buffers();

	void error(const char* msg);

	static void   store_uint32(uint32 val, byte* mem);
	static void   store_uint32(uint32 val, char* mem) { store_uint32(val, (byte*) mem); }
	static uint32 load_uint32(const byte* mem);
	static uint32 load_uint32(const char* mem) { return load_uint32((const byte*) mem); }

	void negotiate(const CryptByteString& auth_phrase, const CryptByteString& cipher_phrase, bool enable_compression);
	void negotiate_as_client(CryptByteString& session_seed, bool& enable_compression);
	void negotiate_as_server(CryptByteString& session_seed, bool& enable_compression);

private:
	enum { HeaderSize = 1+4, SeedSize = 32, NonceSize = 8, AckNonceSize = 16, 
	       MaxMsgSize = 4*4 + NonceSize + SeedSize, Overshoot = 100 };
	enum Header { h_Ok = 0x00, h_Ack = 0x01, h_EOF = 0x02, h_OkOrEOF = 0x03 };
	enum Flags { f_Ack = 0x01, f_Compress = 0x02 };

	void reset_crypt_streambuf();
	void encrypt_and_send(char* buf, int n, Header h = h_Ok);
	int  receive_and_decrypt(char* buf, int n, Header expected_header = h_Ok);

	void send_put_buffer(Header header = h_Ok);
	int  fill_get_buffer();

private:
	leda_socket&     Socket;
	crypt_streambuf  CryptStreambuf;
	CryptKey         AuthKey, CipherKey;
	char*            PutBuffer;
	char*            GetBuffer;
	uint32           OutgoingPacketSize;
	uint32           IncomingPacketSize;
	uint32           OutgoingMessageCnt;
	uint32           IncomingMessageCnt;
	memory_streambuf InternalBuffer;
	string           ErrorMsg;
	bool             Failed;
	bool             HasPutEOF;
	bool             HasGotEOF;
	bool             CompressionEnabled;
	bool             WaitForAcknowlegdeOfOutgoingPackets;
	bool             SendAcknowlegdeForIncomingPackets;
};

#undef LEDA_STREAMBUF

#if LEDA_ROOT_INCL_ID == 450717
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif

LEDA_END_NAMESPACE

#endif
