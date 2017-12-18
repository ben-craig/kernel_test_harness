#include "dummy.h"
#include <random>

extern "C" int add_one()
{
    std::mt19937_64 gen(1);
    std::uniform_int_distribution<> dis(1, 100);
    return dis(gen);
}
