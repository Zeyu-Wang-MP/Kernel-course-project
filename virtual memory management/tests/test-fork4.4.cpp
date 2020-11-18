#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using std::cout;

// yield when no other processes available
int main()
{
    /* Allocate swap-backed page from the arena */
    char *filename = (char *) vm_map(nullptr, 0);

    /* Write the name of the file that will be mapped */
    strcpy(filename, "shakespeare.txt");

    /* Map a page from the specified file */
    char *p = (char *) vm_map (filename, 0);
    char *a = (char *) vm_map (filename, 0);

    /* Print the first speech from the file */
    for (unsigned int i=0; i<25; i++) {
	cout << p[i];
    }

    // Should only have one fault between p and a
    strcpy(p, "HELLO WORLD");

    // p and a share same resource. a should see changes.
    for (unsigned int i=0; i<25; i++) {
	cout << a[i];
    }

    vm_yield();

    // p and a share same resource. a should see changes.
    for (unsigned int i=0; i<25; i++) {
	cout << a[i];
    }

}
