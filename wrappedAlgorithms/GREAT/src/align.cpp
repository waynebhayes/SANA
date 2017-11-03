#include "common.h"
#include "matrix.h"
#include "munkres.h"

void read_alignment_vector(vector<int> & align, string const align_file) {
    enum error {BAD_IO, BAD_DIM};
    try {
        ifstream ifs(align_file.c_str());
        if (!ifs.good())
            throw BAD_IO;
        int network1_size; ifs >> network1_size;
        if (network1_size <= 0)
            throw BAD_DIM;
        align.resize(network1_size);
        for (int row = 0; row < network1_size; ++row) {
            if (!ifs.good())
                throw BAD_IO;            
            ifs >> align[row];
        }
        ifs.close();
    } catch(error code) {
        if (code == BAD_IO)
            cerr << "Error reading " << align_file << endl;
        else if (code == BAD_DIM) {
               cerr << align_file;
            cerr << " must have positive number of rows";
            cerr << endl;
        }
        exit(0);
    }
}

void write_alignment_vector(string const align_file, vector<int> & align) {
    int network1_size = align.size();
    ofstream ofs(align_file.c_str());
    ofs << network1_size << endl;
    for (int row = 0; row < network1_size; ++row)
        ofs << align[row] << endl;
    ofs << endl;
    ofs.close();
}

void read_paired_cost_matrix(Matrix<double> & cmat, string const 
        paired_cost_matrix_file) {
    enum error {BAD_IO, BAD_DIM, DIM_MISMATCH};
    try {
        ifstream ifs(paired_cost_matrix_file.c_str());
        if (!ifs.good())
            throw BAD_IO;
        int network1_size; ifs >> network1_size;
        if (!ifs.good())
            throw BAD_IO;
        int network2_size; ifs >> network2_size;
        if (network1_size <= 0)
            throw BAD_DIM;
        if (network1_size > network2_size)
            throw DIM_MISMATCH;
        cmat.resize(network1_size, network2_size);
        for (int row = 0; row < network1_size; ++row) {
            for (int col = 0; col < network2_size; ++col) {
                if (!ifs.good())
                    throw BAD_IO;
                double d; ifs >> d;
                cmat.set(row, col, d);
            }
        }
    } catch(error code) {
        if (code == BAD_IO)
            cerr << "Error reading " << paired_cost_matrix_file << endl;
        else if (code == BAD_DIM) {    
            cerr << paired_cost_matrix_file;
            cerr << " must have positive number of rows and columns";
            cerr << endl;
        } else if (code == DIM_MISMATCH) {
            cerr << paired_cost_matrix_file;
            cerr << " has more rows than columns" << endl;
        }
        exit(0);
    }
}

void write_paired_cost_matrix(string const paired_cost_matrix_file,
        Matrix<double> & cmat) {
    int network1_size = cmat.rows();
    int network2_size = cmat.columns();
    ofstream ofs(paired_cost_matrix_file.c_str());
    ofs.setf(ios_base::fixed, ios_base::floatfield);
    ofs.precision(11);
    ofs << network1_size << " " << network2_size << endl;
    for (int row = 0; row < network1_size; ++row) {
        for (int col = 0; col < network2_size; ++col)
            ofs << cmat.get(row, col) << " ";
        ofs << endl;
    }
}

void write_opt_pairs(string const opt_pairs_file,
        vector<set<int> > & opt_pairs, int const start_row, int const end_row) {
    ofstream ofs(opt_pairs_file.c_str());
    ofs << start_row << " " << end_row << endl;
    int const opt_pairs_size = opt_pairs.size();
    for (int j = 0; j < opt_pairs_size; ++j) {
        for (set<int>::iterator it = opt_pairs[j].begin(); it
                != opt_pairs[j].end(); ++it)
            ofs << (*it) << " ";
        ofs << endl;
    }
}

void find_one_opt_align(string const align_file,
        string const mod_paired_cost_matrix_file,
        string const ori_paired_cost_matrix_file) {
    Matrix<double> ori_cmat;
    read_paired_cost_matrix(ori_cmat, ori_paired_cost_matrix_file);
    cerr << "Inputs are valid" << endl;

    Munkres m;
    vector<int> align;
    time_t const begin = time(0);
    m.solve(align, ori_cmat);
    time_t const end = time(0);
    cout << endl << "Time = " << difftime(end, begin) << " seconds." << endl;

    write_alignment_vector(align_file, align);
    write_paired_cost_matrix(mod_paired_cost_matrix_file, ori_cmat);
}

void find_opt_pairs(string const opt_pairs_file,
        string const one_opt_align_file,
        string const mod_paired_cost_matrix_file,
        string const ori_paired_cost_matrix_file,
        int const start_row, int const end_row) {            
    Matrix<double> ori_cmat;
    read_paired_cost_matrix(ori_cmat, ori_paired_cost_matrix_file);
    Matrix<double> mod_cmat;
    read_paired_cost_matrix(mod_cmat, mod_paired_cost_matrix_file);
    if ((ori_cmat.rows() != mod_cmat.rows()) ||
            (ori_cmat.columns() != mod_cmat.columns())) {
        cerr << ori_paired_cost_matrix_file << " and ";
        cerr << mod_paired_cost_matrix_file;
        cerr << " do not have the same dimensions" << endl;
        exit(0);
    }
    vector<int> align;
    read_alignment_vector(align, one_opt_align_file);
    if (static_cast<int>(align.size()) != ori_cmat.rows()) {
        cerr << one_opt_align_file << " and the paired cost matrices ";
        cerr << "do not have the same number of rows" << endl;
        exit(0);
    }
    cerr << "Inputs are valid" << endl;

    Munkres m;
    vector<set<int> > opt_pairs;
    time_t const begin = time(0);
    m.find_opt_pairs(opt_pairs, align, mod_cmat, ori_cmat, start_row, 
            end_row);
    time_t const end = time(0);
    cout << endl << "Time = " << difftime(end, begin) << " seconds." << endl;
    
    write_opt_pairs(opt_pairs_file, opt_pairs, start_row, end_row);
}

#ifdef FIND_ONE_OPT_ALIGN
int main(int argc, char * argv[]) {
    if (argc != 4) {
        string const usage("Usage: find_one_opt_align <one_opt_align_file> "
        "<mod_paired_cost_matrix_file> <ori_paired_cost_matrix_file>");
        cout << usage << endl;
        return 0;
    }
    string one_opt_align_file(argv[1]);
    string mod_paired_cost_matrix_file(argv[2]);
    string ori_paired_cost_matrix_file(argv[3]);

    cout << "Program: find_one_opt_align" << endl << endl;
    cout << "Output alignment will be stored at: ";
    cout << one_opt_align_file << endl;
    cout << "Output modified paired cost matrix will be stored at: ";
    cout << mod_paired_cost_matrix_file << endl;
    cout << "Input original paired cost matrix will be read at: ";
    cout << ori_paired_cost_matrix_file << endl << endl;
    find_one_opt_align(one_opt_align_file,
            mod_paired_cost_matrix_file, ori_paired_cost_matrix_file);
}
#endif /* FIND_ONE_OPT_ALIGN */

#ifdef FIND_OPT_PAIRS
int main(int argc, char * argv[]) {
    if (argc != 7) {
        string const usage("Usage: find_opt_pairs <opt_pairs_file> "
        "<one_opt_align_file> <mod_paired_cost_matrix_file> "
        "<ori_paired_cost_matrix_file> <start_row> <end_row>");
        cout << usage << endl;
        return 0;
    }
    string opt_pairs_file(argv[1]);
    string one_opt_align_file(argv[2]);
    string mod_paired_cost_matrix_file(argv[3]);
    string ori_paired_cost_matrix_file(argv[4]);
    int start_row = atoi(argv[5]);
    int end_row = atoi(argv[6]);
    
    stringstream converter;
    string start_row_str, end_row_str;
    converter << start_row << " " << end_row;
    converter >> start_row_str >> end_row_str;
    opt_pairs_file += "_" + start_row_str + "_" + end_row_str;

    cout << "Program: find_opt_pairs" << endl << endl;
    cout << "Output optimal pairs will be stored at: ";
    cout << opt_pairs_file << endl;
    cout << "Input optimal alignment will be read at: ";
    cout << one_opt_align_file << endl;
    cout << "Input modified paired cost matrix file will be read at: ";
    cout << mod_paired_cost_matrix_file << endl;
    cout << "Input original paired cost matrix file will be read at: ";
    cout << ori_paired_cost_matrix_file << endl;
    cout << "Input start row index: " << start_row << endl;
    cout << "Input end row index: " << end_row << endl << endl;
    find_opt_pairs(opt_pairs_file, one_opt_align_file,
            mod_paired_cost_matrix_file, ori_paired_cost_matrix_file,
            start_row, end_row);
}
#endif /* FIND_OPT_PAIRS */
