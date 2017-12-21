#include <random>
#include "dummy.h"

extern "C" int main()
{
    std::mt19937_64 gen(1);
    std::uniform_int_distribution<> dis(1, 100);
    *g_test_failures += 1;
    dis(gen);
}
