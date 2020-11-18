#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using namespace std;

/*
    Test outgoing edges from state: 
    r: 1, w: 0 (newly mapped file-backed page that has been read, but NOT written)
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

    cout << "Should be 'abc' with NO fault: " << endl;
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
    
    cout << "Should be '8eY' with NO fault: " << endl;
    for(int i = 0; i < 3; ++i)
    {
        cout << file2[i]; 
    }
    cout << endl; 
    /*--------------SETUP ->  r: 1, w: 0 --------------*/

    

    // Read/Write at r: 1, w: 0, see test-file-r1w0.cpp
    
    // Evict at r:1, w:0
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

    // Read r: 0, w: 0 -> r: 1, w: 0
    cout << "Should be 'abc' with ONE fault: " << endl;
    for(int i = 0; i < 3; ++i)
    {
        cout << file1[i]; 
    }
    cout << endl;

    cout << "Should be 'abc' with NO fault: " << endl;
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
    
    // Confirm r: 1
    cout << "Should be '8eY' with NO fault: " << endl;
    for(int i = 0; i < 3; ++i)
    {
        cout << file2[i]; 
    }
    cout << endl; 

    // Write r: 1, w: 0 -> r: 1, w: 1 (Confirm w: 1)
    cout << "Writing file1[0] = 'z' with ONE fault: " << endl;
    file1[0] = 'z';
    cout << "Should be 'zbc' with NO fault: " << endl;
    for(int i = 0; i < 3; ++i)
    {
        cout << file1[i]; 
    }
    cout << endl; 




}