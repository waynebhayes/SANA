#ifndef MUNKRES_H_
#define MUNKRES_H_
/*
 *   Copyright (c) 2007 John Weaver
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */
#include "matrix.h"

#include "common.h"
#include <list>
using std::list;
#include <utility>
using std::pair;
#include <vector>
using std::vector;
#include <set>
using std::set;

class Munkres {
	public:
		void find_opt_pairs(vector<set<int> > & opt_edges, vector<int> & sol,
		        Matrix<double> & m, Matrix<double> & ori_m, int const start_row,
		        int const end_row);
		void solve(vector<int> & sol, Matrix<double> & m);
	private:
		void countzeros(string const s);
		inline bool pair_in_list(const pair<int,int> &,
		        const list<pair<int,int> > &);
		int step12();
		int step3();
		int step4();
		int step5();
		int step6();
		Matrix<int> mask_matrix;
		Matrix<double> matrix;
		bool * row_mask;
		bool * col_mask;
		int saverow, savecol;
		int nrows, ncols;
};
#endif /*MUNKRES_H_*/
