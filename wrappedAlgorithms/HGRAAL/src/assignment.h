#ifndef ASSIGNMENT_H_
#define ASSIGNMENT_H_
#include "common.h"
// standard
#include <algorithm>
using std::fill_n;
using std::swap;
#include <vector>
using std::vector;
// local
#include "Hungarian/matrix.h"
#include "qap.h"

class ASSIGNMENT {
	private:
		size_t const _n;
		bool _cost_computed, _is_complete;
		COST _cost;
		vector<FACILITY> _assigned;

#ifdef SAFE_MODE
		bool is_location(LOCATION const loc) const {
			if (loc < _n)
			return true;
			return false;
		}

		bool is_facility(FACILITY const fac) const {
			if (fac < _n)
			return true;
			return false;
		}
#endif // SAFE_MODE
		inline
		COST distance_flow_product(DISTANCE const d, FLOW const f) const {
			return static_cast<COST>(static_cast<QAP_VALUE>(d)
					*static_cast<QAP_VALUE>(f));
		}
	public:
		ASSIGNMENT(size_t const n) :
			_n(n), _cost_computed(false), _is_complete(false),
					_cost(static_cast<COST>(-1)) {
#ifdef SAFE_MODE
			string msg("ASSIGNMENT::ASSIGNMENT(): ");
			if (_n == 0)
			throw string(msg + "invalid assignment size.");
#endif // SAFE_MODE
			_assigned.resize(_n);
			fill_n(_assigned.begin(), _n, NO_FACILITY);
		}

		void set_facility(LOCATION const loc, FACILITY const fac) {
#ifdef SAFE_MODE
			string msg("ASSIGNMENT::set_facility(): ");
			if (_is_complete)
			throw string(msg + "assignment completed.");
			if (!is_facility(fac))
			throw string(msg + "bad facility.");
			if (!is_location(loc))
			throw string(msg + "bad location.");
			if (find(_assigned.begin(), _assigned.end(), fac) != _assigned.end())
			throw string(msg + "facility _assigned.");
#endif // SAFE_MODE
			_assigned[loc] = fac;
		}

		FACILITY get_facility(LOCATION const loc) const {
#ifdef SAFE_MODE
			string msg("ASSIGNMENT::get_facility(): ");
			if (!is_location(loc))
			throw string(msg + "bad location.");
#endif // SAFE_MODE
			return _assigned[loc];
		}

		void swap_facilities(LOCATION const loc1, LOCATION const loc2) {
#ifdef SAFE_MODE
			string msg("ASSIGNMENT::swap_facilities(): ");
			if (!_is_complete)
			throw string(msg + "assignment not complete.");
			if (!is_location(loc1))
			throw string(msg + "bad location 1.");
			if (!is_location(loc2))
			throw string(msg + "bad location 2.");
#endif // SAFE_MODE
			swap(_assigned[loc1], _assigned[loc2]);
		}

		bool is_complete() {
			return _is_complete;
		}

		void set_complete() {
#ifdef SAFE_MODE
			string msg("ASSIGNMENT::set_complete(): ");
			if (find(_assigned.begin(), _assigned.end(), NO_FACILITY) != _assigned.end())
			throw string(msg + "some location has no facility assigned.");
#endif // SAFE_MODE
			_is_complete = true;
		}

		// compute and cache QAP cost
		COST compute_cost(QAP const& problem) {
#ifdef SAFE_MODE
			string msg("ASSIGNMENT::compute_cost(): ");
			if (!_is_complete)
			throw string(msg + "assignment not complete.");
#endif // SAFE_MODE
			_cost = 0;
			for (LOCATION row = 0; row < _n; ++row)
				for (LOCATION col = 0; col < _n; ++col)
					if (row != col)
						_cost += distance_flow_product(problem.get_distance(row,
								col),
								problem.get_flow(_assigned[row], _assigned[col]));
			_cost_computed = true;
			return _cost;
		}

		// compute swapped QAP cost; cached cost must be present and correct
		COST compute_swapped_cost_difference(LOCATION const loc1,
				LOCATION const loc2, QAP const& problem) const {
			COST cost_diff = 0;
			for (LOCATION loc = 0; loc < _n; ++loc)
				if ((loc != loc1) && (loc != loc2)) {
					FLOW const backward_flow_diff = problem.get_distance(loc,
							loc1) - problem.get_distance(loc, loc2);
					DISTANCE const backward_distance_diff = problem.get_flow(
							_assigned[loc], _assigned[loc1]) - problem.get_flow(
							_assigned[loc], _assigned[loc2]);
					cost_diff -= distance_flow_product(backward_distance_diff,
							backward_flow_diff);
					FLOW const forward_flow_diff = problem.get_distance(loc1,
							loc) - problem.get_distance(loc2, loc);
					DISTANCE const forward_distance_diff = problem.get_flow(
							_assigned[loc1], _assigned[loc]) - problem.get_flow(
							_assigned[loc2], _assigned[loc]);
					cost_diff -= distance_flow_product(forward_distance_diff,
							forward_flow_diff);
				}
			return cost_diff;
		}

		// get cached QAP cost; caller implicitly knows which problem the
		// cost is associated with
		COST get_cached_cost() const {
#ifdef SAFE_MODE
			string msg("ASSIGNMENT::get_cached_cost(): ");
			if (!_cost_computed)
			throw string("ASSIGNMENT: cache empty.");
#endif // SAFE_MODE
			return _cost;
		}
};
#endif /*ASSIGNMENT_H_*/
