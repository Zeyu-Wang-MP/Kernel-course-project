#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using std::cout;

int main(){
    char* swap = (char*)vm_map(nullptr, 0);
    strcpy(swap, "asdasdasdasdasd");
    char* invalid_file_page = (char*) vm_map(swap, 0);
    char* file_page_use_name_in_invalid_file_page = (char*) vm_map(invalid_file_page, 0);
    cout << file_page_use_name_in_invalid_file_page << std::endl;
}