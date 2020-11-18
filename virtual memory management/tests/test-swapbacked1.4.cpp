#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using namespace std;

// Test shared swapspace. 
int main()
{
    if(fork() == 0)
    {
        // In swapblocks swap-backed
        /* Allocate swap-backed page from the arena */
        char *filename = (char *) vm_map(nullptr, 0);
        char *filename1 = (char *) vm_map(nullptr, 0);

        /* Write the name of the file that will be mapped */
        strcpy(filename, "data1.bin");

        /* Create more files to evict filename*/
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
        vm_yield(); 

        cout << filename << endl; 
        cout << filename1 << endl; 

        cout << "Only filename should be data1.bin" << endl; 
        vm_yield(); 
        
    }
    else
    {
        // In swapblocks swap-backed
        /* Allocate swap-backed page from the arena */
        char *filename = (char *) vm_map(nullptr, 0);
        char *filename1 = (char *) vm_map(nullptr, 0);

        /* Write the name of the file that will be mapped */
        strcpy(filename, "shakespeare.txt");

        /* Create more files to evict filename*/
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
        vm_yield(); 

        cout << filename << endl; 
        cout << filename1 << endl; 

        cout << "Only filename should be shakespeare.txt" << endl; 
        vm_yield(); 
    }
}
