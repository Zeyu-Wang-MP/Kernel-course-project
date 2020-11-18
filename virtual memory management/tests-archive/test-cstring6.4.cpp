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

    cout << filename[4095];
    // SHOULD OUTPUT ERROR HERE!
    strcpy(p, "shakespeare.txt");

    /* Map shakesphere.txt using page in file-backed space */
    char *a = (char *) vm_map (p, 0);

    /* Print the first speech from the file */
    for (unsigned int i=0; i<2561; i++) {
	cout << a[i];
    }
}