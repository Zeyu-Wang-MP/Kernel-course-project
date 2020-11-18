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

    strcpy(p, "speare.txt");

    char *filename1 = (char *) vm_map(nullptr, 0);
    filename1[4091] = 's';
    filename1[4092] = 'h';
    filename1[4093] = 'a';
    filename1[4094] = 'k';
    filename1[4095] = 'e';

    char *filename2 = (char *) vm_map(filename, 0);

    /* Map shakesphere.txt using half a page in filename1 (swap) and half in filename2 (file) */
    char *a = (char *) vm_map (filename1 + 4091, 0);

    /* Print the first speech from the file */
    for (unsigned int i=0; i<2561; i++) {
	cout << a[i];
    }
}