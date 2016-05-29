/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  coder_pipe.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:44:37 $

#ifndef _LEDA_CODER_PIPE_H
#define _LEDA_CODER_PIPE_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500396
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/coding/coder_base.h>
#include <LEDA/coding/coder_strbuf_adapter.h>

LEDA_BEGIN_NAMESPACE

class __exportC coder_pipe : public coder_base {
public:
	typedef coder_base base;
	enum { ID = 0x434c0b };

public:
	coder_pipe(unsigned num_coders = 1);
	virtual ~coder_pipe();

	virtual string get_description() const;

	virtual bool is_standalone_coder() const { return false; }
	virtual bool is_single_pass_encoder() const;
	virtual bool is_optimizing_seeks() const;
	virtual bool needs_seeks_during_coding() const;
	virtual bool is_secrecy_sensitive() const;

protected:
	coder_base* get_coder(unsigned coder_idx) const;
	void        set_coder(unsigned coder_idx, coder_base* coder, bool own_coder = false);

protected:
	virtual const char* get_class_name() const { return "coder_pipe"; }
	virtual id_type get_id() const { return ID; } 

	virtual void write_id_to_tgt_stream();
	virtual void read_and_check_id_in_src_stream();
	virtual uint32 get_id_length(bool& is_exact) const;
	virtual void finish_construction_from_stream(id_type id_in_stream, streambuf* stream, streambuf* additional_data);

	virtual void write_header_to_tgt_stream();
	virtual void read_header_from_src_stream();
	virtual uint32 get_header_length(bool& is_exact) const;

	virtual void init_encoding();
	virtual void encode_stream();
	virtual void encoding_finished(); 

	virtual void init_decoding();
	virtual void decode_stream();
	virtual void decoding_finished(); 

	virtual void do_reset(reset_type t); 
		// same state as the default ctor, but keeps all coders 
		// (for every coder reset(hard) is called)

	virtual streampos decode_seek(streampos current_pos, streampos new_pos);
	virtual streambuf* change_src_stream_temporarily(streambuf* src_stream);
	virtual streambuf* change_tgt_stream_temporarily(streambuf* tgt_stream);

private:
	void init_adapter_array();
	void ensure_suitable_adapters();

	coder_base* get_coder1() const { return pCoder1; }
	void        set_coder1(coder_base* c1, bool own_coder = false);

private:
	unsigned               NumberOfCoders;
	coder_base*            pCoder1;
	coder_strbuf_adapter** AdaptersForCoders; // array of pointers
	bool                   OwnCoder1;
	bool                   HasSeekAdapters;
};

#if LEDA_ROOT_INCL_ID == 500396
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif

LEDA_END_NAMESPACE

#endif
