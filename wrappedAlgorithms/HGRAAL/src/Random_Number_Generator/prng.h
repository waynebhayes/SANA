#ifndef PRNG_H_
#define PRNG_H_
#include "../common.h"
// standard
#include <sstream>
using std::stringstream;
#include <string>
using std::string;
#include <algorithm>
using namespace std;
#include <vector>
// boost
#include <boost/random/mersenne_twister.hpp>
using boost::mt19937;
// local
#include "../Cryptographic_Hash/sha2.h"
#include <cstring>

// random number generating functor
class PRNG {

		boost::mt19937 mt;

		void generate_seed(uint32_t *const seed, char const*const buf) {
			sha512_ctx ctx[1]; // hash function state
			unsigned char hval[SHA512_DIGEST_SIZE]; // final message digest

			int len = strlen(buf);

			sha512_begin(ctx);
			sha512_hash((unsigned char *)buf, len, ctx);
			sha512_end(hval, ctx);

			/*
			 cout << "SHA512 Message Digest: ";
			 for (int i = 0; i < SHA512_DIGEST_SIZE; ++i)
			 printf("%02x", hval[i]);
			 cout << endl << endl;
			 */

			// !!!todo: handle endianness!!!
			memcpy((unsigned char *)seed, hval, SHA512_DIGEST_SIZE);
		}

	public:
		PRNG(string const descriptor) {
			// for mt19937, n = 624, so need 624 words = 39 SHA512 outputs to
			// seed generator
			int const SHA512_DIGEST_WORD_SIZE= SHA512_DIGEST_SIZE
					/sizeof(uint32_t);
			//uint32_t *const seed = new uint32_t[39*SHA512_DIGEST_WORD_SIZE];

			uint32_t seed[39*SHA512_DIGEST_WORD_SIZE];
			for (int i = 0; i < 39; ++i) {
				stringstream ss;
				string str_i;
				ss << i;
				ss >> str_i;
				string const part_descriptor = str_i + descriptor;
				generate_seed(seed + i*SHA512_DIGEST_WORD_SIZE,
						part_descriptor.c_str());
			}
			// initialize random number generator
			uint32_t * seed_cpy = seed; // because first parameter of seed() gets modified
			// don't do "seed_cpy + 0" else seed() generates compile error
			mt.seed<uint32_t *>(seed_cpy, seed_cpy + 39
					*SHA512_DIGEST_WORD_SIZE);
			//delete[] seed;
		}

		unsigned operator()(unsigned const sz) {
			assert(sz > 0);
			if (sz == 1) // avoid calling RNG trivially
				return 0;
			unsigned rnd, limit = (static_cast<unsigned>(-1)/sz)*sz;
			// Any integer >= limit is discarded and a new random is
			// generated. This is to eliminate modulo bias from modding by sz.
			do {
			} while ((rnd = mt()) >= limit);
			return rnd % sz; // take residue mod sz
		}
};
#endif /*PRNG_H_*/
