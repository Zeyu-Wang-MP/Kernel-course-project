#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using namespace std;

int main(){
    /* Allocate swap-backed page from the arena */
    char *filename = (char *) vm_map(nullptr, 0);

    /* Write the name of the file that will be mapped */
    strcpy(filename, "shakespeare.txt");

    /* Map a page from the specified file */
    // nothing should be evicted from here becuase one-four referencing same filename + block
    char *one = (char *) vm_map (filename, 0);
    char *two = (char *) vm_map (filename, 0);
    char *three = (char *) vm_map (filename, 0);
    char *four = (char *) vm_map (filename, 0);
    char *five = (char *) vm_map (filename, 1);


    /* Print the first speech from the file */
    for (unsigned int i=0; i<2561; i++) {
	cout << one[i] << " " << two[i] << " " << three[i] << " " << four[i] << " " << five[i];
    }

}