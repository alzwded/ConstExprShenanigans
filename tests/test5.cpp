#include <TinyBasicProgram.hpp>
#include <TestUtils.h>

int main()
{
    Execute(TinyBasic("\
10 LET X = 1 + 2\n\
20 PRINT X"));
}
