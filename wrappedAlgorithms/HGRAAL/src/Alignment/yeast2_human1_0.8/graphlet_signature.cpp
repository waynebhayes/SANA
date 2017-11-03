#if 1
#include <iostream>
#include "../../LEDA/core/string.h"
#include "../../LEDA/core/dictionary.h"
#include "../../LEDA/core/array.h"
#include "../../LEDA/core/set.h"
#include "../../LEDA/core/list.h"
#include "../../LEDA/core/p_queue.h"
#include "../../LEDA/graph/graph.h"
#include "../../LEDA/graph/basic_graph_alg.h"
#include "../../LEDA/graph/ugraph.h"
#include "../../LEDA/graph/graph_alg.h"
#include "../../LEDA/graph/node_array.h"
#include <cstdlib>
#include <string>
#include <sstream>
#include "../../Hungarian/matrix.h"


template <class T>
std::string to_string(T t)
{
  std::ostringstream oss;
  oss << t;
  return oss.str();
}

using namespace leda;

struct pair
{
    long n1;
    long n2;
};

class Aligner
{
    private: 
        UGRAPH<string, long> _g1; //first graph
        UGRAPH<string, long> _g2; //second graph
        double alpha;        
        p_queue<double, pair> pairs; //pair of nodes with their alignment cost
        string *nodeList1; //node list from graph 1
        string *nodeList2; //node list from graph 2
        bool *alignedNodesList1; //which nodes from graph 1 are already aligned
        bool *alignedNodesList2; //which nodes from graph 2 are already aligned
        void calculate_costs_from_gdd(string filename1, string filename2); // calculates costs from gdd
        dictionary<long,long> alignment;
        UGRAPH<string,long> power(UGRAPH<string,long> *g,int p); //Returns copy of g raised to the power p
        array< set<long> > build_spheres(long nd, UGRAPH<string,long> *g);
        pair get_new_seed();
        void align_spheres(set<long> *sp1, set<long> *sp2); //aligns spheres
    public:
        UGRAPH<string,long> g1; //copy of first graph with numbers instead of node names
        UGRAPH<string,long> g2; //copy of second graph with numbers instead of node names
        Matrix<double> Costs; //Costs of aligning nodes
        //public methods
        Aligner(string G1filename, string G2filename, string GDDfile1, string GDDfile2, double al);
        ~Aligner();
        void align();//alignes graphs
        void save_results(string filename); //saves results
};

/////////////////METHODS IMPLEMENTATION///////////////////////////////////////////////////////////////////////

void Aligner::save_results(string filename)
{
    try
    {
        long done_m=0;
        double percentage=0;
        long total_work =this->g1.number_of_edges()*this->g2.number_of_edges();
        
        std::cout<<"Calculating statistics and saving results."<<std::endl;
        std::cout<<"Please wait ..."<<std::endl;
        
        std::ofstream res_file; //File with general info
        std::ofstream al_file; //File with node alignment
        std::ofstream e_al_file; //File with edge alignment
        std::ofstream n_e_al_file; //File with node alignment part which is in edge alignment
        
        string res_filename = filename+".results";
        string al_filename = filename+".aln";
        string e_al_filename = filename+".ealn";
        string n_e_al_filename = filename+".nealn";
        
        res_file.open(res_filename);
        res_file<<"Graph1 has: "<<this->g1.number_of_nodes()<<" nodes."<<std::endl;
        res_file<<"Graph2 has: "<<this->g2.number_of_nodes()<<" nodes."<<std::endl;
        res_file<<std::endl;        
        res_file<<"Graph1 has: "<<this->g1.number_of_edges()<<" edges."<<std::endl;
        res_file<<"Graph2 has: "<<this->g2.number_of_edges()<<" edges."<<std::endl;
        
        
        al_file.open(al_filename);
        e_al_file.open(e_al_filename);
        n_e_al_file.open(n_e_al_filename);
        
        long NA=0; //node alignment
        dic_item it;
        forall_items(it,this->alignment)
        {
            long n1 = this->alignment.key(it); //node in first graph
            long n2 = this->alignment.inf(it); //node in second graph
            
            string sn1=this->nodeList1[n1]; //node name
            string sn2=this->nodeList2[n2]; //node name
            if (sn1==sn2) //names match -> increase NA
                NA++;
            al_file<<sn1<<" "<<sn2<<std::endl; 
        }
        al_file.close();
        res_file<<"Node Correctness is: "<<NA;
        double percent = 100*((double)NA)/((double)this->g1.number_of_nodes());
        res_file<<"; which is "<<percent<<"%"<<" of nodes"<<std::endl;
        
        long EA=0; //edge alignment;
        long IA=0; //interaction alignment;
        
        long counter=0;//this is used to display progress
        
        bool *freeNodes = new bool[this->_g1.number_of_nodes()]; //for node subalignment
        for (long y=0;y<this->_g1.number_of_nodes();y++)
            freeNodes[y]=true;
        
        edge e;
        forall_edges(e,this->g1)
        {
            string sG1n1=this->g1.get_node_entry_string(this->g1.target(e));
            string sG1n2=this->g1.get_node_entry_string(this->g1.source(e));
            long lG1n1=atol(sG1n1.cstring());
            long lG1n2=atol(sG1n2.cstring());
            
            edge j;
            forall_edges(j,this->g2)
            {
                string sG2n1=this->g2.get_node_entry_string(this->g2.target(j));
                string sG2n2=this->g2.get_node_entry_string(this->g2.source(j));
                long lG2n1=atol(sG2n1.cstring());
                long lG2n2=atol(sG2n2.cstring());
                
                long im1=this->alignment.access(lG1n1); // to what node lG1n1 was mapped
                long im2=this->alignment.access(lG1n2); // to what node lG1n2 was mapped
                if (((im1==lG2n1)&&(im2==lG2n2))||((im1==lG2n2)&&(im2==lG2n1))) //edge e mapped to edge j
                {
                    EA++;
                    e_al_file<<this->nodeList1[lG1n1]<<" "<<this->nodeList1[lG1n2]<<" "<<this->nodeList2[lG2n1]<<" "<<this->nodeList2[lG2n2]<<std::endl;
                    
                    if (freeNodes[lG1n1])
                    {
                        n_e_al_file<<this->nodeList1[lG1n1]<<" "<<this->nodeList2[this->alignment.access(lG1n1)]<<std::endl;
                        freeNodes[lG1n1]=false;
                    }
                    if (freeNodes[lG1n2])
                    {
                        n_e_al_file<<this->nodeList1[lG1n2]<<" "<<this->nodeList2[this->alignment.access(lG1n2)]<<std::endl;
                        freeNodes[lG1n2]=false;
                    }
                    
                    string u1=this->nodeList1[lG1n1];
                    string u2=this->nodeList1[lG1n2];
                    
                    string fu1=this->nodeList2[im1];
                    string fu2=this->nodeList2[im2];
                    
                    if ((u1==fu1)&&(u2==fu2)) //is it matched interaction?
                        IA++;                    
                }                
                done_m++;                
            }
            counter++;
            if (counter%40==0)
            {
                percentage=100*((double)done_m)/((double)total_work);
                std::cout<<"Statistics readiness: "<<percentage<<"%"<<std::endl;
            }
        }
        e_al_file.close();
        n_e_al_file.close();
        
        res_file<<"Edge Correctness is: "<<EA;
        percent = 100*((double)EA)/((double)this->g1.number_of_edges());
        res_file<<"; which is "<<percent<<"%"<<" of edges"<<std::endl;
        
        res_file<<"Interaction Correctness is: "<<IA;
        percent = 100*((double)IA)/((double)this->g1.number_of_edges());
        res_file<<"; which is "<<percent<<"%"<<" of edges"<<std::endl;
        
        res_file.close();
        std::cout<<"Done."<<std::endl;
        std::cout<<"Results saved to files:"<<std::endl;
        std::cout<<res_filename<<": file with statistics about EA(edge alignment), NA(node alignment) and IA(interaction alignment)"<<std::endl;
        std::cout<<al_filename<<": file with produced global alignment"<<std::endl; 
        std::cout<<e_al_filename<<": file with produced edge alignment"<<std::endl;
        std::cout<<n_e_al_filename<<": file with part of global node alignment which is present in edge alignmnet"<<std::endl;
        delete []freeNodes;
    }
    catch(...)
    {
        throw "Aligner: Can't save results to file";        
    }    
}

Aligner::Aligner(string G1filename, string G2filename, string GDDfile1, string GDDfile2, double al)
{
    if (al>=0)
    {
        this->alpha = al;        
    }
    else throw "Aligner: Alpha can't be negative";    
    try
    {
        this->_g1.read(G1filename);
        this->_g2.read(G2filename);
        this->alignedNodesList1 = new bool[this->_g1.number_of_nodes()];
        this->alignedNodesList2 = new bool[this->_g2.number_of_nodes()];
        
        for (long i=0;i<this->_g1.number_of_nodes();i++)
            this->alignedNodesList1[i]=false;
        for (long i=0;i<this->_g2.number_of_nodes();i++)
            this->alignedNodesList2[i]=false;
        this->calculate_costs_from_gdd(GDDfile1,GDDfile2);
    }
    catch (...)
    {
        throw "Aligner: Error reading files";
    }    
};

Aligner::~Aligner()
{
    delete [] this->nodeList1;
    delete [] this->nodeList2;
    delete [] this->alignedNodesList1;
    delete [] this->alignedNodesList2;
    //this->g1.~UGRAPH();
    //this->g2.~UGRAPH();
    //this->_g1.~UGRAPH();
    //this->_g2.~UGRAPH();
}

void Aligner::calculate_costs_from_gdd(string filename1, string filename2)
{
    double weight_factor [73] = {1, 2, 2, 2, 3, 4, 3, 3, 4, 3, 4, 4, 4, 4, 3, 4, 6, 5, 4, 5, 6, 6, 4, 4, 4, 5, 7, 4, 6, 6, 7, 4, 6, 6, 6, 5, 6, 7, 7, 5, 7, 6, 7, 6, 5, 5, 6, 8, 7, 6, 6, 8, 6, 9, 5, 6, 4, 6, 6, 7, 8, 6, 6, 8, 7, 6, 7, 7, 8, 5, 6, 6, 4};
    double weight[73];
    
    long done_m=0;
    double percentage=0;
    long total_work = this->_g1.number_of_nodes()*this->_g2.number_of_nodes();
    
    std::ifstream file1;
    std::ifstream file2;
    file1.open(filename1);
    file2.open(filename2);
    if (!file1.is_open())
        throw "Aligner: Can't open first GDD file";
    if (!file2.is_open())
        throw "Aligner: Can't open second GDD file";    
    
    leda::string tmp;
    leda::string name;
    leda::array<unsigned long> ar(73); //GDD vector
    leda::dictionary<leda::string,leda::array<unsigned long> > dic1; // dictionary <node name, GDD vector>
    leda::dictionary<leda::string,leda::array<unsigned long> > dic2; // dictionary <node name, GDD vector>
    
    int k=0;
    try //this block tries to read first GDD file
    {
        while (!file1.eof())
        {
            if (k==0)
            {
                file1>>name;
                k++;
            }
            else if (k<73)
            {
                file1>>tmp;
                ar[k-1]=atol(tmp);
                k++;
            }
            else if (k==73)
            {
                file1>>tmp;
                ar[k-1]=atol(tmp);
                dic1.insert(name,ar);
                k=0;
            }
            
        }
        if (file1.is_open()) file1.close();
    }
    catch(...) //If something bad happend
    {
        if (file1.is_open()) file1.close();
        throw "Aligner: Error reading GDD file1";
    }
    
    k=0;
    try //this block tries to read second GDD file
    {
        while (!file2.eof())
        {
            if (k==0)
            {
                file2>>name;
                k++;
            }
            else if (k<73)
            {
                file2>>tmp;
                ar[k-1]=atol(tmp);
                k++;
            }
            else if (k==73)
            {
                file2>>tmp;
                ar[k-1]=atol(tmp);
                dic2.insert(name,ar);
                k=0;
            }        
        }
        if (file2.is_open()) file2.close();
    }
    catch(...) //If something bad happend
    {
        if (file2.is_open()) file2.close();
        throw "Aliger: Error reading GDD file2";
    }
        
    this->Costs = Matrix<double>(dic1.size(),dic2.size());
                
    double sum_coef=1;
    int num_orbits= 73;
        
    for(int i = 0; i < 72; i++)
    {    
            weight [i] = 1 - log10(weight_factor [i])/log10(num_orbits); 
            sum_coef += weight [i];                            
    }
    
    ////////////////////
    double md1=1;//maximal degree of the graph1
    double md2=1;//maximal degree of the graph2
    
    this->g1 = UGRAPH<string,long>(this->_g1);
    this->g2 = UGRAPH<string,long>(this->_g2);
    this->nodeList1 = new string[this->g1.number_of_nodes()];
    this->nodeList2 = new string[this->g2.number_of_nodes()];
    
    long c=0;
    node n;
    forall_nodes(n,this->g1)
    {
        if (this->g1.degree(n)>md1)
            md1=this->g1.degree(n);
        this->nodeList1[c]=this->g1.get_node_entry_string(n);
        string s(to_string(c).data());
        this->g1.set_node_entry(n,s);
        c++;
    }
    c=0;
    forall_nodes(n,this->g2)
    {
        if (this->g2.degree(n)>md2)
            md2=this->g2.degree(n);
        this->nodeList2[c]=this->g2.get_node_entry_string(n);
        string s(to_string(c).data());
        this->g2.set_node_entry(n,s);
        c++;
    }
    ////////////////////
    
    int counter=0; //this is used to display progress only
    node n1;
    node n2;
    forall_nodes(n1,this->g1)
    {
            forall_nodes(n2,this->g2)
            {
                long i=atol(this->g1.get_node_entry_string(n1));
                long j=atol(this->g2.get_node_entry_string(n2));
                
                leda::array<unsigned long> ar1 = dic1.access(this->nodeList1[i]);
                leda::array<unsigned long> ar2 = dic2.access(this->nodeList2[j]);
                double max= -1;
                for (int q=0;q<73;q++)
                {
                    if (ar1[q]>max)
                        max=ar1[q];
                    if (ar2[q]>max)
                        max=ar2[q];
                }
                    
                double res=0;
                for (int q=0;q<73;q++)
                {
                    res+=weight[q]*(fabs(log10(ar1[q]+1)-log10(ar2[q]+1)))/log10(max+2);                
                }
                res=res/sum_coef; //AFTER THIS res is Tijana's distance between nodes.
                ///PUT cost into the matrix
                double score = (1-this->alpha)*(ar1[0]+ar2[0])/(md1+md2)+(this->alpha)*(1-res);
                pair p;
                p.n1=i;
                p.n2=j;
                this->pairs.insert(2-score,p); // Add pair to priority queue
                this->Costs.set(i,j,2-score);// I used 2 to be sure that I won't get negative values.
                //Cost is in the matrix
                
                done_m++;                                
            }
            counter++;
            if (counter%20==0)
            {
                percentage=100*((double)done_m)/((double)total_work);
                std::cout<<"Cost matrix readiness: "<<percentage<<"%"<<std::endl;
            }
    }
    std::cout<<"Done making cost matrix!"<<std::endl;        
}

UGRAPH<string,long> Aligner::power(UGRAPH<string,long> *g,int p)
{
    UGRAPH<string,long> res = *g;
    node n1;
    node n2;    
    forall_nodes(n1,res)
    {
        node_array<int> dist(res,-1);
        BFS(*g,n1,dist);
        forall_nodes(n2,res)
        {
            if ((dist[n2]>1)&&(dist[n2]<=p))
                res.new_edge(n1,n2);
        }
    }
    return res;
}

array< set<long> > Aligner::build_spheres(long nd, UGRAPH<string,long> *g)
{
    array< set<long> > res;
    node_array<int> d(*g,-1);
    node n;
    node _n;
    forall_nodes(n,*g)
    {
        if (atol(g->get_node_entry_string(n).cstring())==nd)
            {
                _n=n;
                break;
            }
    }    
    
    BFS(*g,_n,d);
    node n1;
    int radius=-1;
    forall_nodes(n1,*g)
    {
        if (d[n1]>radius)
            radius=d[n1];
    }
    if (radius<=0)
        return res;
    else
    {
        res.resize(radius);
        forall_nodes(n1,*g)
        {
            if (d[n1]>0)
                res[d[n1]-1].insert(atol(g->get_node_entry_string(n1).cstring()));
        }        
    }    
    return res;    
}

void Aligner::align_spheres(set<long> *sp1, set<long> *sp2)
{
    long nd1;
    long nd2;
    list<pair> candidates;
        
    while ((!sp1->empty())&&(!sp2->empty()))
    {
        candidates.clear();
        pair p;
        pair temp_p;
        p.n1=-1;
        p.n2=-1;
        double score=10000000;
        forall(nd1, *sp1)
        {
            if (!this->alignedNodesList1[nd1])
            {
                forall(nd2, *sp2)
                {
                    if (!this->alignedNodesList2[nd2])
                    {
                        if (this->Costs.get(nd1,nd2)<score)
                        {
                            temp_p.n1 = nd1;
                            temp_p.n2 = nd2;
                            score=this->Costs.get(nd1,nd2);
                            candidates.clear();
                            candidates.push(temp_p);
                        }
                        else if (this->Costs.get(nd1,nd2)==score)
                        {
                            temp_p.n1 = nd1; 
                            temp_p.n2 = nd2; 
                            candidates.push(temp_p);
                        }
                    }
                    else
                    {
                        sp2->del(nd2);
                    }
                }
            }
            else
            {
                sp1->del(nd1);
            }
        }
        
        if (!candidates.empty())
        {
            if (candidates.size()==1)
                p=candidates.pop();
            else
            {
                int k = rand() % candidates.size();
                int i=0;
                int sz=candidates.size();
                while (i<sz)
                {
                    pair p1 = candidates.pop();
                    if (i==k)
                        p=p1;
                    i++;
                }
            }
        }
        if ((p.n1!=-1)&&(p.n2!=-1))
        {
            this->alignment.insert(p.n1,p.n2);
            sp1->del(p.n1);
            sp1->del(p.n2);
            this->alignedNodesList1[p.n1]=true;
            this->alignedNodesList2[p.n2]=true;
            double proc =100*((double)this->alignment.size())/((double)this->_g1.number_of_nodes());
            std::cout<<"Alignement readiness: "<<proc<<"%"<<std::endl;        
        }        
    }
}

pair Aligner::get_new_seed()
{
    if (this->pairs.size()>0)
    {
        list<pair> candidates;
        candidates.clear();
        while (candidates.size()==0)
        {
            pq_item it = this->pairs.find_min();
            double score=this->pairs.prio(it);
            
            pair cur=this->pairs[it];
            if ((!this->alignedNodesList1[cur.n1])&&(!this->alignedNodesList2[cur.n2]))
                candidates.push(cur);
            this->pairs.del_min();
            bool search=true;
            while (search)
            {
                if (pairs.size()==0)
                    break;
                it = this->pairs.find_min();
                double s=this->pairs.prio(it);
                if (s==score)
                {
                    pair cur2=this->pairs[it];
                    if ((!this->alignedNodesList1[cur2.n1])&&(!this->alignedNodesList2[cur2.n2]))
                        candidates.push(cur2);
                    this->pairs.del_min();
                }
                else
                    search=false;
            }            
        }
        
        if (candidates.size()==1)
        {
            pair p = candidates.pop();
            return p;
        }            
        else
        {
            int k = rand() % candidates.size();
            int i=0;
            pair p;
            int sz=candidates.size();
            while (i<sz)
            {
                pair p1 = candidates.pop();
                if (i==k)
                    p=p1;
                else
                    this->pairs.insert(this->Costs.get(p1.n1,p1.n2),p1);
                i++;
            }
            return p;
        }        
    }        
}

void Aligner::align()
{
    UGRAPH<string,long> G1=this->g1;
    UGRAPH<string,long> G2=this->g2;
    int power=1;
    bool do_power=true;
    while (this->alignment.size()<this->_g1.number_of_nodes())
    {
        pair seed = this->get_new_seed();            
        this->alignment.insert(seed.n1,seed.n2);
        this->alignedNodesList1[seed.n1]=true;
        this->alignedNodesList2[seed.n2]=true;
        double proc =100*((double)this->alignment.size())/((double)this->_g1.number_of_nodes());
        std::cout<<"Alignment readiness: "<<proc<<"%"<<std::endl;
        
        array< set<long> > ar1 = this->build_spheres(seed.n1,&G1);
        array< set<long> > ar2 = this->build_spheres(seed.n2,&G2);
        
        int i=0;
        bool doiter=false;
        if ((ar1.size()>0)&&(ar2.size()>0))
                doiter=true;
        while (doiter)
        {
            if ((i<ar1.size())&&(i<ar2.size()))
            {
                set<long> s1 = ar1[i];
                set<long> s2 = ar2[i];
                this->align_spheres(&s1,&s2);                            
            }
            else
                doiter=false;            
            i++;
        }
        if ((i<1)||(power>=3))
            do_power=false;
        if (do_power)
        {
            power++;
            G1=this->power(&(this->g1),power);
            G2=this->power(&(this->g2),power);
        }
        do_power=true;
    }
}

//////////////////END OF ALIGNER CLASS/////////////////////////////////////////////

int main(int argc, char *argv[])
{
    try
        {
            if (argc!=7)
            {
                std::cout<<"USAGE: ./Aligner3 alpha graph1.gw graph2.gw gdd_file1 gdd_file2 results_filename"<<std::endl;
                std::cout<<"Program accepts only graphs in LEDA .gw format."<<std::endl;
                std::cout<<"First graph must have less then or equal to number of nodes as second graph"<<std::endl;
                std::cout<<"gdd_file1 and gdd_file2 are files wich contain for each node it's gdd vector, these files are produced by ncount program"<<std::endl;
            }
            else
            {
                std::cout<<"Aligner started!"<<std::endl;
                double a=atof(argv[1]);
                Aligner *al = new Aligner(argv[2],argv[3],argv[4],argv[5],a);
                al->align();
                al->save_results(argv[6]);
                delete al;
            }
        }
        catch(const char *msg)
        {
                std::cout<<msg;
        }    
}
#endif
