/*****************************************************************/
// Implementation of the robust taboo search of: E. Taillard
// "Robust taboo search for the quadratic assignment problem", 
// Parallel Computing 17, 1991, 443-455.
//
// Data file format: 
//  n,
// (nxn) flow matrix,
// (nxn) distance matrix
//
// Copyright : E. Taillard, 1990-2004
// This code can be freely used for non-commercial purpose.
// Any use of this implementation or a modification of the code
// must acknowledge the work of E. Taillard
/****************************************************************/

#include <iostream>
#include <fstream>
#include <vector>

using std::vector;
using std::cout;
using std::cin;
using std::ifstream;

const long infinite = 999999999;

typedef int*   type_vector;
typedef long** type_matrix;


/*************** L'Ecuyer random number generator ***************/
const long m = 2147483647; const long m2 = 2145483479; 
const long a12= 63308; const long q12=33921; const long r12=12979; 
const long a13=-183326; const long q13=11714; const long r13=2883; 
const long a21= 86098; const long q21=24919; const long r21= 7417; 
const long a23=-539608; const long q23= 3976; const long r23=2071;
const double invm = 4.656612873077393e-10;

long x10 = 12345, x11 = 67890, x12 = 13579, // init. de la
     x20 = 24680, x21 = 98765, x22 = 43210; // valeur des germes

double rando()
 {long h, p12, p13, p21, p23;
  h = x10/q13; p13 = -a13*(x10-h*q13)-h*r13;
  h = x11/q12; p12 = a12*(x11-h*q12)-h*r12;
  if (p13 < 0) p13 = p13 + m; if (p12 < 0) p12 = p12 + m;
  x10 = x11; x11 = x12; x12 = p12-p13; if (x12 < 0) x12 = x12 + m;
  h = x20/q23; p23 = -a23*(x20-h*q23)-h*r23;
  h = x22/q21; p21 = a21*(x22-h*q21)-h*r21;
  if (p23 < 0) p23 = p23 + m2; if (p21 < 0) p21 = p21 + m2;
  x20 = x21; x21 = x22; x22 = p21-p23; if(x22 < 0) x22 = x22 + m2;
  if (x12 < x22) h = x12 - x22 + m; else h = x12 - x22;
  if (h == 0) return(1.0); else return(h*invm);
 }

/*********** return an integer between low and high *************/
long unif(long low, long high)
 {return(low + long(double(high - low + 1) * rando() ));}

void transpose(int & a, int & b) {long temp = a; a = b; b = temp;}

int min(long a, long b) {if (a < b) return(a); else return(b);}



/*--------------------------------------------------------------*/
/*       compute the cost difference if elements i and j        */
/*         are transposed in permutation (solution) p           */
/*--------------------------------------------------------------*/
long compute_delta(int n, type_matrix & a, type_matrix & b,
                   type_vector & p, int i, int j)
 {long d; int k;
  d = (a[i][i]-a[j][j])*(b[p[j]][p[j]]-b[p[i]][p[i]]) +
      (a[i][j]-a[j][i])*(b[p[j]][p[i]]-b[p[i]][p[j]]);
  for (k = 1; k <= n; k++) if (k!=i && k!=j)
    d = d + (a[k][i]-a[k][j])*(b[p[k]][p[j]]-b[p[k]][p[i]]) +
            (a[i][k]-a[j][k])*(b[p[j]][p[k]]-b[p[i]][p[k]]);
  return(d);
 }

/*--------------------------------------------------------------*/
/*      Idem, but the value of delta[i][j] is supposed to       */
/*    be known before the transposition of elements r and s     */
/*--------------------------------------------------------------*/
long compute_delta_part(type_matrix & a, type_matrix & b,
                        type_vector & p, type_matrix & delta, 
                        int i, int j, int r, int s)
  {return(delta[i][j]+(a[r][i]-a[r][j]+a[s][j]-a[s][i])*
     (b[p[s]][p[i]]-b[p[s]][p[j]]+b[p[r]][p[j]]-b[p[r]][p[i]])+
     (a[i][r]-a[j][r]+a[j][s]-a[i][s])*
     (b[p[i]][p[s]]-b[p[j]][p[s]]+b[p[j]][p[r]]-b[p[i]][p[r]]) );
  }

void tabu_search(long n,                  // problem size
                 type_matrix & a,         // flows matrix
                 type_matrix & b,         // distance matrix
                 type_vector & best_sol,  // best solution found
                 long & best_cost,        // cost of best solution
                 long min_size,           // parameter 1 (< n^2/2)
                 long max_size,           // parameter 2 (< n^2/2)
                 long aspiration,         // parameter 3 (> n^2/2)
                 long nr_iterations,      // number of iterations 
                 bool verbose)
           
 
 {type_vector p;                        // current solution
  type_matrix delta;                    // store move costs
  type_matrix tabu_list;                // tabu status
  long current_iteration;               // current iteration
  long current_cost;                    // current sol. value
  int i, j, k, i_retained, j_retained;  // indices

  /***************** dynamic memory allocation *******************/
  p = new int[n+1];
  delta = new long* [n+1];
  for (i = 1; i <= n; i++) delta[i] = new long[n+1];
  tabu_list = new long* [n+1];
  for (i = 1; i <= n; i++) tabu_list[i] = new long[n+1];

  /************** current solution initialization ****************/
  for (i = 1; i <= n; i = i + 1) p[i] = best_sol[i];

  /********** initialization of current solution value ***********/
  /**************** and matrix of cost of moves  *****************/
  if(verbose) cout << "Initializing solution\n";
  ProgressBar pbar(n*n);
  current_cost = 0;
  for (i = 1; i <= n; i++) for (j = 1; j <= n; j++){
    if(verbose) pbar.update();
    current_cost += a[i][j] * b[p[i]][p[j]];
    if(i < j) delta[i][j] = compute_delta(n, a, b, p, i, j);
  }
  best_cost = current_cost;

  /****************** tabu list initialization *******************/
  if(verbose) cout << "Initializing tabu list\n";
  for (i = 1; i <= n; i++) for (j = 1; j <= n; j++)
    tabu_list[i][j] = -(n*i + j);

  /******************** main tabu search loop ********************/
  //ProgressBar pbar2(nr_iterations);
  for (current_iteration = 1; current_iteration <= nr_iterations; current_iteration++)
  {/** find best move (i_retained, j_retained) **/
    //cout << "Starting iteration " << current_iteration << "\n";
    //pbar2.update();

    i_retained = infinite;       // in case all moves are tabu
    long min_delta = infinite;   // retained move cost
    int autorized;               // move not tabu?
    int aspired;                 // move forced?
    int already_aspired = false; // in case many moves forced

    for (i = 1; i < n; i++) 
      for (j = i+1; j <= n; j++){
        autorized = (tabu_list[i][p[j]] < current_iteration) || 
                    (tabu_list[j][p[i]] < current_iteration);

        aspired =
         (tabu_list[i][p[j]] < current_iteration-aspiration)||
         (tabu_list[j][p[i]] < current_iteration-aspiration)||
         (current_cost + delta[i][j] < best_cost);                

        if ((aspired && !already_aspired) || // first move aspired
           (aspired && already_aspired &&    // many move aspired
            (delta[i][j] < min_delta)   ) || // => take best one
           (!aspired && !already_aspired &&  // no move aspired yet
            (delta[i][j] < min_delta) && autorized)){
          i_retained = i; j_retained = j;
          min_delta = delta[i][j];
          if (aspired) already_aspired = true;
        }
      }

    if (i_retained == infinite) if(verbose) cout << "All moves are tabu! \n"; 
    else{/** transpose elements in pos. i_retained and j_retained **/
      //cout << "Found best move\n";
      transpose(p[i_retained], p[j_retained]);
      // update solution value
      current_cost = current_cost + delta[i_retained][j_retained];
      // forbid reverse move for a random number of iterations
      tabu_list[i_retained][p[j_retained]] = 
        current_iteration + unif(min_size,max_size);
      tabu_list[j_retained][p[i_retained]] = 
        current_iteration + unif(min_size,max_size);

      // best solution improved ?
      if (current_cost < best_cost){
        best_cost = current_cost;
        for (k = 1; k <= n; k++) best_sol[k] = p[k];
        if(verbose) cout << "Solution of value " << best_cost 
             << " found at iter. " << current_iteration << '\n';
      }

      // update matrix of the move costs
      for (i = 1; i < n; i++) for (j = i+1; j <= n; j++)
        if (i != i_retained && i != j_retained && 
            j != i_retained && j != j_retained){
          delta[i][j] = 
            compute_delta_part(a, b, p, delta, i, j, i_retained, j_retained);
        }
        else delta[i][j] = compute_delta(n, a, b, p, i, j);
     }
      
   }
  // free memory
  delete[] p;
  for (i=1; i <= n; i++) delete[] delta[i]; delete[] delta;
  for (i=1; i <= n; i++) delete[] tabu_list[i]; 
  delete[] tabu_list;
} // tabu

void generate_random_solution(long n, type_vector  & p)
{
  int i;
  for (i = 0; i <= n; i++) p[i] = i;
  for (i = 1; i <  n; i++) transpose(p[i], p[unif(i, n)]);
}


vector<int> doAlignment(int n, vector<int> av, vector<int> bv, int iters, int* seed, bool verbose)
{
  type_vector solution = new int[n+1];
  long cost;

  if(verbose) cout << "Building matrices\n";
  type_matrix a = new long* [n+1];
  type_matrix b = new long* [n+1];
  for (int i = 0; i < n; i++){
    a[i+1] = new long[n+1];
    b[i+1] = new long[n+1];
    for(int j = 0; j < n; j++){
      a[i+1][j+1] = av[i*n + j];
      b[i+1][j+1] = bv[i*n + j];
    }
  }

  if(seed) for(int i = 0; i < n; i++) solution[i+1] = seed[i] + 1;
  else{
    if(verbose) cout << "Getting random initial solution\n";
    generate_random_solution(n, solution);
  }
  if(verbose) cout << "Starting search\n";
  tabu_search(n, a, b,                     // problem data
              solution, cost,              // tabu search results
              9*n/10, 11*n/10, n*n*2,      // parameters
              iters,                       // number of iterations 
              verbose);

  vector<int> res;
  for (int i = 1; i <= n; i = i+1) res.push_back(solution[i]);
  return res;
}
