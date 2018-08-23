#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>

int main(int argc, char **argv)
{
    if(argc != 2)
    {
        std::cerr<<"Incorrect command line format\n";
        exit(-2);
    }
    const char *asm_fname = argv[1];
    std::ifstream fin(asm_fname);
    std::string current_line;
    bool in_memcpy = false;
    while(std::getline(fin, current_line))
    {
        if("memcpy:" == current_line)
        {
            in_memcpy = true;
            continue;
        }
        if(in_memcpy && current_line.size() > 3 && current_line.front() != ' ' && current_line.back() == ':')
        {
            in_memcpy = false;
            continue;
        }
        if(in_memcpy)
            continue;
        if(current_line.find("xmm") != std::string::npos)
        {
            std::cerr<<"Found xmm\n";
            std::cerr<<current_line<<"\n";
            exit(-1);
        }
    }
    return 0;
}
