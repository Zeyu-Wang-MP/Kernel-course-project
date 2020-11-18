#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using namespace std;

/*
    Test outgoing edges from state: 
    r: 0, w: 0 (swapback page that was written, then evicted)
*/
int main(){
    /*--------------SETUP ->  r: 0, w: 0 --------------*/
    // Allocate swap-backed page from the arena 
    char *filename1 = (char *) vm_map(nullptr, 0);
    char *filename2 = (char *) vm_map(nullptr, 0);
    // Write r: 1, w:0 (should be fault) -> r: 1 w: 1
    cout << "Fault here (write shakespeare.txt to filename2):: " << endl;
    strcpy(filename1, "shakespeare.txt");
    cout << "Fault here (write data1.bin to filename2): " << endl;
    strcpy(filename2, "data1.bin");

    // Evict to get into r:0, w: 0
    char *filler1 = (char *) vm_map(nullptr, 0);
    strcpy(filler1, "FILLER");
    char *filler2 = (char *) vm_map(nullptr, 0);
    strcpy(filler2, "FILLER");
    char *filler3 = (char *) vm_map(nullptr, 0);
    strcpy(filler3, "FILLER");
    char *filler4 = (char *) vm_map(nullptr, 0);
    strcpy(filler4, "FILLER");
    char *filler5 = (char *) vm_map(nullptr, 0);
    strcpy(filler5, "FILLER");
    char *filler6 = (char *) vm_map(nullptr, 0);
    strcpy(filler6, "FILLER");
    char *filler7 = (char *) vm_map(nullptr, 0);
    strcpy(filler7, "FILLER");
    char *filler8 = (char *) vm_map(nullptr, 0);
    strcpy(filler8, "FILLER");
    /*--------------SETUP ->  r: 0, w: 0 --------------*/

    // filename and filename2 are both r: 0, w: 0 (in disk)

    // Read r: 0, w: 0 -> r: 1, w: 0
    cout << "Should be 'shakespeare.txt' and WITH fault: "<< filename1 << endl;

    // Check it is r: 1, w: 0
    cout << "Should be 'shakespeare.txt' and NO fault: "<< filename1 << endl;
    cout << "Should be 'test.txt' and WITH fault: " << endl; 
    strcpy(filename1, "test.txt");
    cout << filename1 << endl; 

    // Write r: 0, w: 0 -> r: 1, w: 1
    cout << "Writing as 'test.txt' and WITH fault" << endl;
    strcpy(filename2, "test.txt");
    cout << "Should be 'test.txt' and NO fault: "<< filename2 << endl;

    // Evict r: 0, w: 0 -> Cannot be evicted (already in disk)
}