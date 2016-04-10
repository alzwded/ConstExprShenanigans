#include "buffer.hpp"
#include "validate.hpp"
#include "parser_rt.hpp"
#include <cstdio>

using namespace Jak;
int main()
{
    TinyBasicParser p(Buf("10 PRINT 'Hello, World!'"));
    printf("%d\n", static_cast<int>(p.file().code()));
}
