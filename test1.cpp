#include <TinyBasicProgram.hpp>
#include <TestUtils.h>

int main()
{
    Execute(TinyBasicProgram("10 PRINT 'Hello, World!'\
            20 GOTO 10"));
}
