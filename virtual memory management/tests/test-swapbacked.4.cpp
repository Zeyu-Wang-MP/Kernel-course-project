#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using std::cout;

// Test that mapping more than limit of swapblocks to pinned page is NOT valid.
int main()
{
    // In swapblocks swap-backed
    /* Allocate swap-backed page from the arena */
    char *filename = (char *) vm_map(nullptr, 0);

    /* Write the name of the file that will be mapped */
    strcpy(filename, "data1.bin");

    /* Fill swap space */
    for (int i = 0; i < 1024; ++i)
    {
        char *test = (char *) vm_map(nullptr, 0);
        //cout << i << std::endl;
        if(test == nullptr)
        {
            break;
        }
    }

    cout << "hello" << std::endl;
}
