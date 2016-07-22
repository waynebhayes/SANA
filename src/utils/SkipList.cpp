#include "SkipList.hpp"
SkipNode::SkipNode(){
	height = MAX_LEVEL;
	key = 0;
	value = std::make_pair(0,0);
	for(int i = 0; i < MAX_LEVEL; ++i){
		forward[i] = NULL;
	}
}

SkipNode::SkipNode(int h, double k, std::pair<ushort,ushort> & v)
	:height(h), key(k), value(v){
	for(int i = 0; i < MAX_LEVEL; ++i){
		forward[i] = NULL;
	}
}

SkipNode::~SkipNode(){
	//delete[] forward;
}

/*
	limit: number of levels to traverse 
	prevents infinite loops 
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



SkipList::SkipList()
	:_isMaxHeap(false), length(0), level(0), delta(0.0 + EPSILON){
	//this->head created automatically, no initialization needed
	srand(std::random_device()());
	rng.seed(std::random_device()());
	this->debug();
}

SkipList::SkipList(double delta)
	:_isMaxHeap(false), length(0), level(0), delta(delta + EPSILON){
	//this->head created automatically, no initialization needed
	srand(std::random_device()());
	rng.seed(std::random_device()());
	this->debug();
}

SkipList::SkipList(double delta, bool setMaxHeap)
	:_isMaxHeap(setMaxHeap), length(0), level(0), delta(delta + EPSILON){
	srand(std::random_device()());
	rng.seed(std::random_device()());
	this->debug();
}

SkipList::~SkipList(){
	//nothing to do
}

int SkipList::random_height(void){
	int level = 1;
	while ((rand()&0xFFFF) < (SkipList::SKIPLIST_P * 0xFFFF) && level < SkipNode::MAX_LEVEL)  //why multiply here?
		level += 1;
	return (level<SkipNode::MAX_LEVEL) ? level : SkipNode::MAX_LEVEL;
}

void SkipList::insert(double similarity, std::pair<ushort,ushort> entry){
	similarity = _isMaxHeap ? (1 - similarity) : similarity;
	
	SkipNode * curr;
	SkipNode * update[SkipNode::MAX_LEVEL];
	for(int i = 0; i < SkipNode::MAX_LEVEL; ++i){
		update[i] = NULL;
	}
	
	curr = &(this->head);
	for(int i = this->level - 1; i >=0; --i){
		while(curr->forward[i] && curr->forward[i]->key < similarity){
			curr = curr->forward[i];
		}
		update[i] = curr;
	}
	
	int height = random_height();
	if(height > this->level){
		for(int i = this->level; i < height; i++){
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
}
	
bool SkipList::empty(){
	if(this->length < 0){
		std::cerr << "Error:fewer than 0 items in list!" << std::endl;
		//could throw an exception if needed
	}
	return this->length <= 0;
}

/*
 * super slow O(n*h) method
 * check every node for a pointer to this skipnode
 * if so, move it to the next node
 */
void SkipList::removeNode(SkipNode * n){
	SkipNode * curr = &(this->head);
	while(curr && curr->key <= n->key){
		for(int i = curr->height; i >= 0; --i){
			if(curr->forward[i] == n){
				curr->forward[i] = n->forward[i];
			}
		}
		curr = curr->forward[0];
	}
	delete n;
}

std::pair<ushort,ushort> SkipList::pop_uniform(){
	if(this->empty()){
		//throw new std::exception("SkipList is empty");
		std::cout << "Error: list is empty" << std::endl;
		return std::make_pair(0,0);
	}
	SkipNode * choice = this->head.forward[0];
	SkipNode * curr = choice;
	double tail = curr->key + this->delta;
	
	int n = 0;
	while(curr->forward[0] && curr->forward[0]->key <= tail){
		curr = curr->forward[0];
		std::uniform_int_distribution<int> uni(0,++n); 
		if(uni(rng) == 0){
			choice = curr;
		}
	}
	std::pair<ushort,ushort> result = choice->value;
	removeNode(choice);
	this->length -= 1;
	this->debug();
	return result;
}

/*
 * WARNING: this function doesn't work in max heap mode due to 
 * the random selection function 
 */
std::pair<ushort,ushort> SkipList::pop_distr(){
	/*
	if(this->isMaxHeap()){
		std::cerr << "ERROR: not implemented for max heap yet" << std::endl;
		//throw an exception
	}
	*/
	if(this->empty()){
		//throw new std::exception("SkipList is empty");
		std::cout << "Error: list is empty" << std::endl;
		return std::make_pair(0,0);
	}
	std::pair<ushort,ushort> result;
	if(this->length == 1){
		result = this->head.forward[0]->value;
		removeNode(this->head.forward[0]);
		return result;
	}
	
	SkipNode * curr = &(this->head);
	double start = this->head.forward[0]->key;
	std::uniform_real_distribution<double> uni(start,start+this->delta);
	double pick_value = uni(rng);
	for(int i = this->level - 1; i >= 0; --i){
		while(curr->forward[i] && (curr->forward[i]->key <= pick_value)){
			curr = curr->forward[i];
			if(curr->key == pick_value){
				result = curr->value;
                removeNode(curr);
                return result;
			}
		}
	}
	result = curr->value;
	removeNode(curr);
	this->length -= 1;
	return result;
}
bool SkipList::isMaxHeap(){
	return _isMaxHeap;
}

void SkipList::test(){
	
	SkipList list(0.10,true);
	/*
	std::cout << "list initialized. empty() = " << list.empty() << std::endl;
	std::cout << "inserting a value" << list.empty() << std::endl;
	list.insert(0.99, std::make_pair(1, 1001));
	std::cout << "list should be non empty " << !list.empty() << std::endl;
	//std::cout << "list contents " << std::endl;
	//list.head.debug(16);
	std::cout << "testing pop_uniform()" << std::endl;
	std::pair<ushort,ushort> result1 = list.pop_uniform();
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
	list.insert(0.90, std::make_pair(9, 1001));
	

	std::cout << "testing pop_uniform()" << std::endl;
	while(!list.empty()){
		//list.debug();
		std::pair<ushort,ushort> result1 = list.pop_uniform();
		std::cout << "(" << result1.first << ", " << result1.second << ")" << std::endl;
	}
	std::cout << "List should be empty " << list.empty() << std::endl;
	
	/*
	std::cout << "next pop should fail: " << std::endl;
	list.pop_uniform();
	*/
	
}

void SkipList::debug(){
	std::cout << "length " << this->length << std::endl;
	/*
	std::cout << "head " << std::endl;
	head.debug(16);
	*/
}
/*
int main(int argc, char **argv){
	std::cout << "running skiplist test" << std::endl;
	SkipList::test();
	std::cout << "skiplist test completed" << std::endl;
}
*/
