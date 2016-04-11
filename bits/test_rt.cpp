#include "buffer.hpp"
#include "validate.hpp"
#include "parser_rt.hpp"
#include <cstdio>

#ifndef TEST
# error "Please specify a test."
#endif

using namespace Jak;
int main()
{
#if TEST == 1
    TinyBasicParser p(Buf("10 PRINT 'Hello, World!'"));
    int refCode = 0;
#elif TEST == 2
    TinyBasicParser p(Buf("\
10 PRINT 'Hello, World!'\n\
20 GOTO 10"));
    int refCode = 0;
#elif TEST == 3
    TinyBasicParser p(Buf("\
10 PRINT 'Hello, World!'\n\
20 GOTO 10\n"));
    int refCode = 0;
#endif
    auto code = static_cast<int>(p.file().code());
    printf("%d\n", code);
    printf("\n%s\n", (code == refCode) ? "PASS" : "FAIL");
}
