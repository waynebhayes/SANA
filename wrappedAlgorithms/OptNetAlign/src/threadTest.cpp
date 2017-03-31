#include <iostream>
#include <thread>
using namespace std;

int main(){
	auto worker = [](){
		cout<<"hello from thread"<<endl;
	};
	thread t1(worker);
	thread t2(worker);
	t1.join();
	t2.join();

	return 0;
}