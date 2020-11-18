#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using namespace std;

int main(){
    if(fork() != 0){
        char* filename = (char*) vm_map(nullptr, 0);
        strcpy(filename, "data1.bin");

        char* file1 = (char*) vm_map(filename, 0);
        cout << file1[0] << endl;

        char* file1_copy = (char*) vm_map(filename, 0);

        char* swap2 = (char*) vm_map(nullptr, 0);

        char* swap3 = (char*) vm_map(nullptr, 0);

        char* swap4 = (char*) vm_map(nullptr, 0);

        swap2[0] = 'a';
        swap3[0] = 'b';
        swap4[0] = 'd';

        cout << file1_copy[0] << endl;
        cout << file1[0] << endl;
    }
    else{
        char* filename = (char*) vm_map(nullptr, 0);
        strcpy(filename, "data1.bin");

        char* file1 = (char*) vm_map(filename, 0);
        cout << file1[0] << endl;

        char* file1_copy = (char*) vm_map(filename, 0);

        char* swap2 = (char*) vm_map(nullptr, 0);

        char* swap3 = (char*) vm_map(nullptr, 0);

        char* swap4 = (char*) vm_map(nullptr, 0);

        swap2[0] = 'a';
        swap3[0] = 'b';
        swap4[0] = 'd';

        cout << file1_copy[0] << endl;
        cout << file1[0] << endl;
    }
}