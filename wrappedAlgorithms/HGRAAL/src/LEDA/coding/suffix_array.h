#ifndef LEDA_SUFFIX_ARRAY_H
#define LEDA_SUFFIX_ARRAY_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500317
#include <LEDA/internal/PREAMBLE.h>
#endif

#include<LEDA/core/string.h>
#include<LEDA/core/list.h>
#include<LEDA/core/array.h>
#include<LEDA/internal/std/iostream.h>


LEDA_BEGIN_NAMESPACE

/*{\Manpage {suffix_array} {} {Suffix Arrays} {SA}}*/

class __exportC suffix_array
{
	/*{\Mdefinition Given a string $T[0,N-1]$ of $N$ characters, let $T[i,N-1]$ denote the $i$-th suffix
	of T and $T[0,i]$ denote the i-th prefix of $T$. The suffix array $SA$ built on string $T$ is an array containing the lexicographically ordered
	sequence of the suffixes of $T$ represented by their starting positions in $T$. Thus, SA[0] stores the
	lexicographically smallest suffix of $T$ while SA[N-1] stores the lexicographically largest suffix of $T$.
	Data type |\Mname| is a full text index, i.e., every text position of $T$ is index and thus searchable.
	We say the $T$ is indexed by $SA$ and denote by $T_{SA}$ the string, which is indexed by
	the instance $SA$ of type |\Mname|.
	Suffix arrays allow to search for string patterns $P[0,p-1]$.

	Data type |\Mname| also supports to construct the {\em longest common prefix} array (LCP). The LCP array
	stores the length of the longest common prefix between two consecutive pairs of suffixes in SA. 
	In other words, $LCP[i]$ stores the length of the longest common prefix between suffix $i-1$ and $i$ for
	$i=1, \ldots N-1$. By definition, $LCP[0]=-1$.

	}*/
	unsigned char* text;
	int* sa;
	int len;
	int* LCP;

	bool sa_computed;
	bool lcp_computed;

	enum smode {larger=1,smaller=0};
	
	
	void construct_sa(const char* t,int tlen);
	int search(const string &p, smode m) const;
	void lcp9();
	void lcp13();
	
	

public:
	/*{\Mcreation 4}*/

	suffix_array();
	/*{\Mcreate creates a suffix array for an empty string, i.e., a string containing no characters.}*/

	suffix_array(string t);
	/*{\Mcreate builds the suffix array for string $t$.}*/

	suffix_array(const char* t);
	/*{\Mcreate builds the suffix array for the text string given by character pointer $t$.}*/

	suffix_array(istream& in);
	/*{\Mcreate builds the suffix array for the text read from stream $in$.}*/

	/* copy constructor, assignment operator */

	suffix_array(const suffix_array& S);
	suffix_array& operator=(const suffix_array& S);

	/* destructor */
	~suffix_array();

	/*{\Moperations }*/

	int length() const { return len; }
	/*{\Mop returns the length of the suffix array, i.e., the length of the text on which the suffix
	array is built.}*/
	int size()   const { return len; }
	/*{\Mop returns $SA.length()$.}*/

	int operator[](int i) const;
	/*{\Marrop returns the position of the $i$-th smallest suffix in $SA$.
	\precond $0\leq i \leq SA.length()-1$.}*/

	unsigned char symbol(int i) const;
	/*{\Mop returns the $i$-th text position. \precond $0\leq i \leq SA.length()-1$.}*/
	
	void compute_lcp();
	/*{\Mop computes the LCP-array, i.e., the array storing at position $i$ the length of the
	longest common prefix of suffixes $T[SA[i-1],N-1]$ and $T[SA[i],N-1]$, $1\leq i \geq SA.length()-1$}*/

	void build(const string& t);
	/*{\Mop builds the suffix array for string $t$. The information of a previously computed
	suffix array and LCP-array is discarded.}*/

	int lcp(int i) const;
	/*{\Mop returns the length of the longest common prefix between suffix 
                $T[SA[i],N-1]$ and suffix $T[SA[i-1],N-1], 1\leq i \geq SA.length()-1$. If $i=0$, the return value is -1.}*/

	int lcp(int i, int j) const;
	/*{\Mop returns the length of the longest common prefix between suffix 
                $i$ and suffix $j, 1\leq i,j \geq SA.length()-1$.}*/

	int num_occurrences(const string& p) const;
	/*{\Mop returns the number of occurrences of string $p$ in the string $T_{SA}$.}*/

	list<int> occurrences(const string& p) const;
	/*{\Mop searches for occurrences of string $p$ in $T_{SA}$ and returns the positions of
	the occurrences as a list.}*/

	string suffix(int i) const;
	/*{returns the $i$-th suffix of $T_{SA}$.}*/
	

	void print() const;

	void print_lcp() const;

	void clear();

};

/*{\Mimplementation The suffix array uses the construction algorithm of P. Ferragina and G. Manzini as
proposed in \cite{Ferragina-Manzini}. The suffix array for text $T$ of length $n$ characters (bytes) runs in
$O(n\log n)$ time and in $5.03 n$ construction space (assuming that $n<2^32$). Searching for pattern $P$ of length $p$ is
possible in $O(p\log n)$ time. The computation of the LCP-array implements a variant of G. Manzini and
runs in $O(n)$ time and $9n$ construction space.}*/

#if LEDA_ROOT_INCL_ID == 500317
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif

LEDA_END_NAMESPACE

#endif
