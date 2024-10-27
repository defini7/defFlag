#define DEF_FLAG_IMPL
#include "../defFlag.hpp"

#include <iostream>

int main(int argc, char* argv[])
{
    def::Flag flag;

    auto& name = flag.Set("name", "undefined", "your name");
    auto& age = flag.Set("age", 0, "your age");
    auto& isCool = flag.Set("cool", false, "are you cool or not");

    try
    {
        for (int i = flag.Parse(argc, argv); i < argc; i++)
            std::cout << argv[i] << ' ';

        std::cout << std::endl;
    }
    catch(const def::Flag::Exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    std::cout << name << ' ' << age << ' ' << std::boolalpha << isCool << std::endl;

    return 0;
}
