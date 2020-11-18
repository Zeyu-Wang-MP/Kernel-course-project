#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using std::cout;


int main()
{
    // In swapblocks swap-backed
    /* Allocate swap-backed page from the arena */
    char *filename = (char *) vm_map(nullptr, 0);

    /* Write the name of the file that will be mapped */
    strcpy(filename, "shakespeare.txt");

    /* Map two pages to the specified file */
    char *indisk_file1 = (char *) vm_map (filename, 0);
    char *indisk_file2 = (char *) vm_map (filename, 0);

    /* Read to bring into memory */
    cout << "indisk_file1 (should be 'a'): " << indisk_file1[0] << std::endl;
    cout << "indisk_file2 (should be 'a'): " << indisk_file2[0] << std::endl;
    
    cout << "modifying indisk_file2 to t" << std::endl; 
    indisk_file2[0] = 't';

    cout << "indisk_file1 (should be 't'): " << indisk_file1[0] << std::endl;
    cout << "indisk_file2 (should be 't'): " << indisk_file2[0] << std::endl;
    
    cout << "evicting indisk_file1 and indisk_file2" << std::endl; 

    /* Evict filename and indisk_file */
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

    /* Map new page to same place in filename */
    char *indisk_file3 = (char *) vm_map (filename, 0);
    cout << "indisk_file3 should be 't': " << indisk_file2[0] << std::endl;
    cout << "indisk_file1 should be 't': " << indisk_file1[0] << std::endl;
    cout << "indisk_file2 should be 't': " << indisk_file2[0] << std::endl;

    
}
