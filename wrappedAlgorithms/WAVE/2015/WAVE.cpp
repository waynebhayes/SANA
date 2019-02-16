#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <time.h>
#include <ctime>
#include <cmath>
using namespace std;

#define new_index (1-now_index)
#define equal(a,b) (abs(a-b)<1e-5)
//#define alpha 0.5
#define beta (1-alpha)
#define M 8000
#define inf 2147483647.0

//double r[2][M][M], tsim[M][M], sim[M][M], maxsim = 0.0, dd[M][M];
double **tsim, **sim, maxsim = 0.0, **dd;
//int nei[2][M][M];
int** nei[2];
int d[2][M];
int totnode[2], totedge[2];
int now_index, x, y;
int match[M];
bool sx[M],sy[M];
//bool g[2][M][M];
bool **g[2];
double alpha;

void readdata(char* filename, int ind)
{
    freopen(filename, "r", stdin);
    cin >> totnode[ind] >> totedge[ind];
    for (int i = 0; i < totedge[ind]; i++)
    {
        cin >> x >> y;
        nei[ind][x][d[ind][x]] = y;
        d[ind][x]++;
        nei[ind][y][d[ind][y]] = x;
        d[ind][y]++;
        g[ind][x][y] = true;
        g[ind][y][x] = true;
    }
    fclose(stdin);
}

void readsimi(char* filename)
{
    freopen(filename, "r", stdin);
    for (int i = 0; i < totnode[0]; i++)
        for (int j = 0; j < totnode[1]; j++) {
            cin >> x >> y;
            cin >> tsim[x][y];
            //if (tsim[x][y]>maxsim) maxsim = tsim[x][y];
        }
    for (int i = 0; i < totnode[0]; i++)
        for (int j = 0; j < totnode[1]; j++) {
            sim[i][j] = tsim[i][j];
            //sim[i][j] = 0.0;
            dd[i][j] = 1;
            //dd[i][j] = 0;
        }
            
    fclose(stdin);
}

int main(int argc, char** argv)
{    
    time_t now_time = time(NULL);
    for (int i = 0; i < 2; i++)
    {
        //r[i] = new double*[M];
        nei[i] = new int*[M];
        g[i] = new bool*[M];
    }
    tsim = new double*[M];
    sim = new double*[M];
    dd = new double*[M];
    for (int i = 0; i < M; i++) 
    {
        for (int j = 0; j < 2; j++)
        {
                //r[j][i] = new double[M];
                nei[j][i] = new int[M];
                g[j][i] = new bool[M];
        }
        tsim[i] = new double[M];
        sim[i] = new double[M];
        dd[i] = new double[M];
    }
    memset(d, 0, sizeof(d));
    //memset(nei, 0, sizeof(nei));
//    memset(g[0], false, M*M*sizeof(bool));
//    memset(g[1], false, M*M*sizeof(bool));
    for (int i = 0; i < M; i++)
        for (int j = 0; j < M; j++) {
            g[0][i][j] = false;
            g[1][i][j] = false;
        }
    memset(sx, false, sizeof(sx));
    memset(sy, false, sizeof(sy));
    //cout << "here!" << endl;
    readdata(argv[1], 0);
    readdata(argv[2], 1);
    readsimi(argv[3]);
    //alpha = atof(argv[4]); 
    for (int i = 0; i < totnode[1]; i++) match[i] = -1;
    
    //cout << totnode[0] << " " << totnode[1] << endl;
    
    //getR();
    int maxd = 0;
    /*for (int i = 0; i < totnode[0]; i++)
        if (d[0][i]>maxd) maxd = d[0][i];
        */
    //cout << maxd << endl;
    freopen(argv[4], "w", stdout);
    
    
    for (int i = 0; i < totnode[0]; i++)
    {
        int maxx, maxy;
        double maxs = -1.0;
        //cout << i << endl;
        for (int x = 0; x < totnode[0]; x++)
            for (int y = 0; y < totnode[1]; y++)
            {
                //cout << x << " " << y << endl;
                if (sx[x] || sy[y]) continue;
                if (sim[x][y]>maxs) {
                    maxx = x; maxy = y;
                    maxs = sim[x][y];
                }
            }
        //sim[maxx][maxy] = 0.0;
        match[maxy] = maxx;
        printf("%d %d\n", maxx, maxy);
        //cout << "(" << maxx << " " << maxy << ")\t";
        sx[maxx] = true; sy[maxy] = true;
        for (int j = 0; j < d[0][maxx]; j++)
        {
            int x = nei[0][maxx][j];
            //if (sx[x]) continue;
            for (int k = 0; k < d[1][maxy]; k++)
            {
                int y = nei[1][maxy][k];
                //if (sy[y]) continue;
                //sim[x][y] = alpha*(((++dd[x][y])+0.0)/(d[0][x]+0.0))+beta*tsim[x][y]/maxsim;
                sim[x][y] += (tsim[maxx][maxy]+tsim[x][y]);
                //sim[x][y] += (tsim[maxx][maxy]);
                //sim[x][y] += (tsim[maxx][maxy]/getd(x, y)+tsim[x][y]/getd(maxx, maxy));
            }
        }
    }
/*
    int rightnode = 0, rightedge = 0;
    for (int i = 0; i < totnode[1]; i++)
    {
        //if (match[i]==i) rightnode++;
        if (match[i] == -1) continue;
        cout << match[i] << " " << i << endl;
        
        for (int j = 0; j < d[1][i]; j++)
        {
            int y = nei[1][i][j];
            if (match[y]==-1) continue;
            if (g[0][match[i]][match[y]]) rightedge++;
        }
    }
*/
    fclose(stdout);
    
    for (int i = 0; i < M; i++) 
    {
        for (int j = 0; j < 2; j++)
        {
                //delete[] r[j][i];
                delete[] nei[j][i];
                delete[] g[j][i];
        }
        delete[] tsim[i];
        delete[] sim[i];
        //delete[] dd[i];
    }
    
    for (int i = 0; i < 2; i++)
    {
        //delete[] r[i];
        delete[] nei[i];
        delete[] g[i];
    }
    delete[] tsim;
    delete[] sim;
    //delete[] dd;
    time_t end_time = time(NULL);
    //cout << "Takes time: " << end_time - now_time << endl;

    return 0;
}