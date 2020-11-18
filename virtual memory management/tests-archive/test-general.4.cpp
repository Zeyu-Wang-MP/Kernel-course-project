#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using namespace std;

int main(){
    // start with r: 0, w: 0
    char* name1 = (char*)vm_map(nullptr, 0);
    cout << name1 << endl;
    strcpy(name1, "TEST");
    cout << name1 << endl;

    // r: 1, w: 1c
    cout << name1 << endl;
    strcpy(name1, "TEST2");
    cout << name1 << endl;


}