#include "SkipList.hpp"

using get_time = std::chrono::steady_clock ;

SkipNode::SkipNode(){
  height = MAX_LEVEL;
  key = 0;
  value = std::make_pair(0,0);
  for(int i = 0; i < MAX_LEVEL; ++i){
    forward[i] = NULL;
  }
}

SkipNode::SkipNode(int h, float k, std::pair<uint,uint> v)
  :height(h), key(k), value(v){
  for(int i = 0; i < MAX_LEVEL; ++i){
    forward[i] = NULL;
  }
}

SkipNode::~SkipNode(){
}

/*
  limit: number of levels to traverse to prevent infinite loops 
  suggested maximum value: 16
*/
void SkipNode::debug(int limit){
  if(limit > 0){
    std::cout << "(Node) key: " << key 
          << " value: (" << value.first << ", " 
          << value.second << ") " << std::endl;
    std::cout << "next_ptrs " << std::endl;
    for(int i = 0; i < MAX_LEVEL; ++i){
      if(forward[i] == NULL){
    //std::cout << i << ": Null" << std::endl;
      }else{
    std::cout << i << ": " 
          << "key: " << forward[i]->key
          << " value: (" << forward[i]->value.first << ", " 
          << forward[i]->value.second << ") " << std::endl;
      }
    }
    for(int i = 0; i < MAX_LEVEL; ++i){
      if(forward[i] != NULL){
    forward[i]->debug(limit-1);
      }
    }
  }
}

SkipList::SkipList(float delta, bool setMaxHeap, std::unordered_set<uint> & lex, std::unordered_set<uint> & rex)
  :_isMaxHeap(setMaxHeap), length(0), level(0), delta(delta + EPSILON),
   left_exclude(lex), right_exclude(rex)
{
  rng.seed(getRandomSeed());
  this->debug();
  miss_counter = 0;
  insert_time -= insert_time;
  cleanup_time -= cleanup_time;
  pop_time -= pop_time;
  cumulative_miss = 0;
  for(int i = 0; i < 3000; ++i){
    std::uniform_int_distribution<int> rnd(0,i);
    uni.push_back(rnd);
  }
}

SkipList::~SkipList(){
  SkipNode * prev = NULL;
  SkipNode * curr = this->head.forward[0];
  while(curr){
    prev = curr;
    curr = curr->forward[0];
    delete prev;
  }
}

int SkipList::random_height(void){
  int level = 1;
  std::uniform_real_distribution<float> uni(0, 1.0);
  while (uni(rng) < SkipList::SKIPLIST_P && level < SkipNode::MAX_LEVEL)
    level += 1;
  return (level<SkipNode::MAX_LEVEL) ? level : SkipNode::MAX_LEVEL;
}

void SkipList::insert(float similarity, std::pair<uint,uint> entry){
  auto start = std::chrono::steady_clock::now();
  similarity = _isMaxHeap ? (1 - similarity) : similarity;
    
  SkipNode * curr;
  SkipNode * update[SkipNode::MAX_LEVEL];
  for(int i = 0; i < SkipNode::MAX_LEVEL; ++i){
    update[i] = NULL;
  }

  curr = &(this->head);
  for(int i = this->level - 1; i >= 0; --i){
    while(curr->forward[i] && (curr->forward[i]->key < similarity)){
      curr = curr->forward[i];
    }
    update[i] = curr;
  }

  int height = random_height();
  if(height > this->level){
    for(int i = this->level; i < height; ++i){
      update[i] = &this->head;
    }
    this->level = height;
  }
  
  SkipNode * node = new SkipNode(height, similarity, entry);
  //insert new node into the SkipList
  for(int i = 0; i < height; ++i){
    node->forward[i] = update[i]->forward[i];
    update[i]->forward[i] = node;
  }
  this->length += 1;
  this->debug();
  auto end = std::chrono::steady_clock::now();
  insert_time += std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
}
    
bool SkipList::empty(){
  if(this->length < 0){
    std::cout << "Error:fewer than 0 items in list!" << std::endl;
    //could throw an exception if needed
  }
  return this->length <= 0;
}

/*
 * removes a node by reference 
 * scan each level for the pointer to the given node
 * skip over that node and descend to the next level
 * shorten the search by restarting at the last node
 * where the pointer was found 
 */
void SkipList::removeNode(SkipNode * n){
  if(!n){
    std::cout << "tried removing null" << std::endl;
    return;
  }
  //std::cout << "remove node " << this->length << std::endl;
  bool found = false;
  SkipNode * curr = &(this->head);
  
  SkipNode * start = curr;
  for(int i = this->level - 1; i >= 0; --i){
    curr = start;
    while(curr && (curr->key <= n->key)){
      if(curr->forward[i] == n){
    found = true;
    curr->forward[i] = n->forward[i];
    start = curr;
      }else{
    curr = curr->forward[i];
      }
    }
  }
  
  /*
    while(curr && (curr->key <= n->key)){
    for(int i = curr->height; i >= 0; --i){
    if(curr->forward[i] == n){
    curr->forward[i] = n->forward[i];
    }
    }
    curr = curr->forward[0];
    }
  */
  if(found){
    delete n;
    this->length -= 1;
  }
}

uint SkipList::random_int(uint n){
  if(n < uni.size()){
    return uni[n-1](rng);
  }else{
    std::uniform_int_distribution<int> rnd(0,n);
    uni.push_back(rnd);
    return rnd(rng);
  }
}

/*
 * this function cannot be recursive due to stack overflow 
 * for seeds > 2300
 */
std::pair<uint,uint> SkipList::pop_reservoir(){
  auto start = std::chrono::steady_clock::now();
  std::pair<uint,uint> result;
  uint miss_counter = 0;

  if(this->empty()){
    throw QueueEmptyException();
  }
  SkipNode * choice = this->head.forward[0];
  SkipNode * curr = choice;
  float tail = curr->key + this->delta;
  partial_cleanup(tail);
  
  while(left_exclude.find(result.first) != left_exclude.end() ||
    right_exclude.find(result.second) != right_exclude.end()){
    ++miss_counter;
    if(miss_counter > SkipList::CLEANUP_THRESH){
      this->cleanup();
      cumulative_miss += miss_counter;
      miss_counter = 0;
    }
    if(this->empty()){
      throw QueueEmptyException();
    }
    //std::cout << "pop uniform" << std::endl;
    choice = this->head.forward[0];
    curr = choice;
    tail = curr->key + this->delta;
    
    uint n = 0;
    while(curr && curr->key <= tail){
      if(random_int(++n) == 0){
    choice = curr;
      }
      curr = curr->forward[0];
    }

    result = choice->value;
    removeNode(choice);
  }
  --miss_counter;
  cumulative_miss += miss_counter;
  this->debug();
  auto end = std::chrono::steady_clock::now();
  pop_time += std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  return result;

/*
  std::pair<uint,uint> result;
  uint miss_counter = 0;
  do{
    if(this->empty()){
      throw QueueEmptyException();
    }
    //std::cout << "pop uniform" << std::endl;
    SkipNode * choice = this->head.forward[0];
    SkipNode * curr = choice;
    float tail = curr->key + this->delta;
    
    uint n = 0;
    while(curr && curr->key <= tail){
      if(random_int(++n) == 0){
    choice = curr;
      }
      curr = curr->forward[0];
    }

    result = choice->value;
    removeNode(choice);
    ++miss_counter;
    if(miss_counter > SkipList::CLEANUP_THRESH){
      this->cleanup();
      cumulative_miss += miss_counter;
      miss_counter = 0;
    }
  }while(left_exclude.find(result.first) != left_exclude.end() ||
     right_exclude.find(result.second) != right_exclude.end());
  cumulative_miss += miss_counter;
  this->debug();
  auto end = std::chrono::steady_clock::now();
  pop_time += std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  return result;
*/
}

std::pair<uint,uint> SkipList::pop_distr(){
  std::cout << "pop distr" << this->length << std::endl;
  std::pair<uint,uint> result;
  SkipNode * curr;
  SkipNode * choice = this->head.forward[0];
  do{
    if(this->empty()){
      throw QueueEmptyException();
    }
    curr = this->head.forward[0];
    float start = this->head.forward[0]->key;
    std::uniform_real_distribution<float> uni(start,start+this->delta);
    float pick_value = uni(rng);
    for(int i = this->level - 1; i >= 0; --i){
      while(curr->forward[i] && (curr->forward[i]->key <= pick_value)){
    curr = curr->forward[i];
      }
    }
    if(!curr->forward[0] || curr->forward[0]->key > start+this->delta){
      choice = curr;
    }else{ //resevoir sample
      int n = 0;
      while(curr && curr->key <= start+this->delta){
    if(random_int(++n) == 0){
      choice = curr;
    }
    curr = curr->forward[0];
      }
    }

    result = choice->value;
    removeNode(choice);
    ++miss_counter; //count misses in a row
    if(miss_counter > SkipList::CLEANUP_THRESH){
      this->cleanup();
    }
  }while(left_exclude.find(result.first) != left_exclude.end() ||
     right_exclude.find(result.second) != right_exclude.end());
  miss_counter = 0;
  return result;
  /*


  SkipNode * choice = NULL;
  do{
    if(this->empty()){
      throw QueueEmptyException();
    }
    
    if(this->length == 1){
      result = this->head.forward[0]->value;
      removeNode(this->head.forward[0]);
      return result;
    }else{
      choice = this->head.forward[0];
      float start = this->head.forward[0]->key;
      std::uniform_real_distribution<float> uni(start,start+this->delta);
      float pick_value = uni(rng);
      for(int i = this->level - 1; i >= 0; --i){
    while(curr->forward[i] && (curr->forward[i]->key <= pick_value)){
      curr = curr->forward[i];
      if(curr->key <= pick_value){
        choice = curr;


          result = choice->value;
          std::cout << "result (" << result.first << ", " << result.second << ")" << std::endl;
          removeNode(curr);

          if(left_exclude.find(result.first) == left_exclude.end() ||
          right_exclude.find(result.second) == right_exclude.end()){
          return result;
          }
      }
    }
      }
      curr = choice->forward[0];
      //if(!curr || curr->key > pick_value){
      result = choice->value;      
      //}else{
      //resevoir sample
      
      //}
      ++miss_counter; //count misses in a row
      if(miss_counter > SkipList::CLEANUP_THRESH){
    this->cleanup();
      }
    }
  }while(left_exclude.find(result.first) != left_exclude.end() ||
     right_exclude.find(result.second) != right_exclude.end());
  miss_counter = 0; //reset after a hit
  removeNode(choice);
  return result;
  */
}
bool SkipList::isMaxHeap(){
  return _isMaxHeap;
}

void SkipList::partial_cleanup(float tail){
  miss_counter = 0;
  auto start = std::chrono::steady_clock::now();
  std::cout << "partial:old size=" << this->length << std::endl;
  SkipNode * curr = this->head.forward[0];
  
  SkipNode * update[SkipNode::MAX_LEVEL];
  SkipNode * forward[SkipNode::MAX_LEVEL];
  for(int i = 0; i < SkipNode::MAX_LEVEL; ++i){
    update[i] = &(this->head);
    forward[i] = this->head.forward[i];
  }
  while(curr && curr->key <= tail){
    //if curr is alive, point to next living node
    if(left_exclude.find(curr->value.first) == left_exclude.end() &&
       right_exclude.find(curr->value.second) == right_exclude.end()){
      for(int i = 0; i < curr->height; ++i){
    update[i]->forward[i] = curr;
    update[i] = curr;
    forward[i] = curr->forward[i];
      }
      curr = curr->forward[0];
    }else{ //if curr is dead, delete this node
      this->length--;
      for(int i = 0; i < curr->height; ++i){
    forward[i] = curr->forward[i];
      }
      //prev = curr;
      delete curr;
      curr = forward[0];

    }
  }

  //tail for end of list
  for(int i = 0; i < SkipNode::MAX_LEVEL; ++i){
    if(update[i]){
      update[i]->forward[i] = forward[i];
    }
  }

  std::cout << "\tnew size=" << this->length << std::endl;
  auto stop = std::chrono::steady_clock::now();
  auto diff = stop - start;
  cleanup_time += std::chrono::duration_cast<std::chrono::milliseconds>(diff);
  std::cout << "cleanup time = (" << std::chrono::duration_cast<std::chrono::milliseconds>(diff).count()<< "ms)" << std::endl;
}

void SkipList::cleanup(){
  miss_counter = 0;
  auto start = std::chrono::steady_clock::now();
  std::cout << "full: old size=" << this->length << std::endl;
  SkipNode * curr = this->head.forward[0];
  
  SkipNode * update[SkipNode::MAX_LEVEL];
  SkipNode * forward[SkipNode::MAX_LEVEL];
  for(int i = 0; i < SkipNode::MAX_LEVEL; ++i){
    update[i] = &(this->head);
    forward[i] = this->head.forward[i];
  }
  while(curr){
    //if curr is alive, point to this (live) node
    if(left_exclude.find(curr->value.first) == left_exclude.end() &&
       right_exclude.find(curr->value.second) == right_exclude.end()){
      for(int i = 0; i < curr->height; ++i){
    update[i]->forward[i] = curr;
    update[i] = curr;
    forward[i] = curr->forward[i];
      }
      curr = curr->forward[0];
    }else{ //if curr is dead, delete this node
      this->length--;
      for(int i = 0; i < curr->height; ++i){
    forward[i] = curr->forward[i];
      }
      //prev = curr;
      delete curr;
      curr = forward[0];

    }
  }

  //tail case for end of list
  for(int i = 0; i < SkipNode::MAX_LEVEL; ++i){
    if(update[i]){
      update[i]->forward[i] = forward[i];
    }
  }

  std::cout << "new size=" << this->length << std::endl;
  auto stop = std::chrono::steady_clock::now();
  auto diff = stop - start;
  cleanup_time += std::chrono::duration_cast<std::chrono::milliseconds>(diff);
  std::cout << "cleanup time = (" << std::chrono::duration_cast<std::chrono::milliseconds>(diff).count()<< "ms)" << std::endl;
}
/*
 * Don't save the head or there will be two after reload
 */
void SkipList::serialize(std::string fname){
  std::cout << "start serialize " << this->length << std::endl;
  auto start = get_time::now();
  std::ofstream out(fname, std::ofstream::out);
  out << this->length << std::endl;
  SkipNode * curr = this->head.forward[0];
  while(curr){
    //std::cout << "writing" << curr->key << " " << curr->value.first << " " << curr->value.second << " " << curr->height << std::endl;
    out << curr->key << " " << curr->value.first << " " << curr->value.second << " " << curr->height << std::endl;
    curr = curr->forward[0];
  }

  auto stop = get_time::now();
  auto diff = stop - start;
  cleanup_time += std::chrono::duration_cast<std::chrono::milliseconds>(diff);
  std::cout << "time to save = (" << std::chrono::duration_cast<std::chrono::milliseconds>(diff).count()<< "ms)" << std::endl;
  out.close();
}


bool SkipList::deserialize(std::string fname){
  std::cout << "deserialize " << fname << std::endl;
  SkipNode * update[SkipNode::MAX_LEVEL];
  for(int i = 0; i < SkipNode::MAX_LEVEL; ++i){
    update[i] = &(this->head);
  }

  std::ifstream file(fname, std::ifstream::in);
  if(!file.is_open() || !file.good()){
    std::cout << "no file found" << std::endl;
    return false;
  }
  uint height = 1, n1 = 0, n2 = 0;
  float sim = 1;
  
  file >> this->length;
  std::cout << "length " << this->length << std::endl;
  while(file >> sim >> n1 >> n2 >> height){
    //std::cout << "node" << sim << "\t" << n1 << " " << n2 << " " << height << std::endl; 
    SkipNode * curr = new SkipNode(height, sim, std::make_pair(n1, n2));
    for(uint i = 0; i < height; ++i){
      update[i]->forward[i] = curr;
      update[i] = curr;
    }
  }
  for(int i = 0; i < SkipNode::MAX_LEVEL; ++i){
    update[i]->forward[i] = NULL;
  }
  
  return true;
}

void SkipList::test(){
    
  //SkipList list(0.10,true);
  /*
    std::cout << "list initialized. empty() = " << list.empty() << std::endl;
    std::cout << "inserting a value" << list.empty() << std::endl;
    list.insert(0.99, std::make_pair(1, 1001));
    std::cout << "list should be non empty " << !list.empty() << std::endl;
    //std::cout << "list contents " << std::endl;
    //list.head.debug(16);
    std::cout << "testing pop_uniform()" << std::endl;
    std::pair<uint,uint> result1 = list.pop_uniform();
    std::cout << "result is (" << result1.first << ", " << result1.second << ")" << std::endl;
    
    std::cout << "List should be empty " << list.empty() << std::endl;
  */
  /*
  //deterministic test
  list.insert(0.99, std::make_pair(1, 1001));
  list.insert(0.98, std::make_pair(2, 1001));
  list.insert(0.97, std::make_pair(3, 1001));
  list.insert(0.96, std::make_pair(4, 1001));
  list.insert(0.95, std::make_pair(5, 1001));
  list.insert(0.94, std::make_pair(6, 1001));
  list.insert(0.93, std::make_pair(7, 1001));
  */
  /*
  //deterministic reverse test
  list.insert(0.93, std::make_pair(1, 1001));
  list.insert(0.94, std::make_pair(2, 1001));
  list.insert(0.95, std::make_pair(3, 1001));
  list.insert(0.96, std::make_pair(4, 1001));
  list.insert(0.97, std::make_pair(5, 1001));
  list.insert(0.98, std::make_pair(6, 1001));
  list.insert(0.99, std::make_pair(7, 1001));
  */
    
  /*
  //non-deterministic test
  list.insert(0.90, std::make_pair(1, 1001));
  list.insert(0.90, std::make_pair(2, 1001));
  list.insert(0.90, std::make_pair(3, 1001));
  list.insert(0.90, std::make_pair(4, 1001));
  list.insert(0.90, std::make_pair(5, 1001));
  list.insert(0.90, std::make_pair(6, 1001));
  list.insert(0.90, std::make_pair(7, 1001));
  list.insert(0.90, std::make_pair(8, 1001));
  list.insert(0.90, std::make_pair(9, 1001));
    
  list.insert(0.80, std::make_pair(11, 1001));
  list.insert(0.80, std::make_pair(12, 1001));
  list.insert(0.80, std::make_pair(13, 1001));
  list.insert(0.80, std::make_pair(14, 1001));
  list.insert(0.80, std::make_pair(15, 1001));
  list.insert(0.80, std::make_pair(16, 1001));
  */
    
  //non-deterministic test reverse order
  /*
    list.insert(0.80, std::make_pair(11, 1001));
    list.insert(0.80, std::make_pair(12, 1001));
    list.insert(0.80, std::make_pair(13, 1001));
    list.insert(0.80, std::make_pair(14, 1001));
    list.insert(0.80, std::make_pair(15, 1001));
    list.insert(0.80, std::make_pair(16, 1001));
    
    list.insert(0.70, std::make_pair(101, 1001));
    list.insert(0.70, std::make_pair(102, 1001));
    list.insert(0.70, std::make_pair(103, 1001));
    list.insert(0.70, std::make_pair(104, 1001));
    list.insert(0.70, std::make_pair(105, 1001));
    list.insert(0.70, std::make_pair(106, 1001));
  */
  /*
    list.insert(0.90, std::make_pair(1, 1001));
    list.insert(0.90, std::make_pair(2, 1001));
    list.insert(0.90, std::make_pair(3, 1001));
    list.insert(0.90, std::make_pair(4, 1001));
    list.insert(0.90, std::make_pair(5, 1001));
    list.insert(0.90, std::make_pair(6, 1001));
    list.insert(0.90, std::make_pair(7, 1001));
    list.insert(0.90, std::make_pair(8, 1001));
  */
  //list.insert(0.90, std::make_pair(9, 1001));
    
  /*
    std::cout << "testing pop_uniform()" << std::endl;
    while(!list.empty()){
    //list.debug();
    std::pair<uint,uint> result1 = list.pop_uniform();
    std::cout << "(" << result1.first << ", " << result1.second << ")" << std::endl;
    }
    std::cout << "List should be empty " << list.empty() << std::endl;
  */    
  /*
    std::cout << "next pop should fail: " << std::endl;
    list.pop_uniform();
  */
    
}

void SkipList::debug(){
  //std::cout << "length " << this->length << std::endl;
}

void SkipList::perf(){
  std::cout << "insert time (" << this->insert_time.count() << "ms)" << std::endl;
  std::cout << "cleanup time (" << this->cleanup_time.count() << "ms)" << std::endl;
  std::cout << "pop time (" << this->pop_time.count() << "ms)" << std::endl;
  std::cout << "misses = " << this->cumulative_miss << std::endl;
}
