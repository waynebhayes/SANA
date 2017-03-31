/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  checksum.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:44:36 $

#ifndef _LEDA_CHECKSUM_H
#define _LEDA_CHECKSUM_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500022
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/coding/coder_base.h>

#include <LEDA/core/b_queue.h>

LEDA_BEGIN_NAMESPACE

/// checksummer_base ////////////////////////////////////////////////////////////////////////

/*{\Manpage {checksummer_base} {} {Checksummers} {C}}*/

class __exportC checksummer_base : public coder_base {
/*{\Mdefinition
The class |\Mname| is the base class for all LEDA checksummers. It cannot
be instantiated. In order to avoid repeating this documentation for each 
derived class below we will discuss its members here.

When a checksummer is used in encoding mode it can add checksums 
to the output stream. This is controlled by the |checksums_in_stream| 
flag. If this flag is set and the block size is zero (default setting)
then one checksum is appended at the end of the stream. If the flag is 
switched on and the block size $b$ is positive then a checksum is 
written for every block of $b$ characters.\\
When the checksummer is used in decoding mode this flag specifies 
whether the source stream contains checksums. If so, they will be compared
against the computed checksum for the stream (or for the respective block
if the block size is positive).

If you use a checksummer in a coder pipe (cf.~Section~\ref{Coder Pipes})
then it should be the first coder in the pipe. This ensures that the 
checksum is computed for the original input. \\
Finally, we want to point out that all checksummers provide fast seek 
operations (cf.~Section~\ref{Decoding File Stream}).
}*/
public:
	typedef coder_base base;
	enum { MaxByteLengthOfCheckSum = 20 };
public:
/*{\Mcreation}*/

	checksummer_base(streambuf* src_stream = 0, streambuf* tgt_stream = 0, bool own_streams = false);
	/*{\Mcreate creates an instance |\Mvar| which uses the given source 
	and target streams. If |own_streams| is set, then |\Mvar| is 
	responsible for the destruction of the streams, otherwise the pointers 
	|src_stream| and |tgt_stream| must be valid during the life-time of 
	|\Mvar|.}*/

	checksummer_base(const char* src_file_name, const char* tgt_file_name = 0);
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
	uint32 encode_memory_chunk(const char* in_buf, uint32 in_len, char* out_buf, uint32 out_len);
*/
	/*{\Mop encodes the memory chunk starting at |in_buf| with size |in_len| 
	into the buffer starting at |out_buf| with size |out_len|. 
	The function returns actual length of the encoded chunk which may be 
	smaller than |out_len|. If the output buffer is too small for the encoded
	data the failure flag will be set (see below).}*/

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
	values.}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	bool failed() const;
*/
	/*{\Mop returns |true| if an error occured or a checksum in the 
	stream does not match the computed checksum.}*/

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

/*{\Moptions nextwarning=no }*/
	virtual bool is_optimizing_seeks() const { return true; }

/*{\Mtext \headerline{Additional Operations}}*/

	string check();
	/*{\Mop checks the source stream and returns the computed checksum. 
	No target stream needs to be set. (If a target stream is set, 
	[[check]] coincides with [[decode]]).}*/

	bool checksum_is_valid() const { return (CheckSumState == cs_okay) && !failed(); }
	/*{\Mop returns whether |\Mvar| has a valid checksum.}*/

	string get_checksum_as_hex_string() const;
	/*{\Mop returns the checksum for the stream that has been processed
	last as hexadecimal string.}*/

	string check(string str);
	/*{\Mop computes a checksum for a string. (Sets source and target stream to
	|nil|.)}*/

	virtual unsigned get_byte_length_of_checksum() const = 0;
	virtual byte     get_ith_byte_of_checksum(unsigned i) const = 0;

	void set_checksums_in_stream_flag(bool sums_in_stream, uint32 blocksize = 0);
	/*{\Mop sets the |checksums_in_stream| flag and the block size.}*/

	bool get_checksums_in_stream_flag() const { return CheckSumsInStream; }
	/*{\Mop returns the |checksums_in_stream| flag.}*/ 

	uint32 get_block_size() const { return BlockSize; }
	/*{\Mop returns the current block size.}*/ 

protected:
	// derived classes should overwrite the following methods:
//	virtual id_type get_id() const;

	virtual void reset_checksum() = 0;
	virtual void update_checksum(byte b) = 0;
	virtual void finish_checksum() {}

protected:
	virtual void write_header_to_tgt_stream();
	virtual void read_header_from_src_stream();

	virtual void init_encoding();
	virtual void encode_stream();

	virtual void init_decoding();
	virtual void decode_stream();

	virtual void do_reset(reset_type t);

	virtual streampos decode_seek(streampos current_pos, streampos new_pos);

private:
	void write_checksum();
	bool fill_read_ahead_buffer();
	void compare_checksum_with_read_ahead();

private:
	enum cs_state { cs_invalid, cs_okay, cs_computing };

private:
	uint32        ByteCount;
	uint32        BlockSize;
	b_queue<byte> ReadAheadBuffer;
	bool          CheckSumsInStream;
	cs_state      CheckSumState;
};


/// checksummer32_base //////////////////////////////////////////////////////////////////////

class __exportC checksummer32_base : public checksummer_base {
public:
	typedef checksummer_base base;
	typedef uint32           checksum;
	enum { ByteLengthChkSum = 4 };

public:
	checksummer32_base(streambuf* src_stream = 0, streambuf* tgt_stream = 0, bool own_streams = false)
		: base(src_stream, tgt_stream, own_streams) {}
	checksummer32_base(const char* src_file_name, const char* tgt_file_name = 0)
		: base(src_file_name, tgt_file_name) {}

	checksum get_checksum() const { return checksum_is_valid() ? CheckSum : 0; }

	virtual unsigned get_byte_length_of_checksum() const { return ByteLengthChkSum; }
	virtual byte get_ith_byte_of_checksum(unsigned i) const { return byte(CheckSum >> i * 8); }
	
protected:
	checksum CheckSum;
};


/// checksummer16_base //////////////////////////////////////////////////////////////////////

class __exportC checksummer16_base : public checksummer_base {
public:
	typedef checksummer_base base;
	typedef uint16           checksum;
	enum { ByteLengthChkSum = 2 };

public:
	checksummer16_base(streambuf* src_stream = 0, streambuf* tgt_stream = 0, bool own_streams = false)
		: base(src_stream, tgt_stream, own_streams) {}
	checksummer16_base(const char* src_file_name, const char* tgt_file_name = 0)
		: base(src_file_name, tgt_file_name) {}

	checksum get_checksum() const { return checksum_is_valid() ? CheckSum : 0; }

	virtual unsigned get_byte_length_of_checksum() const { return ByteLengthChkSum; }
	virtual byte     get_ith_byte_of_checksum(unsigned i) const { return byte(CheckSum >> i * 8); }
	
protected:
	checksum CheckSum;
};


/// CRC32Coder //////////////////////////////////////////////////////////////////////////////

/*{\Manpage {CRC32Coder} {} {CRC32 Checksum} {C}}*/

class __exportC CRC32Coder : public checksummer32_base {
/*{\Mdefinition
The class |\Mname| computes CRC32 checksums
(see ISO 3309 and ITU-T V.42 for a formal specification).}*/
public:
	typedef checksummer32_base base;
	enum { ID = 0x434c80 };

public:
	CRC32Coder(streambuf* src_stream = 0, streambuf* tgt_stream = 0, bool own_streams = false);
	CRC32Coder(const char* src_file_name, const char* tgt_file_name = 0);

protected:
	virtual const char* get_class_name() const { return "CRC32Coder"; }
	virtual id_type get_id() const { return ID; }
	virtual void reset_checksum();
	virtual void update_checksum(byte b);
};


/// CCITTCoder //////////////////////////////////////////////////////////////////////////////

/*{\Manpage {CCITTCoder} {} {CRC-CCITT Checksum} {C}}*/

class __exportC CCITTCoder : public checksummer16_base {
/*{\Mdefinition
The class |\Mname| computes CRC-CCITT checksums
(see ISO 3309).}*/
public:
	typedef checksummer16_base base;
	enum { ID = 0x434c81 };

public:
	CCITTCoder(streambuf* src_stream = 0, streambuf* tgt_stream = 0, bool own_streams = false);
	CCITTCoder(const char* src_file_name, const char* tgt_file_name = 0);

protected:
	virtual const char* get_class_name() const { return "CCITTCoder"; }
	virtual id_type get_id() const { return ID; }
	virtual void reset_checksum();
	virtual void update_checksum(byte b);
};


/// Adler32Coder ////////////////////////////////////////////////////////////////////////////

/*{\Manpage {Adler32Coder} {} {Adler32 Checksum} {C}}*/

class __exportC Adler32Coder : public checksummer32_base {
/*{\Mdefinition
The class |\Mname| computes Adler32 checksums 
(see RFC 1950, ZLIB specification version 3.3).}*/
public:
	typedef checksummer32_base base;
	enum { ID = 0x434c82 };

public:
	Adler32Coder(streambuf* src_stream = 0, streambuf* tgt_stream = 0, bool own_streams = false);
	Adler32Coder(const char* src_file_name, const char* tgt_file_name = 0);

protected:
	virtual const char* get_class_name() const { return "Adler32Coder"; }
	virtual id_type get_id() const { return ID; }
	virtual void reset_checksum();
	virtual void update_checksum(byte b);
	virtual void finish_checksum();

private:
	uint32 S1, S2, Cnt;
};

/// MD5SumCoder /////////////////////////////////////////////////////////////////////////////

/*{\Manpage {MD5SumCoder} {} {MD5 Checksum} {C}}*/

class __exportC MD5SumCoder : public checksummer_base {
/*{\Mdefinition
The class |\Mname| computes MD5 checksums 
(see RFC 1321, The MD5 Message-Digest Algorithm).}*/
public:
	typedef checksummer_base base;
	typedef const byte* checksum;
	enum { ID = 0x434c83, ByteLengthChkSum = 16 };

public:
	MD5SumCoder(streambuf* src_stream = 0, streambuf* tgt_stream = 0, bool own_streams = false);
	MD5SumCoder(const char* src_file_name, const char* tgt_file_name = 0);
	~MD5SumCoder();

	checksum get_checksum() const { return checksum_is_valid() ? Digest : 0; }

	virtual unsigned get_byte_length_of_checksum() const { return ByteLengthChkSum; }
	virtual byte     get_ith_byte_of_checksum(unsigned i) const { return Digest[i]; }
	
	const byte* get_digest() const { return Digest; }

protected:
	virtual const char* get_class_name() const { return "MD5SumCoder"; }
	virtual id_type get_id() const { return ID; }
	virtual void reset_checksum();
	virtual void update_checksum(byte b);
	virtual void finish_checksum();

private:
	void md5_transform();

	uint32 getWord(int i) const
	{ // needed if sizeof(uint32) > 4
		const byte* bytes = Chunk + 4*i;
		return  uint32(bytes[0])        | (uint32(bytes[1]) << 8) 
			 | (uint32(bytes[2]) << 16) | (uint32(bytes[3]) << 24);
	}

	void setWord(int i, uint32 val)
	{ // needed if sizeof(uint32) > 4
		byte* bytes = Chunk + 4*i;
		bytes[0] = byte(val      );
		bytes[1] = byte(val >>  8);
		bytes[2] = byte(val >> 16);
		bytes[3] = byte(val >> 24);
	}

	void setDigest(int i, uint32 val)
	{ // needed if sizeof(uint32) > 4
		byte* bytes = Digest + 4*i;
		bytes[0] = byte(val      );
		bytes[1] = byte(val >>  8);
		bytes[2] = byte(val >> 16);
		bytes[3] = byte(val >> 24);
	}

private:
	uint32 ByteCount;
	union {
		byte   Chunk[64];
		uint32 Words[16];
	};
	union {
		uint32 Buf[4];
		byte   Digest[16];
	};
};

/// SHACoder ////////////////////////////////////////////////////////////////////////////////

/*{\Manpage {SHACoder} {} {SHA-1 Checksum} {C}}*/

class __exportC SHACoder : public checksummer_base {
/*{\Mdefinition
The class |\Mname| computes SHA-1 checksums 
(see FIPS PUB 180-1, Secure Hash Standard).}*/
public:
	typedef checksummer_base base;
	typedef const byte* checksum;
	enum { ID = 0x434c84, ByteLengthChkSum = 20 };

public:
	SHACoder(streambuf* src_stream = 0, streambuf* tgt_stream = 0, bool own_streams = false);
	SHACoder(const char* src_file_name, const char* tgt_file_name = 0);
	~SHACoder();

	checksum get_checksum() const { return checksum_is_valid() ? Digest : 0; }

	virtual unsigned get_byte_length_of_checksum() const { return ByteLengthChkSum; }
	virtual byte     get_ith_byte_of_checksum(unsigned i) const { return Digest[i]; }
	
	const byte* get_digest() const { return Digest; }

protected:
	virtual const char* get_class_name() const { return "SHACoder"; }
	virtual id_type get_id() const { return ID; }
	virtual void reset_checksum();
	virtual void update_checksum(byte b);
	virtual void finish_checksum();

private:
	void sha_transform();

	uint32 getWord(int i) const
	{ // needed if sizeof(uint32) > 4
		const byte* bytes = Chunk + 4*i;
		return  uint32(bytes[3])        | (uint32(bytes[2]) << 8) 
			 | (uint32(bytes[1]) << 16) | (uint32(bytes[0]) << 24);
	}

	void setWord(int i, uint32 val)
	{ // needed if sizeof(uint32) > 4
		byte* bytes = Chunk + 4*i;
		bytes[3] = byte(val      );
		bytes[2] = byte(val >>  8);
		bytes[1] = byte(val >> 16);
		bytes[0] = byte(val >> 24);
	}

	void setDigest(int i, uint32 val)
	{ // needed if sizeof(uint32) > 4
		byte* bytes = Digest + 4*i;
		bytes[3] = byte(val      );
		bytes[2] = byte(val >>  8);
		bytes[1] = byte(val >> 16);
		bytes[0] = byte(val >> 24);
	}

private:
	uint32 ByteCount;
	union {
		byte   Chunk[64];
		uint32 Words[80];
	};
	union {
		uint32 Buf[5];
		byte   Digest[20];
	};
};

#if LEDA_ROOT_INCL_ID == 500022
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif

LEDA_END_NAMESPACE

#endif
