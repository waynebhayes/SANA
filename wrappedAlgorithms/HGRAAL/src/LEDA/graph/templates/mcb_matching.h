/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  mcb_matching.t
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:45:16 $

#include <LEDA/core/b_stack.h>
#include <LEDA/internal/std/math.h>
#include <LEDA/core/queue.h>

#include <LEDA/system/assert.h>

LEDA_BEGIN_NAMESPACE


template <class graph_t>
class mcb_matching {

typedef typename graph_t::node node;
typedef typename graph_t::edge edge;


int number_of_augmentations;



bool find_aug_path_by_bfs(graph& G, node a, 
                          node_array<bool,graph_t>& free, 
                          node_array<edge,graph_t>& pred,
                          node_array<int,graph_t>&  mark)
{ queue<node> Q; 

  Q.append(a); 
  mark[a] = number_of_augmentations;   


  while ( !Q.empty() )
  { node v = Q.pop();  // v is a node in A
    edge e;
    forall_adj_edges(e,v)
    { node w = G.target(e); // w is a node in B

      if (mark[w] == number_of_augmentations) continue;

      // w has not been reached before in this search

      pred[w] = e; 
      mark[w] = number_of_augmentations;
      if (free[w])
      { // augment path from a to w
        free[w] = free[a] = false;
        while (w != a) 
        { e = pred[w];
          w = G.source(e); 
          G.rev_edge(e); 
        }
        return true;
      }
      // w is not free
      edge f = G.first_adj_edge(w);
      node x = G.target(f);
      pred[x] = f; 
      mark[x] = number_of_augmentations;
      Q.append(x);
    }
  }
  return false;
}

// for the basic algorithm

edge ce(const graph_t& G, node v, 
        const node_array<int,graph_t>& layer, 
        node_array<edge,graph_t>& ce_it)
{ 
  edge e = ce_it[v];
  if ( e == nil ) e = G.first_adj_edge(v);
  while (e && layer[G.target(e)] != layer[v] - 1) e = G.adj_succ(e);
  ce_it[v] = e;
  return e;
 }


list<edge> run(graph& G, const list<node>& A, 
                         const list<node>& B,
                         node_array<bool,graph_t>& NC,
                         bool use_heuristic,
                         int Lmax)
{ node v; edge e;

  //check that all edges are directed from A to B
  forall(v,B) assert(G.outdeg(v) == 0);

  
    node_array<bool,graph_t> free(G,true);
    node_array<int,graph_t> layer(G);

    if (use_heuristic) 
    { node v;
      forall(v,A)
      { edge e;
        forall_adj_edges(e,v)
        { node w = G.target(e);
          if (free[v] && free[w])
          { free[v] = free[w] = false;
            G.rev_edge(e);
          }
        }
      }
    }

    list<node> free_in_A;

    forall(v,B) layer[v] = 0;

    forall(v,A) 
    { layer[v] = 1;
      if (free[v]) free_in_A.append(v);
    }

    int L = 1;


    node_array<edge,graph_t> ce_it(G,nil); // current edge iterator

  if (Lmax == -1) Lmax = (int)(0.1*sqrt((double)G.number_of_nodes()));

  b_stack<edge> p(G.number_of_nodes());

  while ( L <= Lmax && free_in_A.size() > 50 * L)
  { node v = free_in_A.pop();
    L = layer[v];

    
    node w = v;
     
    while (true)
    { if ( free[w] && layer[w] == 0 )
      { // breakthrough
        free[w] = free[v] = false;
        while ( !p.empty() )
        {  e = p.pop(); 
           
           if (e == ce_it[G.source(e)]) 
             ce_it[G.source(e)] = G.adj_succ(e);

           G.rev_edge(e); 
        } 
        break;
      }
      else
      { e = ce(G,w,layer,ce_it);
        if (e)
        { // advance
          p.push(e); 
          w = G.target(e); 
        }
      else
        { // retreat
          layer[w] += 2;
          
          ce_it[w] = nil;
 
          if (p.empty()) 
          { free_in_A.append(w); 
            break; 
          }
          w = G.source(p.pop()); 
        }
      }
    }

  }

  
  node_array<int,graph_t> mark(G,-1);
  node_array<edge,graph_t> pred(G);
  number_of_augmentations = 0;
  forall(v,free_in_A)
  { if ( find_aug_path_by_bfs(G,v,free,pred,mark) ) 
     number_of_augmentations++; 
  }


  
  list<edge> result;

  forall(v,B) 
   forall_adj_edges(e,v) result.append(e);

  forall_nodes(v,G) NC[v] = false;
  node_array<bool,graph_t> reachable(G,false);
  forall(v,A)
    if (free[v]) DFS(G,v,reachable);
  forall(e,result)
    if ( reachable[G.source(e)] )
      NC[G.source(e)] = true;
    else
      NC[G.target(e)] = true;

  forall(e,result) G.rev_edge(e);

  return result;

}
};


LEDA_END_NAMESPACE

