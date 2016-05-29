/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  huffman.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:44:38 $

#ifndef _LEDA_HUFFMAN_H
#define _LEDA_HUFFMAN_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500146
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/coding/coder_base.h>

// core library includes
#include <LEDA/internal/std/string.h> // -> we need the function memset

// work-around for Borland (which does not allow to use enums in unions):
#define LEDA_HUFFMAN_MAX_SYMBOLS 256
#define LEDA_HUFFMAN_MAX_NODES   (LEDA_HUFFMAN_MAX_SYMBOLS*2 - 1)

LEDA_BEGIN_NAMESPACE

/// HuffmanCoder ////////////////////////////////////////////////////////////////////////////

/*{\Manpage {HuffmanCoder} {} {Static Huffman Coder} {C}}*/

class __exportC HuffmanCoder : public coder_base {
/*{\Mdefinition
The class |\Mname| provides a static coder based on the algorithm by 
Huffman~\cite{Huffman52}.
The term \emph{static} means that a fixed model is used for encoding. 
The input is scanned twice. In the first scan the frequencies of the 
characters are counted. This information is used to precompute the 
Huffman-trees. The actual coding takes place in a second scan.
}*/
public:
	typedef coder_base base;
	enum { ID = 0x434c07 };

public:
/*{\Mcreation}*/

	HuffmanCoder(streambuf* src_stream = 0, streambuf* tgt_stream = 0, bool own_streams = false);
	/*{\Mcreate creates an instance |\Mvar| which uses the given source 
	and target streams. If |own_streams| is set, then |\Mvar| is 
	responsible for the destruction of the streams, otherwise the pointers 
	|src_stream| and |tgt_stream| must be valid during the life-time of 
	|\Mvar|.}*/

	HuffmanCoder(const char* src_file_name, const char* tgt_file_name);
	/*{\Mcreate creates an instance |\Mvar| which uses file-streams for 
	input and output.}*/

	~HuffmanCoder();

	virtual bool is_single_pass_encoder() const { return false; }

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
	/*{\Mop returns |true| if the coding is finished.}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	virtual string get_description() const;
*/
	/*{\Mop provides a description for |\Mvar|.}*/

protected:
	virtual const char* get_class_name() const { return "HuffmanCoder"; }
	virtual id_type get_id() const { return ID; }

	virtual void init_encoding();
	virtual void encode_stream();

	virtual void init_decoding();
	virtual void decode_stream();

//private:
public:
	typedef uint32 CountType; // ByteCount is also uint32!
	typedef uint16 LinkType;

	enum { MaxSymbols = LEDA_HUFFMAN_MAX_SYMBOLS, MaxNodes = LEDA_HUFFMAN_MAX_NODES, NIL = MaxNodes+1 };

	struct huff_tree_node {
		CountType count;
		LinkType child[2];
		LinkType parent;
		byte* hcode; // pointer to bitcode
		int bitcount; // #bits in bitcode
	}; 

	struct dec_tree_node {
		LinkType child[2];
	};

private:
	void build_tree();
	int  walk_tree(LinkType k, int depth, byte*& ptr_in_huff_table);

private:
	uint32 ByteCount;
	uint16 Root;
	uint16 NumActiveLeaves;
	union {
		huff_tree_node HuffTree[LEDA_HUFFMAN_MAX_NODES];
		dec_tree_node DecHuffTree[LEDA_HUFFMAN_MAX_NODES];
	};

	// for encoding
	byte*  HuffTable;

	// for decoding
	int    BitNum;
	int    LastByte;
};

/// floating_huff_tree (for AdaptiveHuffmanCoder) ///////////////////////////////////////////

class floating_huff_tree : public coder_types {
public:
	// leaves         range from 0 to NumSymbols-1
	// internal nodes range from NumSymbols to MaxNumNodes-1

	enum { 
		NumSymbols = 256, NumNodes = 2*NumSymbols-1, 
		LastLeaf = NumSymbols-1, LastNode = NumNodes-1, NilIdx = -1
	};

	enum parity_t { p_Left = 0, p_Right = 1 };

	typedef sint16 node_idx;

	struct block_t {
		uint32   Weight;
		node_idx First; // leader (= rightmost node)
		node_idx Last;  // smallest idx in block (= leftmost node)
		node_idx ParentOfFirst;
		parity_t ParityOfFirst;
		node_idx RtChildOfFirst;
		block_t* PrevBlk;
		block_t* NextBlk;

		LEDA_MEMORY(block_t);
	};

public:
	floating_huff_tree();
	~floating_huff_tree();

	void clear() { destroy(); initialize(); }

	byte     symbol_at(node_idx leaf) const { return LeafIdxToSymbol[leaf]; }
	node_idx corresp_leaf(byte symbol) const { return SymbolToLeafIdx[symbol]; }
	bool     contains(byte symbol) const { return corresp_leaf(symbol) > ZeroLeaf; }

	node_idx root() const { return ZeroLeaf == LastLeaf ? LastLeaf : LastNode; }
	node_idx zero_leaf() const { return ZeroLeaf; }
	bool is_leaf(node_idx idx) const { return idx <= LastLeaf; }
	bool is_inner_node(node_idx idx) const { return idx > LastLeaf; }
	bool is_zero_leaf(node_idx idx) const { return idx == ZeroLeaf; }

	       node_idx child(node_idx idx, parity_t which) const;
	inline node_idx parent(node_idx idx) const;
	inline parity_t parity(node_idx idx) const;
	inline uint32   weight(node_idx idx) const { return block(idx)->Weight; }

	void update(byte symbol);

	void print();

private:
	void initialize();
	void destroy();

	block_t*& block(node_idx idx) { return NodeIdxToBlock[idx]; }
	block_t*  block(node_idx idx) const { return NodeIdxToBlock[idx]; }

	inline void interchange_leaves(node_idx l1, node_idx l2);

	void slide_and_increment_inner_nodes(node_idx q);
	void slide_and_increment_leaf(node_idx& q);

	void print_subtree(node_idx idx, int level);
	void print_block_list();

private:
	node_idx ZeroLeaf; // idx of the zero leaf
	byte     LeafIdxToSymbol[NumSymbols]; // = alpha
	byte     SymbolToLeafIdx[NumSymbols]; // = rep
	block_t* NodeIdxToBlock[NumNodes];    // = block
};

inline
floating_huff_tree::node_idx 
floating_huff_tree::parent(node_idx idx) const
{
	block_t* blk = block(idx);
	return blk->ParentOfFirst - ((blk->First - idx + 1 - blk->ParityOfFirst) >> 1);
}

inline
floating_huff_tree::parity_t
floating_huff_tree::parity(node_idx idx) const
{
	block_t* blk = block(idx);
	return parity_t((blk->First - idx + blk->ParityOfFirst) & 1);
}


inline 
void 
floating_huff_tree::interchange_leaves(node_idx l1, node_idx l2)
{ 
	SymbolToLeafIdx[symbol_at(l1)] = byte(l2); SymbolToLeafIdx[symbol_at(l2)] = byte(l1);
	byte tmp = LeafIdxToSymbol[l1]; LeafIdxToSymbol[l1] = LeafIdxToSymbol[l2]; LeafIdxToSymbol[l2] = tmp;
}

/// huff_bit_stack (for AdaptiveHuffmanCoder) ///////////////////////////////////////////////

class huff_bit_stack : public coder_types {
public:
	enum { NumWords = floating_huff_tree::NumSymbols / 32 + 1  }; // +1 -> to be sure ...
	typedef floating_huff_tree::parity_t bit; // type that holds a bit

public:
	huff_bit_stack() { clear(); }

	void clear() 
	{ 
		Word = 0; PosInWord = 0;
		memset(Words, 0, NumWords * sizeof(uint32));
	}

	bool empty() const
	{
		return PosInWord == 0 && Word == 0;
	}

	void push(bit b)
	{
		Words[Word] |= uint32(b) << PosInWord;
		if (++PosInWord == 32) { PosInWord = 0; ++Word; }
	}

	bit pop()
	{
		if (--PosInWord < 0) { PosInWord = 31; --Word; }
		return bit((Words[Word] & (1 << PosInWord)) == 0 ? 0 : 1);
	}

private:
	uint32 Words[NumWords];
	signed Word, PosInWord;
};

/// AdaptiveHuffmanCoder ////////////////////////////////////////////////////////////////////

/*{\Mtext \newpage}*/
/*{\Manpage {AdaptiveHuffmanCoder} {} {Adaptive Huffman Coder} {C}}*/

class __exportC AdaptiveHuffmanCoder : public coder_base {
/*{\Mdefinition
The class |\Mname| provides an adaptive coder based on the algorithm by 
J.S.~Vitter~\cite{Vi89}. 
Vitter has proven that his algorithm achieves the best compression among all
one-pass Huffman schemes.
}*/
public:
	typedef coder_base base;
	enum { ID = 0x434c0d };

public:
/*{\Mcreation}*/

	AdaptiveHuffmanCoder(streambuf* src_stream = 0, streambuf* tgt_stream = 0, bool own_streams = false);
	/*{\Mcreate creates an instance |\Mvar| which uses the given source 
	and target streams. If |own_streams| is set, then |\Mvar| is 
	responsible for the destruction of the streams, otherwise the pointers 
	|src_stream| and |tgt_stream| must be valid during the life-time of 
	|\Mvar|.}*/

	AdaptiveHuffmanCoder(const char* src_file_name, const char* tgt_file_name);
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
	/*{\Mop returns |true| if the coding is finished.}*/

/*{\Moptions nextwarning=no }*/
/*	inherited:
	virtual string get_description() const;
*/
	/*{\Mop provides a description for |\Mvar|.}*/

protected:
	virtual const char* get_class_name() const { return "AdaptiveHuffmanCoder"; }
	virtual id_type get_id() const { return ID; }

	virtual void init_encoding();
	virtual void encode_stream();

	virtual void init_decoding();
	virtual void decode_stream();

private:
	bool has_more_bits() const { return BitsInBuffer != 0 || ReadAhead2 != EOF; }
	byte get_bit();

private:
	floating_huff_tree	Tree;
	byte				BitsInBuffer;
	byte				Buffer;

	// for encoding
	huff_bit_stack Stack;

	// for decoding
	int ReadAhead1, ReadAhead2;
};

#undef LEDA_HUFFMAN_MAX_SYMBOLS
#undef LEDA_HUFFMAN_MAX_NODES

#if LEDA_ROOT_INCL_ID == 500146
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif

LEDA_END_NAMESPACE

#endif
