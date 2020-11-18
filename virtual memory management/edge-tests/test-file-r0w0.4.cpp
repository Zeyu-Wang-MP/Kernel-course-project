#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using namespace std;

/*
    Test outgoing edges from state: 
    r: 0, w: 0 (newly mapped file-backed page)
*/
int main(){
    /*--------------SETUP ->  r: 0, w: 0 --------------*/
    // Allocate swap-backed page from the arena 
    char *filename = (char *) vm_map(nullptr, 0);
    strcpy(filename, "shakespeare.txt");

    char *filename2 = (char *) vm_map(nullptr, 0);
    strcpy(filename2, "data1.bin");

    char *file1 = (char *) vm_map (filename, 0);
    char *file2 = (char *) vm_map (filename2, 0);
    /*--------------SETUP ->  r: 0, w: 0 --------------*/

    // Read r: 0, w: 0 -> r: 1, w: 0
    cout << "Should be 'abc' with ONE fault: " << endl;
    for(int i = 0; i < 3; ++i)
    {
        cout << file1[i]; 
    }
    cout << endl; 

    // Read r: 1, w: 0 
    cout << "Should be 'abc' with NO fault: " << endl;
    for(int i = 0; i < 3; ++i)
    {
        cout << file1[i]; 
    }
    cout << endl; 

    // Write at r: 1, w: 0
    cout << "Writing 'z' to file[0], should incur fault: " << endl; 
    file1[0] = 'z';

    // Read at r: 1, w: 1
    cout << "Should be 'zbc' with NO fault: "; 
    for(int i = 0; i < 3; ++i)
    {
        cout << file1[i]; 
    }

    // Write at w: 1, w: 1
    cout << "Writing 'y' to file[0], NO fault: " << endl; 
    file1[0] = 'y';
    cout << "Should be 'ybc' with NO fault: "; 
    for(int i = 0; i < 3; ++i)
    {
        cout << file1[i]; 
    }

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

    // Write w: 1, w: 1
    cout << "Writing 'K' to file2[0]. should NOT incur fault: " << endl;
    file2[0] = 'K';

    cout << "Reading file2. Should be 'KeY', and NO fault: "; 
    for(int i = 0; i < 3; ++i)
    {
        cout << file2[i]; 
    }
    
    //------------//

    // Evict r:0 w: 0 -> cannot evict on newly mapped page 
}