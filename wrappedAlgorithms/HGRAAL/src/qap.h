#ifndef QAP_H_
#define QAP_H_
#include "common.h"
// standard
#include <fstream>
using std::ifstream;
#include <vector>
using std::vector;
#include <string>
using std::string;
// local
#include "Hungarian/matrix.h"
#include <cstdint>
// 16 bits should be plenty for current problem sizes
typedef uint16_t QAP_INDEX;
typedef double QAP_VALUE;
typedef QAP_INDEX LOCATION;
typedef QAP_INDEX FACILITY;
typedef QAP_VALUE DISTANCE;
typedef QAP_VALUE FLOW;
typedef QAP_VALUE COST;
#define NO_FACILITY -1

class QAP {
		size_t _n;
		Matrix<DISTANCE> _dmat;
		Matrix<FLOW> _fmat;

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
		void read_problem_file(string const problem_file_name) {
			ifstream ifs(problem_file_name.c_str());
			ifs >> _n;
			assert(_n> 0);
			_dmat.resize(_n, _n), _fmat.resize(_n, _n);
			for (LOCATION row = 0; row < _n; ++row)
				for (LOCATION col = 0; col < _n; ++col)
					ifs >> _dmat(row, col);
			for (FACILITY row = 0; row < _n; ++row)
				for (FACILITY col = 0; col < _n; ++col)
					ifs >> _fmat(row, col);
			// at some future time incorporate more info like best lower
			// bounds etc
		}

	public:
		QAP(string const problem_file_name) {
			try {
				read_problem_file(problem_file_name);
			} catch(...) {
				throw ("QAP::QAP(): problem reading file");
			}
		}

		QAP(Matrix<DISTANCE> const& dmat, Matrix<FLOW> const& fmat) :
			_dmat(dmat), _fmat(fmat), _n(_dmat.rows()) {
		}

		~QAP() {
		}

		size_t get_size() const {
			return _n;
		}

		inline
		DISTANCE get_distance(LOCATION const src, LOCATION const dst) const {
#ifdef SAFE_MODE
			string msg("ASSIGNMENT::get_distance(): ");
			if (!is_location(src))
			throw string(msg + "bad source location index.");
			if (!is_location(dst))
			throw string(msg + "bad destination location index.");
#endif // SAFE_MODE
			return _dmat.get(static_cast<int>(src), static_cast<int>(dst));
		}

		inline
		FLOW get_flow(FACILITY const src, FACILITY const dst) const {
#ifdef SAFE_MODE
			string msg("ASSIGNMENT::get_flow(): ");
			if (!is_facility(src))
			throw string(msg + "bad source facility index.");
			if (!is_facility(dst))
			throw string(msg + "bad destination facility index.");
#endif // SAFE_MODE
			return _fmat.get(static_cast<int>(src), static_cast<int>(dst));
		}

		DISTANCE get_min_distance() {
			DISTANCE dmin = get_distance(0, 1);
			for (int row = 0; row < _n; ++row)
				for (int col = 0; col < _n; ++col)
					if ((row != col) && (dmin > get_distance(row, col)))
						dmin = get_distance(row, col);
			return dmin;
		}

		DISTANCE get_max_distance() {
			DISTANCE dmax = get_distance(0, 1);
			for (int row = 0; row < _n; ++row)
				for (int col = 0; col < _n; ++col)
					if ((row != col) && (dmax < get_distance(row, col)))
						dmax = get_distance(row, col);
			return dmax;
		}

		double get_mean_distance() {
			DISTANCE dsum = 0.0;
			for (int row = 0; row < _n; ++row)
				for (int col = 0; col < _n; ++col)
					if (row != col)
						dsum += get_distance(row, col);
			return dsum/static_cast<double>(_n*(_n-1));
		}

		FLOW get_min_flow() {
			FLOW fmin = get_flow(0, 1);
			for (int row = 0; row < _n; ++row)
				for (int col = 0; col < _n; ++col)
					if ((row != col) && (fmin > get_flow(row, col)))
						fmin = get_flow(row, col);
			return fmin;
		}

		FLOW get_max_flow() {
			FLOW fmax = get_flow(0, 1);
			for (int row = 0; row < _n; ++row)
				for (int col = 0; col < _n; ++col)
					if ((row != col) && (fmax < get_flow(row, col)))
						fmax = get_flow(row, col);
			return fmax;
		}

		double get_mean_flow() {
			DISTANCE fsum = 0.0;
			for (int row = 0; row < _n; ++row)
				for (int col = 0; col < _n; ++col)
					if (row != col)
						fsum += get_flow(row, col);
			return fsum/static_cast<double>(_n*(_n-1));
		}
};
#endif /*QAP_H_*/
