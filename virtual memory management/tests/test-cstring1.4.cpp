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

    /* Map a page from the specified file */
    char *p = (char *) vm_map (filename, 0);

    /* Print the first speech from the file */
    for (unsigned int i=0; i<2561; i++) {
	cout << p[i];
    }
}