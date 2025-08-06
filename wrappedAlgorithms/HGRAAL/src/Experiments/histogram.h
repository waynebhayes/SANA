#ifndef HISTOGRAM_H_
#define HISTOGRAM_H_
#include "../common.h"
// standard
#include <algorithm>
#include <vector>
using std::vector;
#include <cmath>

class Histogram {
private:
	double const _min;
	double const _max;
	int const _width;
	int const _height;
	double _cat_width;
	int _tot_frq;
	vector<int> _dist;
public:
	Histogram(double const min, double const max, int const width,
			int const height) :
		_min(min), _max(max), _width(width), _height(height), _tot_frq(0) {
		assert(_max > _min);
		assert(_width > 0);
		assert(_height > 0);
		_dist.resize(_height, 0);
		_cat_width = (_max-_min)/_height;
	}
	void accumulate(vector<double> const& vec) {
		vector<double>::const_iterator it;
		for (it = vec.begin(); it != vec.end(); ++it)
			if ((*it >= _min) && (*it <= _max))
				++_dist[floor((*it-_min)/_cat_width)];
		_tot_frq += vec.size();
	}
	void display() {
		int const max_frq = *max_element(_dist.begin(), _dist.end());
		double const display_width = static_cast<double>(_width);
		double const fscale = (max_frq > display_width) ? display_width/max_frq
				: 1;

		double display_min = _min;
		int display_min_i = 0;
		for (int i = 0; i < _height; ++i) {
			if (_dist[i] != 0) {
				display_min_i = i;
				break;
			}
			display_min += _cat_width;
		}

		double display_max = _max;
		int display_max_i = _height-1;
		for (int i = _height-1; i >= 0; --i) {
			if (_dist[i] != 0) {
				display_max_i = i;
				break;
			}
			display_max -= _cat_width;
		}

		cout << "---MIN---" << display_min << "---TOT_FRQ---" << _tot_frq
				<< endl;
		for (int i = display_min_i; i <= display_max_i; ++i) {
			double const frac = _dist[i]/static_cast<double>(_tot_frq);
			double const midpt = _min + (i+0.5)*_cat_width;
			cout.setf(ios_base::fixed, ios_base::floatfield);
			cout.precision(4);
			cout.width(10);
			cout << midpt << "," << frac << "|";
			for (int j = 0; j < ceil(_dist[i]*fscale); ++j)
				cout << "*";
			cout << endl;
		}
		cout << "---MAX---" << display_max << "---CAT_WIDTH---" << _cat_width
				<< endl;
	}
};
#endif /*HISTOGRAM_H_*/
