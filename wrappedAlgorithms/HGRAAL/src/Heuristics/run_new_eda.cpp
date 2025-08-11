#include "../common.h"
#include "new_eda.h"

void run_new_eda(){
    NEW_EDA heuristic("Problems/nug30.dat");
    heuristic.start(1, 100, 0.5, "nug30.dat");
    for (int i = 0; i < 100; ++i){
        cout << "here1 " << i << endl;
        heuristic.step();
        cout << "here2 " << i << endl;
    }
    heuristic.stop();
}
