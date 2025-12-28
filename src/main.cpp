#include <iostream>
#include <ncurses.h>
#include <myWindow.h>
#include <cursor.h>
#include <utils.h>
#include <myFile.h>
#include <myApplication.h>

using namespace std;

int main(int argc, char* argv[]){

    if(argc == 1){
        std::cerr << "Too few arguments." << std::endl;
        return 0;
    }

    if(argc > 3) {
        std::cerr << "Too many arguments." << std::endl;
        return 0;
    }

    std::string mode = argv[1];

    std::string path = argv[argc-1];

    Application app(path, argc == 3 && mode == "-t", argc == 3 && mode == "-R");

    if(!app.initialize()) {
        std::cerr << "Failed to initialize." << std::endl;
        return 0;
    }

    app.run();

    return 0;
}