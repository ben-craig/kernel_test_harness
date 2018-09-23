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
    bool in_optimized = false;
    while(std::getline(fin, current_line))
    {
        if(
            "memcpy:" == current_line ||
            "memset:" == current_line ||
            "__std_reverse_trivially_swappable_4:" == current_line ||
            "__std_swap_ranges_trivially_swappable_noalias:" == current_line)
        {
            in_optimized = true;
            continue;
        }
        if(in_optimized && current_line.size() > 3 && current_line.front() != ' ' && current_line.back() == ':')
        {
            in_optimized = false;
        }
        if(in_optimized)
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
