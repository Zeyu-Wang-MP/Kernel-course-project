#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using namespace std;

int main(){
    char* name1 = (char*)vm_map(nullptr, 0);
    strcpy(name1, "NAME1");
    cout << name1 << endl;
    
    char* name2 = (char*)vm_map(nullptr, 0);
    strcpy(name2, "NAME2");
    cout << name2 << endl;

    char* name3 = (char*)vm_map(nullptr, 0);
    strcpy(name3, "NAME3");
    cout << name3 << endl;

    char* name4 = (char*)vm_map(nullptr, 0);
    strcpy(name4, "NAME4");
    cout << name4 << endl;

    char* name5 = (char*)vm_map(nullptr, 0);
    strcpy(name5, "NAME5");
    cout << name5 << endl;

    // One should be evicted to memory. Test if retrieval works.
    cout << name1 << endl;
    cout << name2 << endl;
    cout << name3 << endl;
    cout << name4 << endl;
    cout << name5 << endl;

    cout << "STDOUT:" << name5 << endl;

}