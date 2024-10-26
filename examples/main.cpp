#define DEF_FLAG_IMPL
#include "../defFlag.hpp"

#include <iostream>

int main(int argc, char* argv[])
{
    def::Flag flag;

    auto& name = flag.Set("name", "undefined", "specify your name");
    auto& age = flag.Set("age", 0, "specify your age");
    auto& is_cool = flag.Set("cool", false, "specify whether you're cool or not");

    try
    {
        flag.Parse(argc, argv);
    }
    catch(const def::Flag::Exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    std::cout << name << ' ' << age << ' ' << std::boolalpha << is_cool << std::endl;

    return 0;
}
