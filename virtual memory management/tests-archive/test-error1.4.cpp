#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using std::cout;

int main()
{
    /* Allocate swap-backed page from the arena */
    char *filename = (char *) vm_map(nullptr, 0);
    
    filename[4095] = 'T';
     cout << filename[4095];

    /* Write the name of the file that will be mapped */
    filename[4096] = 'T';

    cout << "THIS SHOULD NOT OUTPUT";

    /* Print the first speech from the file */
    for (unsigned int i=0; i<2561; i++) {
	cout << filename[i];
    }
}
