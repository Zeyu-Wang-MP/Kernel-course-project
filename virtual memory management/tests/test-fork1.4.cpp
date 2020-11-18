#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using std::cout;

// No evictions. Test if shared filebacked resource works for vm_fault.
int main()
{
    if(fork() == 0)
    {

    }

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

    for (unsigned int i=0; i<25; i++) {
	cout << a[i];
    }

    // Should only have one fault between p and a
    strcpy(p, "HELLO WORLD");

    // p and a share same resource. a should see changes.
    for (unsigned int i=0; i<25; i++) {
	cout << a[i];
    }
    for (unsigned int i=0; i<25; i++) {
	cout << p[i];
    }

    char *c = (char *) vm_map (filename, 0);

    for (unsigned int i=0; i<25; i++) {
	cout << c[i];
    }

    vm_yield();

}
