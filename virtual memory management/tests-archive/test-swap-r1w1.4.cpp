#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using namespace std;

/*
    Test outgoing edges from state: 
    r: 1, w: 1 (swapback page that was written)
*/
int main(){
    /*--------------SETUP ->  r: 1, w: 1 --------------*/
    // Allocate swap-backed page from the arena 
    char *filename = (char *) vm_map(nullptr, 0);

    // Write r: 1, w:0 (should be fault) -> r: 1 w: 1
    cout << "Fault here: " << endl;
    strcpy(filename, "shakespeare.txt");

    
    /*--------------SETUP ->  r: 1, w: 1 --------------*/

    // Read r: 1, w: 1 -> r: 1, w: 1
    cout << "Should be 'shakespeare.txt' and no fault: "<< filename << endl;

    // Write r: 1, w: 1 -> r: 1, w: 1 (no fault)
    cout << "Writing as 'test.txt' and NO fault" << endl;
    strcpy(filename, "test.txt");
    cout << "Should be 'test.txt' and no fault: "<< filename << endl;

    // Evict r:1 w: 1 -> r: 0 w: 0 
    // see test-swap-r0w0.4.cpp setup
}