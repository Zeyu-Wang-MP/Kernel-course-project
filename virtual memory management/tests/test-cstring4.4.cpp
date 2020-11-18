#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using namespace std;

/* 
TESTCASE WIP:
Evict page with filename containing desired c-string. 
Then check reading for a filebacked page reads it correctly.
*/

int main(){
    /* Allocate swap-backed page from the arena */
    char *filename = (char *) vm_map(nullptr, 0);

    /* Write the name of the file that will be mapped */
    strcpy(filename, "shakespeare.txt");

    /* Map a page from the specified file */
    char *p = (char *) vm_map (filename, 0);

    strcpy(p, "shakespeare.txt");

    /* Map shakesphere.txt using page in file-backed space */
    char *a = (char *) vm_map (p, 0);

    /* Print the first speech from the file */
    for (unsigned int i=0; i<2561; i++) {
	cout << a[i];
    }
}