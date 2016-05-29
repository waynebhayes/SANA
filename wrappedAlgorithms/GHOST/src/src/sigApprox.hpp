#include <vector>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "Eigen/Dense"

#define ZERO 1e-2

using std::vector;
using std::cout;
using namespace Eigen;

VectorXd randomVec(int length)
{
  VectorXd v = VectorXd::Random(length);
  v.normalize();
  return v;
};

double normalize(VectorXd *v)
{
  double ratio=(*v).norm();
  (*v).normalize();
  return ratio;
}

double inverseIter(MatrixXd *A, double mu)
{
  int N=(*A).rows(); // rows and cols should be the same
  double prev=200, cur;  // dummy value
  bool flag=false;
  VectorXd seed = randomVec(N);
  for(int i=0;i<N;i++)
    (*A)(i,i) -= mu;
  LDLT<MatrixXd,Lower> ALU = (*A).ldlt();
  for(int i=0;i<1000;i++)
  {
    VectorXd temp = seed;
    seed = ALU.solve(seed);
    cur = normalize(&seed); // in place
    if(fabs(cur-prev) < ZERO) 
    {
      for(int i=0;i<N;i++)
        if(fabs(temp(i)) > ZERO)
        {
          cur*=(temp(i)*seed(i)>0)?1:-1;
          break;
        }
      flag=true;
      break;
    }
    prev=cur;
  }
  for(int i=0;i<N;i++)
    (*A)(i,i) += mu;
//  if(!flag) cout << "warning: did not converge\n"; // uncomment for warning
  cur = 1.0/cur + mu;
  return cur;
}

double rayleigh(MatrixXd *A)
{
  int N=(*A).rows();
  double prev=200, cur;
  bool flag=false;
  VectorXd seed = randomVec(N);
  for(int i=0;i<1000;i++)
  {
    VectorXd mu = seed.transpose()*(*A)*seed;
    cur=mu[0];
    for(int i=0;i<N;i++)
      (*A)(i,i) -= cur;
    seed = (*A).ldlt().solve(seed);
    seed.normalize();
    for(int i=0;i<N;i++)
      (*A)(i,i) += cur;
    if(fabs(cur-prev) < ZERO) {flag=true; break;}
    prev=cur;
  }
//  if(!flag) cout << "warning: did not converge\n"; // uncomment for warning
  return cur;
}

