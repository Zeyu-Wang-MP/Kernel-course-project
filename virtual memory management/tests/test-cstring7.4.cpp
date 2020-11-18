#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"
using namespace std;


/* 
Test for when c-string doesn't end.
*/

int main(){

    /* Allocate swap-backed page from the arena */
    char *filename = (char *) vm_map(nullptr, 0);
    char *filename2 = (char *) vm_map(nullptr, 0);

    /* Write the name of the file that will be mapped */
    filename[4095] = 's';
    filename2[0] = 'h';

    /* Map a page from the specified file */
    char *p = (char *) vm_map (filename+4095, 0);

    // vm_map should have returned nullptr (maybe)
    cout << filename[4095];
    strcpy(p, "shakespeare.txt");

}