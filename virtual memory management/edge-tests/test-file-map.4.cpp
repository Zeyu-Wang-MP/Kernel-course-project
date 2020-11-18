#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using namespace std;

/*
    Test vm_map to a (r:1 w:0) page
    Test vm_map to a (r:1 w:1) page
*/
int main(){
    /*--------------SETUP ->  r: 1, w: 0 --------------*/
    // Allocate swap-backed page from the arena 
    char *filename = (char *) vm_map(nullptr, 0);
    strcpy(filename, "shakespeare.txt");

    char *filename2 = (char *) vm_map(nullptr, 0);
    strcpy(filename2, "data1.bin");

    char *file1 = (char *) vm_map (filename, 0);
    char *file2 = (char *) vm_map (filename2, 0);

    // Read r: 0, w: 0 -> r: 1, w: 0
    cout << "Should be 'abc' with ONE fault: " << endl;
    for(int i = 0; i < 3; ++i)
    {
        cout << file1[i]; 
    }
    cout << endl;

    // Read r: 0, w: 0 -> r: 1, w: 0
    cout << "Should be '8eY' with ONE fault: " << endl;
    for(int i = 0; i < 3; ++i)
    {
        cout << file2[i]; 
    }
    cout << endl;
    
    // Map file3 to same place as file 1
    char *file3 = (char *) vm_map (filename, 0);

    /*--------------SETUP ->  r: 1, w: 0 --------------*/

    // Read file3 (No fault!)
    cout << "Read file3. Should be 'abc' with NO fault: " << endl;
    for(int i = 0; i < 3; ++i)
    {
        cout << file3[i]; 
    }
    cout << endl;

    // Write file3 (Fault) r: 1, w: 0 -> r: 1, w: 1
    cout << "Write file3[0] = 'z'. Should incur fault: " << endl;
    file3[0] = 'z';

    cout << "Write file3[0] = 'a'. Should NOT incur fault: " << endl;
    file3[0] = 'a';

    cout << "Write file3[0] = 'z'. Should NOT incur fault: " << endl;
    file3[0] = 'z';

    cout << "Read file3. Should be 'zbc' with NO fault: " << endl;
    for(int i = 0; i < 3; ++i)
    {
        cout << file3[i]; 
    }
    cout << endl;

    // Test shared property
    cout << "Read file1. Should be 'zbc' with NO fault: " << endl;
    for(int i = 0; i < 3; ++i)
    {
        cout << file1[i]; 
    }
    cout << endl;

    cout << "Write file1[0] = 'k' with NO fault: " << endl;
    file1[0] = 'k';

    cout << "Read file3. Should be 'kbc' with NO fault: " << endl;
    for(int i = 0; i < 3; ++i)
    {
        cout << file3[i]; 
    }
    cout << endl;

    cout << "Read file1. Should be 'kbc' with NO fault: " << endl;
    for(int i = 0; i < 3; ++i)
    {
        cout << file1[i]; 
    }
    cout << endl;

    // map to a r:1, w: 1 page
    cout << "Mapping file4 to same as file1 and file 3" << endl;
    char *file4 = (char *) vm_map (filename, 0);

    cout << "Read file4. Should be 'kbc' with NO fault: " << endl;
    for(int i = 0; i < 3; ++i)
    {
        cout << file4[i]; 
    }
    cout << endl;

    cout << "Write file4[0] = 'a' with NO fault: " << endl;
    file1[0] = 'a';

    // Test shared
    cout << "Read file4. Should be 'abc' with NO fault: " << endl;
    for(int i = 0; i < 3; ++i)
    {
        cout << file4[i]; 
    }
    cout << endl;

    cout << "Read file1. Should be 'abc' with NO fault: " << endl;
    for(int i = 0; i < 3; ++i)
    {
        cout << file1[i]; 
    }
    cout << endl;

    cout << "Read file3. Should be 'abc' with NO fault: " << endl;
    for(int i = 0; i < 3; ++i)
    {
        cout << file3[i]; 
    }
    cout << endl;

}