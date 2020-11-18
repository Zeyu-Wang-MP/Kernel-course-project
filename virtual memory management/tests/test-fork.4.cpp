#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using namespace std;

/*
Unsure if proper behavior for 4-credit.
*/
int main(){
    if(fork() == 0)
    {
        cout << "CHILD1 PROCESS RUNNING" << endl;
    }
    else
    {
        cout << "PARENT OF CHILD1 RUNNING" << endl;
    }
    
    
    if(fork() == 0)
    {
        cout << "CHILD2 PROCESS RUNNING" << endl;
        
        /* Allocate swap-backed page from the arena */
        char *filename = (char *) vm_map(nullptr, 0);

        /* Write the name of the file that will be mapped */
        strcpy(filename, "shakespeare.txt");

        /* Map a page from the specified file */
        char *p = (char *) vm_map (filename, 0);

        // Yield to parent, call vm_destroy, then print.
        vm_yield();

        /* Print the first speech from the file */
        cout << "CHILD2 PRINTING" << endl;
        for (unsigned int i=0; i<25; i++) {
        cout << p[i];
        }
    }
    else
    {
        cout << "PARENT OF CHILD2 RUNNING" << endl;
        /* Allocate swap-backed page from the arena */
        char *filename = (char *) vm_map(nullptr, 0);

        /* Write the name of the file that will be mapped */
        strcpy(filename, "shakespeare.txt");

        /* Map a page from the specified file */
        char *p = (char *) vm_map (filename, 0);

        /* Print the first speech from the file */
        cout << "PARENT2 PRINTING" << endl;
        for (unsigned int i=0; i<25; i++) {
        cout << p[i];
        }
    }
    
    
    cout << "RIP THREAD" << endl;

}