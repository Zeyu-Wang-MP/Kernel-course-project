#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using namespace std;

// For 6 credit only. Test copy on write for all swapback file states
int main()
{
    /* Allocate swap-backed page from the arena */
    char *pinned_r1w0 = (char *) vm_map(nullptr, 0);
    
    /* Write the name of the file that will be mapped */
    char *indisk_r0w0 = (char *) vm_map(nullptr, 0);
    strcpy(indisk_r0w0, "shakespeare.txt");

    /* Evict r0w0 to disk */
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

    // In memory swap-backed
    char *inmem_r1w1 = (char *) vm_map(nullptr, 0);
    strcpy(inmem_r1w1, "shakespeare.txt");

    if(fork() == 0)
    {
        // Only reads. Should be shared so far.
        cout << inmem_r1w1 << endl; 
        cout << indisk_r0w0 << endl; 
        cout << pinned_r1w0 << endl;
        
    }
    else
    {
        // Force child to read first 
        vm_yield();
        strcpy(inmem_r1w1, "data1.bin");
        cout << inmem_r1w1 << endl;
        strcpy(indisk_r0w0, "data1.bin");
        cout << indisk_r0w0 << endl;
        strcpy(pinned_r1w0, "data1.bin");
        cout << pinned_r1w0 << endl;
        
    }

    // Both processes call this. Output should be different 
    // - shakespeare.txt for child
    // - data1.bin for parent. 
    cout << inmem_r1w1 << endl; 
    cout << indisk_r0w0 << endl; 
    cout << pinned_r1w0 << endl;

    vm_yield();
    
    // Random stuff
    strcpy(inmem_r1w1, "data2.bin");
    strcpy(pinned_r1w0, "data2.bin");
    strcpy(indisk_r0w0, "data2.bin");

    cout << inmem_r1w1 << endl; 
    cout << indisk_r0w0 << endl; 
    cout << pinned_r1w0 << endl;

}
