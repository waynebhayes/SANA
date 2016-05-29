#include <iostream>
#include <fstream>
#include <math.h>
#include "graph.hpp"
#include "adjacency_graph.hpp"
#include "Eigen/Dense"
using std::cout;
using std::ofstream;

// should converge for all inputs
void writeAlternateDistances(Graph *G, Graph *H)
{
  ofstream fout((*G).getName()+"_vs_"+(*H).getName()+".df");
  AdjGraph s,t;
  s.init(G);
  t.init(H);
  MatrixXd *A = s.matrix(),
           *B = t.matrix(),
           S(*t.size(), *s.size()),
           P(*t.size(), *s.size());
  S.fill(1); P.fill(1);
  bool flag=false;
  for(int i=0;i<100;i++)
  {
      MatrixXd temp = (*B)*S*(*A);
      double tot=0, diff=0;
      for(int r=0;r<S.rows();r++)
        for(int c=0;c<S.cols();c++)
          tot+=temp(r,c)*S(r,c);
      tot = sqrt(tot);
      for(int r=0;r<S.rows();r++)
        for(int c=0;c<S.cols();c++)
          temp(r,c)/=tot;
      S=temp;
      if(i%2==0) 
      {
        for(int r=0;r<S.rows();r++)
          for(int c=0;c<S.cols();c++)
            diff+=fabs(temp(r,c)-P(r,c));
        P=S;
        if(diff<0.1)
          {cout << i << " iterations\n";flag=true; break;}
      }
      cout << "iter: " << i << "\n";
  }
  for(int r=0;r<S.rows();r++)
    for(int c=0;c<S.cols();c++)
    {
      fout << s.stringOf(c) << "\t" << t.stringOf(r) << 
             "\t" << 1.0-S(r,c) << "\n";
    }
  fout.close();
  if(!flag) cout << "didn't converge :(\n";
}

