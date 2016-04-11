#include <TinyBasicProgram.hpp>
#include <TestUtils.h>

int main()
{
    Execute(TinyBasic("\
            10 PRINT 'Hello, World!'\n\
            20 GOTO 10\n"));
}
