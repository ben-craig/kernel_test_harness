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
        if(current_line == "memcpy:"
            || current_line == "memset:"
            || current_line == "__memset_repmovs:"
            || current_line == "__memset_query:"
            || current_line == "__std_find_trivial_4:"
            || current_line == "__std_count_trivial_4:"
            || current_line == "`anonymous namespace'::_Minmax_element<1,`anonymous namespace'::_Minmax_traits_4>:"
            || current_line == "`anonymous namespace'::_Minmax_element<2,`anonymous namespace'::_Minmax_traits_4>:"
            || current_line == "`anonymous namespace'::_Minmax_element<3,`anonymous namespace'::_Minmax_traits_4>:"
            || current_line == "__std_reverse_trivially_swappable_4:"
            || current_line == "__std_reverse_copy_trivially_copyable_4:"
            || current_line == "__std_swap_ranges_trivially_swappable_noalias:"
        )
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
            // allow this pattern, which is used when initializing stack arrays
            //0000000140007F82: 0F 57 C0           xorps       xmm0,xmm0
            //0000000140007F85: F2 0F 11 84 24 D8  movsd       mmword ptr [rsp+17D8h],xmm0
            if(current_line.find("xorps       xmm0,xmm0") != std::string::npos)
            {
                continue;
            }
            if(current_line.find("movsd       mmword ptr") != std::string::npos)
            {
                continue;
            }
            std::cerr<<"Found xmm\n";
            std::cerr<<current_line<<"\n";
            exit(-1);
        }
    }
    return 0;
}
