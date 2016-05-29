/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  socket_streambuf.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.3 $  $Date: 2005/04/14 10:44:39 $

#ifndef _LEDA_SOCKET_STREAMBUF_H
#define _LEDA_SOCKET_STREAMBUF_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 450716
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/coding/coder_base.h>
#include <LEDA/system/socket.h>
#include <LEDA/coding/internal/streambuf_workaround.h>

LEDA_BEGIN_NAMESPACE

/// socket_streambuf ////////////////////////////////////////////////////////////////////////

/*{\Manpage {socket_streambuf} {} {Socket Streambuffer} {sb}}*/

class __exportC socket_streambuf : public LEDA_STREAMBUF {
/*{\Mdefinition
An instance |\Mvar| of class |\Mname| can be used as an adapter: It turns a 
|leda_socket| $s$ into a \CC{}-streambuf object. This object can be used in 
standard \CC{} |ostreams| and |istreams| to make the communication through 
the socket easier. 
A |\Mname| can also be applied within an |encoding_ostream| or a 
|decoding_istream| provided by LEDA to send compressed or encrypted data 
over a socket connection (see Sections \ref{encoding_ostream} and 
\ref{decoding_istream}).\\
Observe that |\Mname| itself does not apply cryptography to secure the 
transmitted data. All data is sent as it is. If you want to secure your data, 
consider using the class |secure_socket_streambuf| 
(see Section~\ref{secure_socket_streambuf}).

If two parties want to use the class |\Mname| to exchange data they have to 
do the following. First they establish a connection through |leda_sockets|. 
Then each party contructs an instance of the class |\Mname| which is attached 
to its socket. This is shown in an example at the end of this manual page.

Every instance |\Mvar| has an out-buffer where outgoing data is buffered 
before it is sent through the socket over the internet. In addition it has an 
in-buffer to store data that has been received through the socket.
The user may specify the maximum size of each buffer. The actual size of each 
buffer is determined in a negotiation between the server and the client at the 
beginning of the communication. The size of outgoing packets from the server 
is set to the minimum of the out-buffer size of the server and the in-buffer 
size of the client. The size of the incoming packets is determined in an 
analogous way.
}*/
public:
	typedef LEDA_STREAMBUF base;
	typedef coder_types::byte byte;
	typedef coder_types::uint32 uint32;
	enum { DefaultBufferSize = 1024 };

public:
/*{\Mcreation}*/

	socket_streambuf(leda_socket& s, 
		             uint32 out_buf_sz = DefaultBufferSize, 
					 uint32 in_buf_sz  = DefaultBufferSize,
					 bool send_acknowledge = false);
	/*{\Mcreate creates a |\Mvar| and attaches it to the socket |s|.
	The parameters |out_buf_sz| and |in_buf_sz| determine the maximum size of 
	the out-buffer and the in-buffer. |send_acknowledge| specifies whether an 
	acknowledgement is sent for every received packet.
	\\ \precond The connection between the server and the client must have 
	been established when |\Mvar| is created.}*/

	virtual ~socket_streambuf();

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
	void error(const char* msg);

	static void   store_uint32(uint32 val, byte* mem);
	static uint32 load_uint32(const byte* mem);

	void negotiate_packet_sizes(uint32 out_buf_sz, uint32 in_buf_sz);

	void allocate_buffers();
	void free_buffers();

private:
	enum Header { h_Ok = 0x00, h_Ack = 0x01, h_EOF = 0x02 };
	enum { HeaderSize = 1 };

	void send_put_buffer(Header header = h_Ok);
	int  fill_get_buffer();

private:
	leda_socket& Socket;
	char*        PutBuffer;
	char*        GetBuffer;
	uint32       OutgoingPacketSize;
	uint32       IncomingPacketSize;
	string       ErrorMsg;
	bool         Failed;
	bool         HasPutEOF, HasGotEOF;
	bool         HasHadPuts;
	bool         WaitForAcknowlegdeOfOutgoingPackets;
	bool         SendAcknowlegdeForIncomingPackets;
};

#undef LEDA_STREAMBUF

/*{\Mexample
The following example shows how the usage of |\Mvar| from the server and from 
the client side. In our example the server sends a string, which is received 
by the client. (Note that it would also be possible that the client sends and 
the server receives data.)\\
In order to add compression to the example simply replace ``[[ostream]]'' by 
``[[encoding_ostream<Coder>]]'' and ``[[istream]]'' by ``[[decoding_istream]]''
as indicated in the comments.

\begin{verbatim}
void socket_example_server(int port)
{
  leda_socket sock;
  sock.set_port(port);

  // open port
  if (! sock.listen()) {
    cerr << sock.get_error() << endl; return;
  }
  for (;;) {
    // establish connection
    if (! sock.accept()) {
      cerr << sock.get_error() << endl; continue;
    }

    // send data
    {
      socket_streambuf sb(sock);
      ostream out(&sb);
      // or: encoding_ostream<PPMIICoder> out(&sb);
      out << "Hello world!" << endl;
    } // destroys sb and calls sb.sputEOF() automatically
  }
}

void socket_example_client(int port, string host)
{
  leda_socket sock;
  sock.set_host(host);
  sock.set_port(port);

  // establish connection
  if (! sock.connect()) {
    cerr << sock.get_error() << endl; return;
  }

  // receive data
  socket_streambuf sb(sock);
  istream in(&sb);
  // or: decoding_istream<PPMIICoder> in(&sb);
  string str;
  str.read_line(in);
  cout << "received: " << str << endl;
}
\end{verbatim}
}*/

#if LEDA_ROOT_INCL_ID == 450716
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif

LEDA_END_NAMESPACE

#endif
