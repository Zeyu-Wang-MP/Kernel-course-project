#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using namespace std;

int main(){
    char* name1 = (char*)vm_map(nullptr, 0);
    cout << name1 << endl;
    
    char* name2 = (char*)vm_map(nullptr, 0);
    cout << name2 << endl;

    char* name3 = (char*)vm_map(nullptr, 0);
    cout << name3 << endl;

    char* name4 = (char*)vm_map(nullptr, 0);
    cout << name4 << endl;

    // Above names are mapped to pinned page. This should not cause eviction.
    char* name5 = (char*)vm_map(nullptr, 0);
    strcpy(name5, "NAME5");
    cout << name5 << endl;

}