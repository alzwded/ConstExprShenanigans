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
    int refLine = 1;
#elif TEST == 2
    TinyBasicParser p(Buf("\
10 PRINT 'Hello, World!'\n\
20 GOTO 10"));
    int refCode = 0;
    int refLine = 2;
#elif TEST == 3
    TinyBasicParser p(Buf("\
10 PRINT 'Hello, World!'\n\
20 GOTO 10\n"));
    int refCode = 0;
    int refLine = 3;
#elif TEST == 4
    TinyBasicParser p(Buf("\
10 PIRNT 'Hello, World!'"));
    int refCode = 12;
    int refLine = 1;
#endif
    auto rv = p.file();
    auto code = static_cast<int>(rv.code());
    auto line = rv.lineNo();
    printf("%d %d\n", code, line);
    printf("\n%s\n", (code == refCode && line == refLine) ? "PASS" : "FAIL");
}
