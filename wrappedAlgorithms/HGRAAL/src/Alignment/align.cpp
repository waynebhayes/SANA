#include "../common.h"

#include "../Random_Number_Generator/prng.h"

#include "../Hungarian/matrix.h"
#include "../Hungarian/munkres.h"

#include <vector>
using std::vector;
#include <set>
using std::set;
#include <utility>
using std::pair;
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <cmath>

void read_network_file(vector<vector<int> > & network,
        string network_file, int nedges, vector<string> labels) {
    int const maxchars = 256;
    int const nnodes = network.size();
    for (int node = 0; node < nnodes; ++node)
        network[node] = *(new vector<int>);
    ifstream ifs(network_file.c_str());
    for (int edge = 0; edge < nedges; ++edge) {
        char s1[maxchars], s2[maxchars];
        ifs.getline(s1, maxchars, '\t');
        ifs.getline(s2, maxchars, '\n');
        int node1 = -1, node2 = -1;
        for (int label = 0; label < labels.size(); ++label) {
            if (labels[label] == s1)
                node1 = label;
            else if (labels[label] == s2)
                node2 = label;
            if ((node1 != -1) && (node2 != -1))
                break;
        }
        assert(node1 != -1);
        assert(node2 != -1);
        network[node1].push_back(node2);
        network[node2].push_back(node1);
    }
}

void read_network_labels(vector<string> & network1_labels,
        int const network1_size, vector<string> & network2_labels,
        int const network2_size, string const paired_cost_matrix_file) {

    int const maxchars = 256;

    // read network paired cost matrix file
    ifstream ifs(paired_cost_matrix_file.c_str());
    ifs.ignore(maxchars, ' '); // skip corner label
    for (int col = 0; col < network2_size; ++col) {
        char s[maxchars];
        ifs.getline(s, maxchars, ' ');
        network2_labels.push_back(string(s));
    }
    ifs.ignore(maxchars, '\n');
    for (int row = 0; row < network1_size; ++row) {
        char s[maxchars];
        ifs.getline(s, maxchars, ' ');
        network1_labels.push_back(string(s));
        for (int col = 0; col < network2_size; ++col)
            ifs.ignore(maxchars, ' ');
        ifs.ignore(maxchars, '\n');
    }
}

void read_original_paired_cost_matrix(Matrix<double> & cmat,
        string const paired_cost_matrix_file) {
    int const maxchars = 256;

    ifstream cfs(paired_cost_matrix_file.c_str());
    // get length of file:
    int nlines = -1;
    string c;
    while (!getline(cfs,c).eof())
        ++nlines;
    //cout << "lines = " << nlines << endl;
    cfs.close();

    ifstream ifs(paired_cost_matrix_file.c_str());
    string s;
    getline(ifs, s);
    s += ' ';
    int npos = 0, lpos = 0, network2_size = -1;
    while ((npos = (int)s.find(' ', lpos)) != string::npos) {
        ++network2_size;
        lpos = npos+1;
    }
    //cout << "second = " << network2_size << endl;

    cmat.resize(nlines, network2_size);
    for (int row = 0; row < nlines; ++row) {
        ifs.ignore(maxchars, ' ');
        for (int col = 0; col < network2_size; ++col) {
            double d; ifs >> d;
            //cout << d << " ";
            cmat.set(row, col, d);
        }
        //cout << endl;
    }
}

void find_one_optimal_alignment(string const res_align_file,
        string const res_paired_cost_matrix_file,
        string const paired_cost_matrix_file) {
    // read paired cost matrix file
    Matrix<double> cmat;
    read_original_paired_cost_matrix(cmat, paired_cost_matrix_file);
    int network1_size = cmat.rows();
    int network2_size = cmat.columns();
    //cout << network1_size << " " << network2_size << endl;

    // call Hungarian algorithm routine
    vector<int> sol(network1_size);
    Matrix<double> d(cmat);
    Munkres m;

    time_t begin = time(0);
    m.simple_solve(sol, cmat);
    time_t end = time(0);
    double const elapsed = difftime(end, begin);

    double cost = 0.0;
    for (int row = 0; row < network1_size; ++row)
        cost += d(row, sol[row]);

    // write results to file
    {
        ofstream ofs(res_align_file.c_str());
        for (int row = 0; row < network1_size; ++row)
            ofs << sol[row] << endl;
        ofs << endl;

        ofs << "Time = " << elapsed << endl;
        ofs.setf(ios_base::fixed, ios_base::floatfield);
        ofs.precision(11);
        ofs << "Cost = " << cost << endl;
        ofs.close();
    }

    // write paired cost matrix file
    {
        ofstream ofs(res_paired_cost_matrix_file.c_str());
        ofs.setf(ios_base::fixed, ios_base::floatfield);
        ofs.precision(11);
        for (int row = 0; row < network1_size; ++row) {
            for (int col = 0; col < network2_size; ++col)
                ofs << cmat.get(row, col) << " ";
            ofs << endl;
        }
    }
}

double compute_edge_match_statistic(string const network1_file,
        vector<string> network1_labels, int const network1_size,
        int const nedges1, string const network2_file,
        vector<string> network2_labels, int const network2_size,
        int const nedges2, string const res_align_file) {

    // read matching from file
    vector<int> sol(network1_size);
    ifstream ifs(res_align_file.c_str());
    for (int row = 0; row < network1_size; ++row)
        ifs >> sol[row];

    // read network files
    vector<vector<int> > network1(network1_size),
            network2(network2_size);
    read_network_file(network1, network1_file, nedges1, network1_labels);
    read_network_file(network2, network2_file, nedges2, network2_labels);

    // compute edge count matches
    int edge_match_cnt = 0;
    int sol_edges = 0;
    for (int row1 = 0; row1 < network1_size; ++row1) {
        int const col1 = sol[row1];
        if (col1 == -1)
            continue;
        sol_edges += network1[row1].size();
        for (int row2 = 0; row2 < network1[row1].size(); ++row2) {
            int const col2 = sol[network1[row1][row2]];
            for (int col = 0; col < network2[col1].size(); ++col)
                if (network2[col1][col] == col2) {
                    ++edge_match_cnt;
                    break;
                }
        }
    }

    // because edges are double counted
    double const edge_match_cnt_d = edge_match_cnt/2.0;
    sol_edges /= 2;
    cout << sol_edges << " " << nedges1 << endl;
    return (edge_match_cnt_d /static_cast<double>(sol_edges))*100;
}

void find_optimal_edges(string const res_opt_edges_file,
        string const res_align_file,
        string const res_paired_cost_matrix_file,
        string const paired_cost_matrix_file, int const network1_size,
        int const network2_size, int const start_row, int const end_row) {

    // read matching from file
    vector<int> sol(network1_size);
    {
        ifstream ifs(res_align_file.c_str());
        for (int row = 0; row < network1_size; ++row)
            ifs >> sol[row];
    }

    // read resulting paired cost matrix from file
    Matrix<double> res_cmat(network1_size, network2_size);
    {
        ifstream ifs(res_paired_cost_matrix_file.c_str());
        for (int row = 0; row < network1_size; ++row)
            for (int col = 0; col < network2_size; ++col) {
                double tmp;
                ifs >> tmp;
                res_cmat.set(row, col, tmp);
            }
    }

    // read original paired cost matrix from file
    Matrix<double> ori_cmat(network1_size, network2_size);
    read_original_paired_cost_matrix(ori_cmat,
            paired_cost_matrix_file);

    vector<set<int> > opt_edges;

    // call Hungarian algorithm routine
    Munkres m;

    time_t begin = time(0);
    m.find_optimal_edge_set(opt_edges, sol, res_cmat, ori_cmat,
            start_row, end_row);
    time_t end = time(0);
    double const elapsed = difftime(end, begin);
    cout << "Time = " << elapsed << endl;

    // write optimal edges to file
    {
        ofstream ofs(res_opt_edges_file.c_str());
        ofs << start_row << " "; // start node
        ofs << end_row << " "; // end node
        ofs << network2_size << endl; // number of nodes in network2
        for (int j = 0; j < opt_edges.size(); ++j) {
            ofs << opt_edges[j].size() << " "; // edges per row
            for (set<int>::iterator it = opt_edges[j].begin(); it
                    != opt_edges[j].end(); ++it)
                ofs << (*it) << " ";
            ofs << endl;
        }
    }
}

void find_bipartite_connected_components(
        string const res_components1_file,
        string const res_components2_file, string const res_opt_edges_file) {

    // read optimal edges file
    ifstream ifs(res_opt_edges_file.c_str());
    int network1_nnode, network2_nnode;
    ifs >> network1_nnode >> network2_nnode;
    vector<set<int> > opt_edges(network1_nnode), inv_opt_edges(
            network2_nnode);
    for (int node1 = 0; node1 < network1_nnode; ++node1) {
        int network2_nnode;
        ifs >> network2_nnode;
        for (int node2 = 0; node2 < network2_nnode; ++node2) {
            int tmp;
            ifs >> tmp;
            opt_edges[node1].insert(tmp);
            inv_opt_edges[tmp].insert(node1);
        }
    }

    vector<vector<int> > components1, components2;
    int comp_cnt = 0;
    for (int marker = 0; marker < network1_nnode; ++comp_cnt) {
        set<int> component1, component2;
        component1.insert(marker);
        for (;;) {
            int const ncomponent1 = component1.size();
            int const ncomponent2 = component2.size();
            for (set<int>::iterator it = component1.begin(); it
                    != component1.end(); ++it) {
                set<int> s = opt_edges[*it];
                component2.insert(s.begin(), s.end());
            }
            for (set<int>::iterator it = component2.begin(); it
                    != component2.end(); ++it) {
                set<int> s = inv_opt_edges[*it];
                component1.insert(s.begin(), s.end());
            }
            if ((ncomponent1 == component1.size()) && (ncomponent2
                    == component2.size()))
                break;
        }
        components1.push_back(*(new vector<int>));
        components2.push_back(*(new vector<int>));
        for (set<int>::iterator it = component1.begin(); it
                != component1.end(); ++it) {
            components1[comp_cnt].push_back(*it);
            opt_edges[*it].clear();
        }
        for (set<int>::iterator it = component2.begin(); it
                != component2.end(); ++it) {
            components2[comp_cnt].push_back(*it);
            inv_opt_edges[*it].clear();
        }
        while ((++marker < network1_nnode) && (opt_edges[marker].size()
                == 0))
            ;
    }

    // write components (network1) to file
    ofstream ofs(res_components1_file.c_str());
    ofs << comp_cnt << endl;
    for (int i = 0; i < comp_cnt; ++i) {
        ofs << components1[i].size() << " ";
        for (int j = 0; j < components1[i].size(); ++j)
            ofs << components1[i][j] << " ";
        ofs << endl;
    }
    ofs.close();

    // write components (network2) to file
    ofs.open(res_components2_file.c_str());
    ofs << comp_cnt << endl;
    for (int i = 0; i < comp_cnt; ++i) {
        ofs << components2[i].size() << " ";
        for (int j = 0; j < components2[i].size(); ++j)
            ofs << components2[i][j] << " ";
        ofs << endl;
    }
}

void find_all_optimal_alignments(string const res_all_align_file,
        string const res_components1_file,
        string const res_components2_file, string const res_align_file,
        string const res_opt_edges_file,
        string const paired_cost_matrix_file) {

    // read optimal edges file
    vector<set<int> > opt_edges, inv_opt_edges;
    int network1_nnode, network2_nnode;
    {
        ifstream ifs(res_opt_edges_file.c_str());
        ifs >> network1_nnode >> network2_nnode;
        opt_edges.resize(network1_nnode);
        inv_opt_edges.resize(network2_nnode);
        for (int node1 = 0; node1 < network1_nnode; ++node1) {
            int network2_nnode;
            ifs >> network2_nnode;
            for (int node2 = 0; node2 < network2_nnode; ++node2) {
                int tmp;
                ifs >> tmp;
                opt_edges[node1].insert(tmp);
                inv_opt_edges[tmp].insert(node1);
            }
        }
    }

    // read network1 components from file
    int comp_cnt;
    vector<vector<int> > components1;
    {
        ifstream ifs(res_components1_file.c_str());
        ifs >> comp_cnt;
        components1.resize(comp_cnt);
        for (int i = 0; i < comp_cnt; ++i) {
            int network2_nnode;
            ifs >> network2_nnode;
            for (int j = 0; j < network2_nnode; ++j) {
                int tmp;
                ifs >> tmp;
                components1[i].push_back(tmp);
            }
        }
    }

    // read network2 components from file
    vector<vector<int> > components2;
    {
        ifstream ifs(res_components2_file.c_str());
        ifs >> comp_cnt;
        components2.resize(comp_cnt);
        for (int i = 0; i < comp_cnt; ++i) {
            int network2_nnode;
            ifs >> network2_nnode;
            for (int j = 0; j < network2_nnode; ++j) {
                int tmp;
                ifs >> tmp;
                components2[i].push_back(tmp);
            }
        }
    }

    // read paired cost matrix
    Matrix<double> cmat(network1_nnode, network2_nnode);
    read_original_paired_cost_matrix(cmat, paired_cost_matrix_file);

    // read matching from file
    vector<int> sol(network1_nnode);
    {
        ifstream ifs(res_align_file.c_str());
        for (int row = 0; row < network1_nnode; ++row)
            ifs >> sol[row];
    }

    vector<vector<vector<int> > > alignments;
    double approx_num_alignments = 1.0;
    for (int i = 0; i < comp_cnt; ++i) {
        alignments.push_back(*(new vector<vector<int> >));
        double sum = 0.0;
        for (int j = 0; j < components1[i].size(); ++j)
            sum += cmat(components1[i][j], sol[components1[i][j]]);

        cout << "Component = " << i << endl;
        int cnt = 0;
        do {
            double tmp = 0.0;
            for (int j = 0; j < components1[i].size(); ++j)
                tmp += cmat(components1[i][j], components2[i][j]);
            cout << "tmp = " << tmp << endl;
            if (tmp == sum) {
                cout << "[" << cnt << "]";
                alignments[i].push_back(*(new vector<int>(components2[i])));
                ++cnt;
                for (int j = 0; j < components1[i].size(); ++j)
                    cout << components2[i][j] << " ";
                cout << endl;
            }
        } while (next_permutation(components2[i].begin(),
                components2[i].end()));
        cout << endl;
        approx_num_alignments *= cnt;
    }
    cout << "Approximate number of alignments = "
            << approx_num_alignments << endl;

    // write optimal alignments to file
    {
        ofstream ofs(res_all_align_file.c_str());
        ofs << comp_cnt << endl;
        for (int i = 0; i < comp_cnt; ++i) {
            ofs << alignments[i].size() << " " << components1[i].size()
                    << endl;
            for (int j = 0; j < alignments[i].size(); ++j) {
                for (int k = 0; k < components1[i].size(); ++k) {
                    ofs << alignments[i][j][k] << " ";
                }
                ofs << endl;
            }
        }
    }
}

void find_all_optimal_alignments_by_node_match(
        string const res_all_align_node_match_file,
        string const res_all_align_file, string const res_components1_file) {

    // read optimal alignments from file
    vector<vector<vector<int> > > alignments;
    int comp_cnt;
    {
        ifstream ifs(res_all_align_file.c_str());
        ifs >> comp_cnt;
        for (int i = 0; i < comp_cnt; ++i) {
            int num_comp_sol, comp_sol_size;
            ifs >> num_comp_sol;
            ifs >> comp_sol_size;
            alignments.push_back(*(new vector<vector<int> >));
            for (int j = 0; j < num_comp_sol; ++j) {
                vector<int> comp_sol;
                for (int k = 0; k < comp_sol_size; ++k) {
                    int tmp;
                    ifs >> tmp;
                    comp_sol.push_back(tmp);
                }
                alignments[i].push_back(*(new vector<int>(comp_sol)));
            }
        }
    }

    // read network1 components from file
    vector<vector<int> > components1;
    {
        ifstream ifs(res_components1_file.c_str());
        ifs >> comp_cnt;
        components1.resize(comp_cnt);
        for (int i = 0; i < comp_cnt; ++i) {
            int network2_nnode;
            ifs >> network2_nnode;
            for (int j = 0; j < network2_nnode; ++j) {
                int tmp;
                ifs >> tmp;
                components1[i].push_back(tmp);
            }
        }
    }

    int total_match = 0;
    vector<vector<int> > max_comp_sol_indices;
    int num_best_matches = 1;
    for (int i = 0; i < comp_cnt; ++i) {
        vector<int> comp_sol_indices;
        int max_num_match = 0;
        for (int j = 0; j < alignments[i].size(); ++j) {
            int num_match = 0;
            for (int k = 0; k < alignments[i][j].size(); ++k)
                if (components1[i][k] == alignments[i][j][k])
                    ++num_match;
            if (num_match > max_num_match) {
                max_num_match = num_match;
                comp_sol_indices.clear();
            }
            if (num_match >= max_num_match)
                comp_sol_indices.push_back(j);
        }
        max_comp_sol_indices.push_back(*(new vector<int>(comp_sol_indices)));
        total_match += max_num_match;
        num_best_matches *= comp_sol_indices.size();
    }

    for (int i = 0; i < comp_cnt; ++i) {
        for (int j = 0; j < max_comp_sol_indices[i].size(); ++j)
            cout << max_comp_sol_indices[i][j] << " ";
        cout << endl;
    }
    cout << "Number of best matches = " << num_best_matches << endl;
    cout << "Best match count = " << total_match << endl;
}

void randomly_generate_alignment(string const res_align_file,
        string const res_components1_file,
        string const res_components2_file,
        string const res_opt_edges_file, int const seed) {

    // read optimal edges file
    vector<vector<int> > opt_edges;
    int network1_nnode, network2_nnode;
    {
        ifstream ifs(res_opt_edges_file.c_str());
        ifs >> network1_nnode >> network2_nnode;
        opt_edges.resize(network1_nnode);
        for (int node1 = 0; node1 < network1_nnode; ++node1) {
            int network2_nnode;
            ifs >> network2_nnode;
            for (int node2 = 0; node2 < network2_nnode; ++node2) {
                int tmp;
                ifs >> tmp;
                opt_edges[node1].push_back(tmp);
            }
        }
    }

    // read network1 components from file
    int comp_cnt;
    vector<vector<int> > components1;
    {
        ifstream ifs(res_components1_file.c_str());
        ifs >> comp_cnt;
        components1.resize(comp_cnt);
        for (int i = 0; i < comp_cnt; ++i) {
            int network2_nnode;
            ifs >> network2_nnode;
            for (int j = 0; j < network2_nnode; ++j) {
                int tmp;
                ifs >> tmp;
                components1[i].push_back(tmp);
            }
        }
    }

    // read network2 components from file
    vector<vector<int> > components2;
    {
        ifstream ifs(res_components2_file.c_str());
        ifs >> comp_cnt;
        components2.resize(comp_cnt);
        for (int i = 0; i < comp_cnt; ++i) {
            int network2_nnode;
            ifs >> network2_nnode;
            for (int j = 0; j < network2_nnode; ++j) {
                int tmp;
                ifs >> tmp;
                components2[i].push_back(tmp);
            }
        }
    }

    PRNG prng("Randomly generate alignment " + seed);
    vector<int> sol(network1_nnode);
    set<int> sol_set;
    for (int i = 0; i < comp_cnt; ++i) {
        /*
        if (components1[i].size() == 1) {
            sol[components1[i][0]] = components2[i][0];
        } else {
            for (int j = 0; j < components1[i].size(); ++j)
                sol[components1[i][j]] = -1;
        }
        */

        vector<int> component2(components2[i]);
        for (int j = 0; j < components1[i].size(); ++j) {
            int idx = prng(component2.size());
            sol[components1[i][j]] = component2[idx];
            component2.erase(component2.begin() + idx);
        }

    }

    // write results to file
    {
        ofstream ofs(res_align_file.c_str());
        for (int row = 0; row < network1_nnode; ++row)
            ofs << sol[row] << endl;
        ofs << endl;
        ofs.close();
    }
}

void find_degree_one_nodes(string const network_file,
        vector<string> & network_labels, int const network_size,
        int const nedges) {

    // read network file
    vector<vector<int> > network(network_size);
    read_network_file(network, network_file, nedges, network_labels);

    vector<vector<int> > hubs(network_size);
    for (int i = 0; i < network_size; ++i)
        hubs.push_back(*(new vector<int>));
    for (int row = 0; row < network_size; ++row)
        if (network[row].size() == 1)
            hubs[network[row][0]].push_back(row);
    for (int row = 0; row < network_size; ++row)
        if (hubs[row].size() > 1) {
            cout << row << ": ";
            for (int i = 0; i < hubs[row].size(); ++i)
                cout << hubs[row][i] << " ";
            cout << endl;
        }
}

void convert_labels_for_optimal_edges_file(
        string const converted_res_opt_edges_file,
        string const res_opt_edges_file, vector<string> network1_labels,
        vector<string> network2_labels) {

    int network1_nnode, tmp;
    ifstream ifs(res_opt_edges_file.c_str());
    ofstream ofs(converted_res_opt_edges_file.c_str());
    ifs >> network1_nnode >> tmp;
    for (int node1 = 0; node1 < network1_nnode; ++node1) {
        int network2_nnode;
        ofs << network1_labels[node1];
        ifs >> network2_nnode;
        for (int node2 = 0; node2 < network2_nnode; ++node2) {
            int idx;
            ifs >> idx;
            ofs << "\t" << network2_labels[idx];
        }
        ofs << endl;
    }
}

void run_network_align() {
    /*
     string const network_pair_name =
     "0Krogan_2007_high_0Krogan_2007_high+5e";
     int const network1_size = 1004;
     int const network2_size = 1004;
     int const nedges1 = 8323;
     int const nedges2 = 8692;
    */

     string const network_pair_name =
     "0Krogan_2007_high_0Krogan_2007_high+10e";
     int const network1_size = 1004;
     int const network2_size = 1004;
     int const nedges1 = 8323;
     int const nedges2 = 9057;

    /*
    string const network_pair_name = "yeast2_human1_0.8";
    int const network1_size = 2390;
    int const network2_size = 9141;
    int const nedges1 = 16127;
    int const nedges2 = 41456;
    */
    /******************************************************************/

    string const directory = "Sources/Alignment/" + network_pair_name + "/";
    string const network1_file = directory + "network1";
    string const network2_file = directory + "network2";
    string const paired_cost_matrix_file = directory
            + "paired_cost_matrix";
    string const res_align_file = directory + "res_one_opt_align";
    string const res_paired_cost_matrix_file = directory
            + "res_paired_cost_matrix";
    string res_opt_edges_file = directory + "res_opt_edges";
    string converted_res_opt_edges_file = res_opt_edges_file
            + "_converted";
    string const res_components1_file = directory + "res_components1";
    string const res_components2_file = directory + "res_components2";
    string const res_all_align_file = directory + "res_all_align";
    string const res_all_align_node_match_file = directory
            + "res_all_align_node_match";
    string const tmp_align_file = directory + "tmp_align_file";
    string const res_best_edge_match_align_file = directory
            + "res_best_edge_match_align_file";

#define GET_NETWORK_LABELS
    //#define FIND_DEGREE_ONE_NODES
    #define FIND_ONE_OPTIMAL_ALIGNMENT
    //#define COMPUTE_EDGE_MATCH_STATISTIC
    //#define FIND_BEST_ALIGNMENT_BY_EDGE_MATCH_STATISTIC
    //#define FIND_OPTIMAL_EDGES
//#define CONVERT_LABELS_FOR_OPTIMAL_EDGES_FILE
    //#define FIND_BIPARTITE_CONNECTED_COMPONENTS
    //#define FIND_ALL_OPTIMAL_ALIGNMENTS
    //#define FIND_ALL_OPTIMAL_ALIGNMENTS_BY_NODE_MATCH

#ifdef GET_NETWORK_LABELS
    // get network labels
    cout << "Get network labels from paired cost matrix" << endl;
    vector<string> network1_labels, network2_labels;
    read_network_labels(network1_labels, network1_size,
            network2_labels, network2_size, paired_cost_matrix_file);
#endif // GET_NETWORK_LABELS
#ifdef FIND_DEGREE_ONE_NODES
    find_degree_one_nodes(network2_file, network2_labels,
            network2_size, nedges2);
#endif // FIND_DEGREE_ONE_NODES
#ifdef FIND_ONE_OPTIMAL_ALIGNMENT
    // find one optimal alignment
    cout << "Use Hungarian Algorithm to find one optimal matching"
    << endl;
    find_one_optimal_alignment(res_align_file,
            res_paired_cost_matrix_file, paired_cost_matrix_file);
#endif // FIND_ONE_OPTIMAL_ALIGNMENT
#ifdef COMPUTE_EDGE_MATCH_STATISTIC
    // edge match statistic
    cout << "Compute percentage matching" << endl;
    double const edge_match_pct = compute_edge_match_statistic(
            network1_file, network1_labels, network1_size, nedges1,
            network2_file, network2_labels, network2_size, nedges2,
            res_align_file);

    cout.setf(ios_base::fixed, ios_base::floatfield);
    cout.precision(2);
    cout << "Edge match % = " << edge_match_pct << endl;
#endif // COMPUTE_EDGE_MATCH_STATISTIC
#ifdef FIND_BEST_ALIGNMENT_BY_EDGE_MATCH_STATISTIC
    // find best alignment by edge match statistic
    cout << "Find best alignment by edge match statistic " << endl;
    double best_pct = 0.0;
    for (int i = 0; i < 100; ++i) {
        cout << i << endl;
        randomly_generate_alignment(tmp_align_file,
                res_components1_file, res_components2_file,
                res_opt_edges_file, i);
        double const edge_match_pct = compute_edge_match_statistic(
                network1_file, network1_labels, network1_size,
                nedges1, network2_file, network2_labels,
                network2_size, nedges2, tmp_align_file);
        if (edge_match_pct > best_pct) {
            best_pct = edge_match_pct;
            //copy_file(res_best_edge_match_align_file, tmp_align_file);
        }
        cout.setf(ios_base::fixed, ios_base::floatfield);
        cout.precision(2);
        cout << "Edge match % = " << edge_match_pct << endl;
    }
    cout << "Best edge match % = " << best_pct << endl;
#endif // FIND_BEST_ALIGNMENT_BY_EDGE_MATCH_STATISTIC
#ifdef FIND_OPTIMAL_EDGES
    // find optimal edges
    cout << "Compute set of edges from which an optimal ";
    cout << "matching must select" << endl;
    int const start_row = 2200;
    int const end_row = 2389;
    stringstream converter;
    string start_row_str, end_row_str;
    converter << start_row << " " << end_row;
    converter >> start_row_str >> end_row_str;
    res_opt_edges_file += "_" + start_row_str + "_" + end_row_str;
    find_optimal_edges(res_opt_edges_file, res_align_file,
            res_paired_cost_matrix_file, paired_cost_matrix_file,
            network1_size, network2_size, start_row, end_row);
#endif // FIND_OPTIMAL_EDGES
#ifdef CONVERT_LABELS_FOR_OPTIMAL_EDGES_FILE
    cout << "Convert optimal edge-set file to original protein labels"
            << endl;
    convert_labels_for_optimal_edges_file(converted_res_opt_edges_file,
            res_opt_edges_file, network1_labels, network2_labels);
#endif // CONVERT_LABELS_FOR_OPTIMAL_EDGES_FILE
#ifdef FIND_BIPARTITE_CONNECTED_COMPONENTS
    // identify connected components of the bipartite graph formed by
    // the optimal edges
    cout << "Compute connected components of bipartite graph ";
    cout << "formed by the optimal edges" << endl;
    find_bipartite_connected_components(res_components1_file,
            res_components2_file, res_opt_edges_file);
#endif // FIND_BIPARTITE_CONNECTED_COMPONENTS
#ifdef FIND_ALL_OPTIMAL_ALIGNMENTS
    // enumerate all optimal alignments as direct sums of component
    // alignments
    cout << "Enumerate all optimal alignments as direct sums ";
    cout << "of component alignments" << endl;
    find_all_optimal_alignments(res_all_align_file,
            res_components1_file, res_components2_file, res_align_file,
            res_opt_edges_file, paired_cost_matrix_file);
#endif // FIND_ALL_OPTIMAL ALIGNMENTS
#ifdef FIND_ALL_OPTIMAL_ALIGNMENTS_BY_NODE_MATCH
    cout << "Enumerate all optimal alignments by nodes matching"
    << endl;
    find_all_optimal_alignments_by_node_match(
            res_all_align_node_match_file, res_all_align_file,
            res_components1_file);
#endif // FIND_ALL_OPTIMAL_ALIGNMENTS_BY_NODE_MATCH
}

void run_advanced_network_align() {
    /*
     string const network1_file = "Alignment/0Krogan_2007_high.txt";
     string const network2_file = "Alignment/0Krogan_2007_high+5e.txt";
     string const cost_matrix_file =
     "Alignment/0Krogan_2007_high_0Krogan_2007_high+5e.costs";
     bool const use_node_matching = true;
     bool const use_edge_matching = true;
     int const nrows = 1004;
     int const ncols = 1004;
     int const nedges1 = 8323;
     int const nedges2 = 8692;
     */

    /*
    string const network1_file = "Alignment/0Krogan_2007_high.txt";
    string const network2_file = "Alignment/0Krogan_2007_high+10e.txt";
    string const cost_matrix_file =
            "Alignment/0Krogan_2007_high_0Krogan_2007_high+10e.costs";
    bool const use_node_matching = true;
    bool const use_edge_matching = true;
    int const nrows = 1004;
    int const ncols = 1004;
    int const nedges1 = 8323;
    int const nedges2 = 9057;
    */

     string const network1_file = "yeast2.txt";
     string const network2_file = "human1.txt";
     string const cost_matrix_file = "Alignment/yeast2_human1_0.8.costs";
     bool const use_node_matching = false;
     bool const use_edge_matching = true;
     int const nrows = 2390;
     int const ncols = 9141;
     int const nedges1 = 16127;
     int const nedges2 = 41456;

    assert(nrows <= ncols);

    int const maxchars = 256;

    // read cost matrix file
    vector<string> col_labels, row_labels;
    Matrix<double> cmat(nrows, ncols);
    {
        ifstream ifs(cost_matrix_file.c_str());
        ifs.ignore(maxchars, ' '); // skip corner label
        for (int col = 0; col < ncols; ++col) {
            char s[maxchars];
            ifs.getline(s, maxchars, ' ');
            col_labels.push_back(string(s));
        }
        ifs.ignore(maxchars, '\n');
        for (int row = 0; row < nrows; ++row) {
            char s[maxchars];
            ifs.getline(s, maxchars, ' ');
            row_labels.push_back(string(s));
            for (int col = 0; col < ncols; ++col) {
                ifs.getline(s, maxchars, ' ');
                double cost = atof(s);
                cmat.set(row, col, cost);
            }
            ifs.ignore(maxchars, '\n');
        }
    }

    // read network files
    vector<vector<int> > network1(nrows), network2(ncols);
    read_network_file(network1, network1_file, nedges1, row_labels);
    read_network_file(network2, network2_file, nedges2, col_labels);

    // call Hungarian algorithm routine
    vector<vector<int> > sols, opt_edges;
    Matrix<double> d(cmat);
    Munkres m;

    time_t begin = time(0);
    m.multi_solve(sols, opt_edges, cmat);
    time_t end = time(0);
    double const elapsed = difftime(end, begin);

    double cost = 0.0;
    for (int row = 0; row < nrows; ++row)
        cost += d(row, sols[0][row]);

    // write results to file
    ofstream ofs("res.txt");
    ofs << "Number of solutions = " << sols.size() << endl << endl;
    for (int sol = 0; sol < sols.size(); ++sol) {
        ofs << "Minimal cost matching (" << sol << ") = " << endl;
        for (int row = 0; row < nrows; ++row)
            ofs << row_labels[row] << " -- " << col_labels[sols[sol][row]]
                    << endl;

        if (use_node_matching) {
            int node_match_cnt = 0;
            for (int row = 0; row < nrows; ++row)
                if (sols[sol][row] == row)
                    ++node_match_cnt;
            ofs << endl;
            ofs.setf(ios_base::fixed, ios_base::floatfield);
            ofs.precision(2);
            double const node_match_pct = (node_match_cnt
                    /static_cast<double>(nrows))*100;
            ofs << "Node match % = " << node_match_pct << endl;
        }

        if (use_edge_matching) {
            int edge_match_cnt = 0;
            for (int row1 = 0; row1 < nrows; ++row1) {
                int const col1 = sols[sol][row1];
                for (int row2 = 0; row2 < network1[row1].size(); ++row2) {
                    int const col2 = sols[sol][network1[row1][row2]];
                    for (int col = 0; col < network2[col1].size(); ++col)
                        if (network2[col1][col] == col2) {
                            ++edge_match_cnt;
                            break;
                        }
                }
            }
            // because edges are double counted
            double const edge_match_cnt_d = edge_match_cnt/2.0;

            ofs.setf(ios_base::fixed, ios_base::floatfield);
            ofs.precision(2);
            double const edge_match_pct = (edge_match_cnt_d
                    /static_cast<double>(nedges1))*100;
            ofs << "Edge match % = " << edge_match_pct << endl << endl;
        }
    }

    ofs << "Time = " << elapsed << endl;
    ofs.setf(ios_base::fixed, ios_base::floatfield);
    ofs.precision(5);
    ofs << "Cost = " << cost << endl;
    ofs.close();

    ofstream ofs1("unambiguous_matches.txt");
    ofstream ofs2("ambiguous_matches.txt");
    int cnt1 = 0, match_cnt1 = 0, cnt2 = 0;
    for (int j = 0; j < opt_edges.size(); ++j) {
        if (opt_edges[j].size() == 1) {
            if (opt_edges[j][0] == j)
                ++match_cnt1;
            ofs1 << row_labels[j] << " -- " << col_labels[opt_edges[j][0]]
                    << endl;
            ++cnt1;
        } else {
            ofs2 << row_labels[j] << " -- ";
            for (int i = 0; i < opt_edges[j].size(); ++i)
                ofs2 << col_labels[opt_edges[j][i]] << " ";
            ofs2 << endl;
            ++cnt2;
        }
    }
    ofs2 << endl << "Number of ambiguous matches = " << cnt2;
    ofs2.close();
    ofs1 << endl << "Number of unambiguous matches = " << cnt1 << endl;
    ofs1.setf(ios_base::fixed, ios_base::floatfield);
    ofs1.precision(2);
    double const node_match_pct = (match_cnt1
            /static_cast<double>(cnt1))*100;
    ofs1 << "Node match % = " << node_match_pct;
    ofs1.close();
}
