#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using std::cout;

// For 6 credit only. General test case.

int main()
{
    // In swapblocks swap-backed
    /* Allocate swap-backed page from the arena */
    char *swapblock_page = (char *) vm_map(nullptr, 0);

    /* Write the name of the file that will be mapped */
    strcpy(swapblock_page, "shakespeare.txt");

    /* In disk file */
    char *indisk_file = (char *) vm_map (swapblock_page, 0);

    /* Evict swapblock_page and indisk_file */
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

    // In memory swap-backed
    char *physmem_swap = (char *) vm_map(nullptr, 0);
    strcpy(physmem_swap, "shakespeare.txt");

    // In 'memory' file-backed
    char *in_memory_file = (char *) vm_map (swapblock_page, 0);
    cout << "swapblock_page name: " << swapblock_page << std::endl;

    // In memory file-backed 2
    char *in_memory_file2 = (char *) vm_map(swapblock_page, 0);
    cout << in_memory_file2[0] << std::endl; 

    if(fork() == 0)
    {
        /* Print the first speech from the file in memory */
        for (unsigned int i=0; i<30; i++) {
        cout << in_memory_file[i];
        }

        in_memory_file[0] = 'z';

        /* Print the first speech from the file (that was) on disk*/
        for (unsigned int i=0; i<30; i++) {
        cout << indisk_file[i];
        }

        /* Print the first speech from the file in memory */
        for (unsigned int i=0; i<30; i++) {
        cout << in_memory_file2[i];
        }

        in_memory_file2[1] = 'z';
    }
    else
    {
        /* Print the first speech from the file in memory */
        for (unsigned int i=0; i<30; i++) {
        cout << in_memory_file[i];
        }

        /* Print the first speech from the file (that was) on disk*/
        for (unsigned int i=0; i<30; i++) {
        cout << indisk_file[i];
        }

        /* Print the first speech from the file in memory */
        for (unsigned int i=0; i<30; i++) {
        cout << in_memory_file2[i];
        }
    }
}
