#include <TinyBasicProgram.hpp>
#include <TestUtils.h>

int main()
{
#if 0
#define S "10 PRINT 'Hello, World!'\n\
            20 GOTO 10"


      Jak::SyntaxCheckHelper<
            (Jak::TinyBasicParser(Jak::Buf(S)).file()).code(),
            (Jak::TinyBasicParser(Jak::Buf(S)).file()).lineNo()
                > () ;



#else
    Execute(TinyBasic("\
            10 PRINT 'Hello, World!'\n\
            20 GOTO 10\n"));
#endif
}
