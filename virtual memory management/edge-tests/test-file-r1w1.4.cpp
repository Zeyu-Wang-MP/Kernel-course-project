#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using namespace std;

/*
    Test outgoing edges from state: 
    r: 1, w: 1 (written in file-backed page)
*/
int main(){
    /*--------------SETUP ->  r: 1, w: 1 --------------*/
    // Allocate swap-backed page from the arena 
    char *filename = (char *) vm_map(nullptr, 0);
    strcpy(filename, "shakespeare.txt");

    char *filename2 = (char *) vm_map(nullptr, 0);
    strcpy(filename2, "data1.bin");

    // currently r: 0, w: 0
    char *file1 = (char *) vm_map (filename, 0);
    char *file2 = (char *) vm_map (filename2, 0);

    // make r: 1, w: 1 by writing
    cout << "writing file1[0] = 'z', should fault: " << endl;
    file1[0] = 'z';

    cout << "writing file2[0] = 'z', should fault: " << endl;
    file2[0] = 'z';

    /*--------------SETUP ->  r: 1, w: 1 --------------*/

    // Read/Write r: 1, w: 1 - see test-file-r0-w0.4.cpp

    // Evict r: 1, w: 1 -> r: 0, w: 0
    // Evict to get r: 0, w: 0
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


    //-------- Check is r: 0, w: 0 ----------//
    // Read r: 0, w: 0 -> r: 1, w: 0
    cout << "Should be 'zbc' with ONE fault: " << endl;
    for(int i = 0; i < 3; ++i)
    {
        cout << file1[i]; 
    }
    cout << endl; 

    // Write at r: 1, w: 0
    cout << "Writing 'a' to file[0], should incur fault: " << endl; 
    file1[0] = 'a';

    // Read at r: 1, w: 1
    cout << "Should be 'abc' with NO fault: "; 
    for(int i = 0; i < 3; ++i)
    {
        cout << file1[i]; 
    }
    cout << endl;

    // Write at w: 1, w: 1
    cout << "Writing 'y' to file[0], NO fault: " << endl; 
    file1[0] = 'y';
    cout << "Should be 'ybc' with NO fault: "; 
    for(int i = 0; i < 3; ++i)
    {
        cout << file1[i]; 
    }
    cout << endl;

    //------------//

    // Write r: 0, w: 0 -> r: 1, w: 1
    cout << "Writing 'a' to file2[0]. should incur fault: " << endl;
    file2[0] = 'a';

    // Read r: 1, w: 1 
    cout << "Reading file2. Should be 'aeY', and NO fault: "; 
    for(int i = 0; i < 3; ++i)
    {
        cout << file2[i]; 
    }
    cout << endl;

    // Write w: 1, w: 1
    cout << "Writing 'K' to file2[0]. should NOT incur fault: " << endl;
    file2[0] = 'K';

    cout << "Reading file2. Should be 'KeY', and NO fault: "; 
    for(int i = 0; i < 3; ++i)
    {
        cout << file2[i]; 
    }
    cout << endl;
    
}