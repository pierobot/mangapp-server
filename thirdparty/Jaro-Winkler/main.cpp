#include <iostream>

#include "jaroWinkler.hpp"





void test(const std::string& s1, const std::string& s2)
{
    static int exampleCount(0);
    std::cout
        << "Example " << ++exampleCount << ": "
        << s1 << " vs " << s2 << std::endl
        << "  Jaro distance:         " << jaroDistance(s1, s2) << std::endl
        << "  Jaro-Winkler distance: " << jaroWinklerDistance(s1, s2) << std::endl
        << std::endl
        ;
}





int main()
{
    test("MARTHA", "MARHTA");
    test("DWAYNE", "DUANE");
    test("DIXON", "DICKSONX");
    
    return 0;
}

