#include <TinyBasicProgram.hpp>
#include <TestUtils.h>

int main()
{
    Execute(TinyBasic("\
5  LET Y = 3\n\
10 LET X = Y + \n\
20 PRINT X"));
}
