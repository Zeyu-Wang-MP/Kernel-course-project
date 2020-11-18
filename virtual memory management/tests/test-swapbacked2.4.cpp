#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using std::cout;

int main()
{
    char *filename = (char *) vm_map(nullptr, 0);

    strcpy(filename, "data1.bin");
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
    char *filler9 = (char *) vm_map(nullptr, 0);
    cout << filler1 << std::endl;
    cout << filler2 << std::endl;
    cout << filler3 << std::endl;
    cout << filler4 << std::endl;
    cout << filler5 << std::endl;
    cout << filler6 << std::endl;
    cout << filler7 << std::endl;
    cout << filler8 << std::endl;
    strcpy(filler9, filler8);
    strcpy(filler8, "changed");
    cout << filler1 << std::endl;
    cout << filler2 << std::endl;
    cout << filler3 << std::endl;
    cout << filler4 << std::endl;
    cout << filler5 << std::endl;
    cout << filler6 << std::endl;
    cout << filler7 << std::endl;
    cout << filler8 << std::endl;
    cout << filler9 << std::endl;
}
