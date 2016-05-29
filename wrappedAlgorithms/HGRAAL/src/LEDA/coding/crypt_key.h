/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  crypt_key.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.5 $  $Date: 2005/04/14 10:44:38 $

#ifndef _LEDA_CRYPT_KEY_H
#define _LEDA_CRYPT_KEY_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500712
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/coding/coder_base.h>
#include <LEDA/internal/std/string.h> // for memcpy, memset

LEDA_BEGIN_NAMESPACE

/// secure_byte_array ///////////////////////////////////////////////////////////////////////

class __exportC secure_byte_array {
public:
	typedef coder_types::byte   byte;
	typedef coder_types::uint32 uint32;

	enum { MaxSizeLockAgainstSwap = 4 * 1024 };
		// If the size is at most MaxSizeLockAgainstSwap, we try to prevent
		// the array from being swapped out by the OS!
		// (Only supported on Win NT/2000/XP so far!)

public:
	secure_byte_array() : Size(0), Bytes(0) {}
	secure_byte_array(const byte* bytes, uint32 sz);
	explicit secure_byte_array(uint32 sz);
	~secure_byte_array() { clear(); }

	uint32 get_size() const { return Size; }
	void   set_size(uint32 sz);
	byte*  get_bytes() { return Bytes; }

	void clear() { set_size(0); }
	void wipe() { if (Size) wipe_memory(Bytes, Size); }

	byte  operator[](uint32 idx) const { return Bytes[idx]; }
	byte& operator[](uint32 idx) { return Bytes[idx]; }

	operator byte*() { return Bytes; }
	operator const byte*() const { return Bytes; }
	operator char*() { return (char*) Bytes; }
	operator const char*() const { return (const char*) Bytes; }
	operator void*() { return Bytes; }

	bool operator==(const secure_byte_array& a2) const
	{ return (Size == a2.Size) && (memcmp(Bytes, a2.Bytes, (size_t) Size) == 0); }
	bool operator!=(const secure_byte_array& a2) const
	{ return !operator==(a2); }

	static void wipe_memory(void* mem, uint32 sz)
	{ memset(mem, 0xFF, sz); memset(mem, 0x55, sz); memset(mem, 0x00, sz); }

private:
	uint32 Size;
	byte*  Bytes;
};

/// crypt_byte_str_rep //////////////////////////////////////////////////////////////////////

class __exportC crypt_byte_str_rep : public handle_rep {
public:
	typedef coder_types::byte   byte;
	typedef coder_types::uint16 uint16;
	typedef coder_types::uint32 uint32;
public:
	crypt_byte_str_rep() {}
	crypt_byte_str_rep(const byte* bytes, uint16 num_key_bytes) : Array(bytes, num_key_bytes) {}
	crypt_byte_str_rep(uint16 num_key_bytes) : Array(num_key_bytes) {}

	uint16 get_size() const { return (uint16) Array.get_size(); }
	byte*  get_bytes() { return Array.get_bytes(); }

	bool operator==(const crypt_byte_str_rep& rep2) const { return Array == rep2.Array; }
	bool operator!=(const crypt_byte_str_rep& rep2) const { return Array != rep2.Array; }

	void wipe() { Array.wipe(); }

private:
	secure_byte_array Array;
};

/// CryptByteString /////////////////////////////////////////////////////////////////////////

/*{\Manpage {CryptByteString} {} {Secure Byte String} {s}}*/

class __exportC CryptByteString : public HANDLE_BASE(crypt_byte_str_rep) {
/*{\Mdefinition
An instance |\Mvar| of the class |\Mname| is basically a string of bytes.
When |\Mvar| is not used anymore its memory is wiped out (by overwriting it a
couple of times) before the memory is freed and returned to the system. 
The goal is to prevent an attacker from reading security sensitive data after
your process has terminated. We want to point out that this mechanism can be 
foiled by the operating system: If it swaps the memory occupied by |\Mvar| to 
a swap file on a hard disc then the data will not be erased by |\Mvar|.
(Some platforms offer to lock certain parts of the memory against swapping. 
|\Mname| uses this feature on Windows NT/2000/XP to protect its memory.)\\
As we have stated above |\Mvar| can be used like a string or an array of 
bytes. The \emph{size} $n$ of |\Mvar| is the number of bytes in |\Mvar|, they
are indexed from $0$ to $n-1$.\\
{\bf Important:} If you create a |\Mname| |\Mvar| from a $C$-style array or a
string, or if you convert |\Mvar| to a |string|, then only the memory of 
|\Mvar| will be wiped out but not the memory of the array or the string.
}*/
	typedef crypt_byte_str_rep rep;
	typedef HANDLE_BASE(rep)   base;
public:
	typedef rep::byte   byte;
	typedef rep::uint16 uint16;
	typedef rep::uint32 uint32;

public:
/*{\Mcreation}*/

	         CryptByteString() { PTR = new rep(); }
	/*{\Mcreate creates an empty string.}*/

	explicit 

        CryptByteString(uint16 size) { PTR = new rep(size); }
	/*{\Mcreate creates a string of the given |size|. All bytes in |\Mvar| are 
	set to zero.}*/

	         CryptByteString(const byte* bytes, uint16 num_bytes) { PTR = new rep(bytes, num_bytes); }
	/*{\Mcreate creates a copy of the array |bytes| of size |num_bytes|.}*/

	explicit 

        CryptByteString(const char* str);
	/*{\Mcreate creates a copy of the $C$-style string |str|.
	(The '$\backslash$0' character at the end is not copied.)}*/

	CryptByteString(const CryptByteString& s) : base(s) {}
	const CryptByteString& operator=(const CryptByteString& s) { base::operator=(s); return *this; }

/*{\Moperations 3.7 3.5}*/

	uint16      get_size() const { return ptr()->get_size(); }
	/*{\Mop returns the size of |\Mvar|.}*/

	bool        is_empty() const { return ptr()->get_size() == 0; }
	/*{\Mop returns true iff |\Mvar| is empty.}*/

	void clear() { operator=( CryptByteString() ); }
	/*{\Mop makes |\Mvar| the empty string.}*/

	const byte* get_bytes() const { return ptr()->get_bytes(); }
	      byte* get_bytes() { make_single_ref(); return ptr()->get_bytes(); }
	/*{\Mop returns the internal byte-array of |\Mvar|.}*/

	byte  operator[](uint16 idx) const;

	byte& operator[](uint16 idx);
	/*{\Marrop returns the byte at position |idx|.\\
	\precond{$0 \le |idx| \le |\Mvar|.\mbox{get\_size()}-1$.}}*/

	CryptByteString operator()(uint16 idx_start, uint16 idx_end) const;
	/*{\Mfunop returns the substring $|\Mvar|[|idx_start|]$, \dots, 
	$|\Mvar|[|idx_end|]$.}*/

	CryptByteString hash(uint16 num_iterations = 1) const;
	/*{\Mop computes a hash of |\Mvar|, |num_iterations| specifies how often
	the hash function is applied.}*/

	string to_hex_string() const;
	/*{\Mop computes a hexadecimal reprasentation of |\Mvar|, each byte is 
	represented by two hex-digits.}*/

	string to_string() const;
	/*{\Mop converts |\Mvar| into a |string|.}*/

	friend CryptByteString operator+(const CryptByteString& x, const CryptByteString& y);
	/*{\Mbinopfunc returns the concatenation of |x| and |y|.}*/

	CryptByteString& operator+=(const CryptByteString& str2)
	{ *this = *this + str2; return *this; }
	
	bool operator==(const CryptByteString& str2) const { return *ptr() == *(str2.ptr()); }
	bool operator!=(const CryptByteString& str2) const { return *ptr() != *(str2.ptr()); }

	streamsize get_size_in_stream() const;
	friend __exportF istream& operator>>(istream& i, CryptByteString& str);
	friend __exportF ostream& operator<<(ostream& o, const CryptByteString& str);

	void wipe() { ptr()->wipe(); } // even if there are several ptrs to rep!

	static CryptByteString from_hex_string(const char* hex_str);
	/*{\Mstatic creates a |\Mname| from a string of hex-digits.}*/

	static CryptByteString from_string(const char* str);
	/*{\Mstatic converts the string |str| into a |\Mname|.}*/

	static void wipe_memory(void* mem, uint32 sz) { secure_byte_array::wipe_memory(mem, sz); }
	/*{\Mstatic wipes out |sz| bytes starting at the address given by |mem|.}*/

	static void wipe_memory(string& str) { wipe_memory(str.cstring(), str.length()); }

protected:
	rep* ptr() const { return (rep*)PTR; }

	void make_single_ref() 
	{ if (refs() > 1) *this = CryptByteString(ptr()->get_bytes(), ptr()->get_size()); }
};

/// CryptKey ////////////////////////////////////////////////////////////////////////////////

/*{\Manpage {CryptKey} {} {Key for Cryptography} {k}}*/

class __exportC CryptKey : public CryptByteString {
/*{\Mdefinition
Instances of the class |\Mname| store keys for cryptographic algorithms. 
|\Mname| is derived from |CryptByteString| and hence, its instances also wipe 
out their memory upon destruction. Apart from the operations of its base class
|\Mname| provides some static operations which can be helpful for key 
generation.
}*/
	typedef CryptByteString base;
public:
	typedef base::byte   byte;
	typedef base::uint16 uint16;
	typedef base::uint32 uint32;

public:
/*{\Mcreation}*/

	         CryptKey() {}
	/*{\Mcreate creates an empty key.}*/

	explicit 

        CryptKey(uint16 size) : base(size) {}
	/*{\Mcreate creates a key of the given |size|. All bytes in |\Mvar| are 
	set to zero.}*/

			 CryptKey(const byte* bytes, uint16 num_key_bytes) : base(bytes, num_key_bytes) {}
	/*{\Mcreate initializes |\Mvar| with a copy of the array |bytes| of size 
	|num_bytes|.}*/

	explicit 

        CryptKey(const CryptByteString& byte_str) : base(byte_str) {}
	/*{\Mcreate initializes |\Mvar| with |byte_str|.}*/

	explicit 

        CryptKey(const char* hex_str) : base(base::from_hex_string(hex_str)) {}
	/*{\Mcreate initializes |\Mvar| with the hexadecimal representation in |hex_str|.}*/

// NOTE: We do not provide any implicit conversion from CryptByteString to CipherKey!

public:
/*{\Mtext \headerline{Key generation}
A key generation algorithm takes as input a (partially) secret seed $s$ of 
arbitrary length and outputs a key $k$ of the desired size.
The gist of this process is to apply a secure hash function $H$ to the seed 
and to use the returned hash value as the key, i.e.~$k = H(s)$.
In our implementation we use the checksum of the |SHACoder|, which is 20 bytes 
long.
Since we want to be able to generate a key of size $n \neq 20$ we use the 
following approach: We divide the key into $p = \lceil n / 20 \rceil$ portions
$k_1, \dots, k_p$, where the size of $k_1, \dots, k_{p-1}$ is 20 and the size 
of $k_p$ is at most 20. We set $k_i = H(s \circ n \circ i)$. 
(Here ``$x \circ y$'' denotes the concatenation of $x$ and $y$, and $k_p$ is 
possibly a proper suffix of the returned hash value.) 
Then we have $k = k_1 \circ \dots \circ k_p$.

The question is now what to use as seed for the key generation. The first idea
that comes to mind is to use the passphrase supplied by the user.
In that case the process would be vulnerable to the so-called dictionary 
attack. This works as follows: Since many users tend to choose their 
passphrases carelessly, the attacker builds a dictionary of common phrases and
their corresponding keys. This gives him a set of ``likely'' keys which he 
tries first to break the system. One way to make this attack harder is to apply
the hash function several times, i.e.~instead of $k = H(s)$ we set 
$k = H(H( \dots H(s)))$. Even if we apply $H$ some thousand times it does not 
slow down the generation of a single key noticably. However, it should slow the 
generation of a whole dictionary considerably.\\
An even more effective counter measure is to use something that is called 
\emph{salt}. This is a string which is appended to the passphrase to form
the actual seed for the key generation. The salt does not have to be kept 
secret, the important point is that each key generation uses a different salt.
So it is impossible to reuse a dictionary built for a specific salt. 
The salt is also useful if you want to reuse a passphrase. E.g., if you want to
authenticate and to encrypt a file you might refrain from remembering two 
different phrases. But since it is not a good idea to use the same key twice 
you could generate two different keys by using two different salts.
}*/

	static CryptKey generate_key(uint16 key_size, 
	                             const CryptByteString& seed, 
	                             uint32 num_iterations);
	/*{\Mstatic generates a key of size |key_size| by applying a hash function
	|num_iterations| times to the given |seed|.}*/

	static CryptKey generate_key(uint16 key_size, 
	                             const CryptByteString& passphrase,
	                             const CryptByteString& salt = CryptByteString(), 
	                             uint32 num_iterations = 4096);
	/*{\Mstatic generates a key from a |passphrase| and a |salt|.
	The seed for the generation is simply $|passphrase| \circ |salt|$.}*/

	static CryptByteString generate_nonce(uint16 nonce_size, 
	                                      const CryptByteString& salt = CryptByteString(), 
										  uint32 num_iterations = 2048);

	static CryptByteString generate_salt(uint16 salt_size);
	/*{\Mstatic generates a salt as follows:
	If |salt_size| is at least sizeof(|date|)+4 then a representation of the
	current date is stored in the last sizeof(|date|) bytes of the salt. 
	The remaining bytes are filled with pseudo-random numbers from a generator 
	which is initialized with the current time.}*/

	static CryptKey generate_key_and_salt(uint16 key_size, uint16 salt_size, 
	                                      const CryptByteString& passphrase, 
										  CryptByteString& salt, 
	                                      uint32 num_iterations = 4096);
	/*{\Mstatic first some |salt| is generated (see above); then a key is 
	generated from this |salt| and the given |passphrase|.}*/

	static CryptByteString read_passphrase(const string& prompt, uint16 min_length = 6);
	/*{\Mstatic writes the |prompt| to |stdout| and then reads a passphrase 
	from |stdin| until a phrase with the specified minimum length is entered.
	(While the phrase is read |stdin| is put into unbuffered mode.)}*/
};

#if LEDA_ROOT_INCL_ID == 500712
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif

LEDA_END_NAMESPACE

#endif
