#include "File_server.h"
#include <iostream>
#include <exception>
#include <cstdlib>

int main(int argc, char* argv[]){
    if(argc > 2){
        std::cerr << "Usage: fs <port> or fs" << std::endl;
    }
    try{
        File_server fs(argc == 1 ? 0 : atoi(argv[1]));
        fs.start();
    }
    catch(const std::exception& e){
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}