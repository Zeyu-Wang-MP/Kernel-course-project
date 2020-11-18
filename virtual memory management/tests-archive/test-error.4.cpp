#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using std::cout;

int main()
{
    /* Allocate swap-backed page from the arena */
    char *filename = (char *) vm_map(nullptr, 0);

    /* Write the name of the file that will be mapped */
    strcpy(filename, "INVALID.txt");

    /* Map a page from the specified file */
    char *p = (char *) vm_map (filename, 0);

    cout << "NO PROBLEM";

    /* Print the first speech from the file */
    for (unsigned int i=0; i<2561; i++) {
	cout << p[i];
    }
}
