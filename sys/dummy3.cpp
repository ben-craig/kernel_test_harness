#include <random>
#include <cassert>
#include <cassert>

extern "C" int main()
{
    std::mt19937_64 gen(1);
    std::uniform_int_distribution<> dis(1, 100);
    assert(dis(gen) == 3);
}
