#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using namespace std;

/*
    Test outgoing edges from state: 

    r: 1, w: 0 (first call vm_map for swapback)
*/
int main(){
    // Allocate swap-backed page from the arena 
    char *filename = (char *) vm_map(nullptr, 0);

    // Read r: 1, w:0 (should be 0s, no fault)
    for (unsigned int i=0; i<20; i++) {
	cout << "Index " << i << " should be nothing: " << filename[i] << endl;
    }
    cout << "Should be nothing (4095): " << filename[4095] << endl;

    // Write r: 1, w:0 (should be fault)
    cout << "Fault here: " << endl; 
    strcpy(filename, "shakespeare.txt");

    // Read r: 1, w: 1
    cout << "Should be 'shakespeare.txt' : "<< filename << endl;
}