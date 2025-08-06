#ifndef NEW_EDA_H_
#define NEW_EDA_H_
// recompose _dsmat from best assignments by cost
#include "../common.h"
// standard
#include <cmath>
#include <utility>
using std::pair;
#include <algorithm>
using std::nth_element;
// local
#include "dsmatrix.h"
#include "../qap.h"
#include "../Random_Number_Generator/prng.h"

class NEW_EDA {
		QAP * _problem;
		DSMATRIX * _dsmat;
		PRNG * _prng;
		vector<ASSIGNMENT *> _assignments;
		ASSIGNMENT * _best_assignment;
		int _generation_factor;
		int _generation_size;
		int _local_search_num_tries;
		double _old_prop, _new_prop;

		class FRAC_CMP {
			public:
				bool operator()(pair<int, double> const& a, pair<int, double> const& b) {
					return (modf(a.second, 0) < modf(b.second, 0));
				}
		};

		void record_best_assignment() {
			int best_i = -1;
			COST best_cost = _best_assignment->get_cached_cost();
			for (int i = 0; i < _generation_size; ++i) {
				// improve assignments with local search
				local_search(_assignments[i], _local_search_num_tries);
				// assume cost not zero else optimal
				COST const cost = _assignments[i]->compute_cost(*_problem);
				if (cost < best_cost) {
					best_i = i;
					best_cost = cost;
				}
			}
			if (best_i != -1) {
				delete _best_assignment;
				_best_assignment = _assignments[best_i];
			}
			for (LOCATION loc = 0; loc < _problem->get_size(); ++loc)
				cout << _best_assignment->get_facility(loc) << " ";
			cout << endl;
		}

		void local_search(ASSIGNMENT *const assignment,
				int const num_tries) {
			for (int i = 0; i < num_tries; ++i) {
				// randomly pick a pair of locations
				int const n = _problem->get_size();
				LOCATION const loc1 = static_cast<LOCATION>((*_prng)(n));
				LOCATION const loc2 = (static_cast<LOCATION>((*_prng)(n-1))
						+ loc1 + 1) %n;

				// compute cost difference of swapping facilities at those
				// locations
				COST const cost_diff =
						assignment->compute_swapped_cost_difference(loc1, loc2,
								*_problem);

				// swap facilities if cost difference is positive
				if (cost_diff < 0)
					assignment->swap_facilities(loc1, loc2);
			}
		}

		void quantize_weights(vector<int> & quantized_weights,
				vector<pair<int, double> > raw_weights, int const sum) {
			// check that the weights sum to sum
			int const num_weights = raw_weights.size();
			double check_sum = 0.0;
			for (int i = 0; i < num_weights; ++i)
				check_sum += raw_weights[i].second;
			assert((sum - check_sum) < 0.5);

			// sum integer parts
			int int_sum = 0;
			for (int i = 0; i < num_weights; ++i)
				int_sum += static_cast<int>(floor(raw_weights[i].second));
			int const frac_sum = sum - int_sum;
			int const num_zeros = num_weights - frac_sum;

			// convert fractional parts to 0's and 1's, minimizing squared error

			// [a] partition weights based on size of fractional parts
			FRAC_CMP frac_compare;
			nth_element(raw_weights.begin(), raw_weights.begin()
					+ num_zeros, raw_weights.end(), frac_compare);

			// [b] take floor of first partition and ceiling of second
			for (int i = 0; i < num_zeros; ++i)
				quantized_weights[raw_weights[i].first]
						= static_cast<int>(floor(raw_weights[i].second));
			for (int i = num_zeros; i < num_weights; ++i)
				quantized_weights[raw_weights[i].first]
						= static_cast<int>(ceil(raw_weights[i].second));
		}
	public:
		NEW_EDA(string const problem_file_name) :
			_prng(0) {
#ifdef SAFE_MODE
			string msg("NEW_EDA::NEW_EDA(): ");
#endif // SAFE_MODE
			try {
				_problem = new QAP(problem_file_name);
			} catch(...) {
				cout << (msg + "error reading problem.");
				throw;
			}
			int const n = _problem->get_size();
			_dsmat = new DSMATRIX(n);
			_best_assignment = new ASSIGNMENT(n);
			for (LOCATION loc = 0; loc < n; ++loc)
				_best_assignment->set_facility(loc,
						static_cast<FACILITY>(loc));
			_best_assignment->set_complete();
			_best_assignment->compute_cost(*_problem);
		}

		~NEW_EDA() {
			delete _problem;
			delete _dsmat;
			if (_prng != 0)
				delete _prng;
		}

		void start(int const generation_factor,
				int const local_search_num_tries, double const old_prop,
				string descriptor) {
			_generation_factor = generation_factor;
			_generation_size = generation_factor * _problem->get_size();
			_dsmat->fill(_generation_factor);
			if (_prng != 0)
				delete _prng;
			_prng = new PRNG(descriptor);
			_local_search_num_tries = local_search_num_tries;
			_old_prop = old_prop;
			_new_prop = 1 - _old_prop;
		}

		void step() {
			// generate random assignments from _dsmat
			_assignments.clear();
			_dsmat->remove_assignments(_assignments, _generation_size,
					*_prng);

			// improve assignments with local search and compute costs
			for (int i = 0; i < _generation_size; ++i) {
				// improve assignment with local search
				local_search(_assignments[i], _local_search_num_tries);
				// assume cost not zero else optimal
				COST const cost = _assignments[i]->compute_cost(*_problem);
			}

			// record best assignment
			record_best_assignment();

			// weight assignments according to some function of the costs
			// currently take harmonic mean as weighting function
			// later generalize to other negative powers
			vector<pair<int, double> > raw_weights(_assignments.size());
			double sum = 0.0;
			for (int i = 0; i < _generation_size; ++i) {
				raw_weights[i].first = i;
				//raw_weights[i].second = 1.0/static_cast<int>(cost);
				sum += raw_weights[i].second;
			}

			// combine old and new assignments
			// there are several possible ways to do this
			// here we try one possible method which involves allocating
			// a portion of _generation_size to the new assignments and the
			// remainder to the old assignments. Each of these portions are
			// multiplied against the raw weights of the respective sets of
			// assignments and then quantized to yield integer weights for
			// each assignment. Formation of the next dsmat then follows by
			// taking integer weight combinations of the permutation matrices
			// of the assignments

			// multiply raw weights of new assignments by portion of _generation_size

			for (int i = 0; i < _generation_size; ++i)
				raw_weights[i].second = (_new_prop * _generation_size)
						* (raw_weights[i].second/sum);

			// quantize raw weights of new assignments
			vector<int> quantized_weights(_generation_size);
			quantize_weights(quantized_weights, raw_weights,
					_generation_size);

			// compute new _dsmat
			_dsmat->fill(0);
			_dsmat->add_assignments(_assignments, quantized_weights);
		}

		void stop() {
			// generate random assignments from _dsmat
			_assignments.clear();
			// delete assignments
			_dsmat->remove_assignments(_assignments, _generation_size,
					*_prng);
			for (int i = 0; i < _generation_size; ++i) {
				// improve assignment with local search
				local_search(_assignments[i], _local_search_num_tries);
				// assume cost not zero else optimal
				COST const cost = _assignments[i]->compute_cost(*_problem);
			}
			// record best assignment
			record_best_assignment();
		}
};
#endif /*NEW_EDA_H_*/
