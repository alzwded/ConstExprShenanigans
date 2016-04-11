#include "buffer.hpp"
#include "validate.hpp"
#include "parser_rt.hpp"
#include <cstdio>
#ifdef _WIN32
# include <windows.h>
#endif

#ifndef TEST
# error "Please specify a test."
#endif

#define TESTCASE(S, C, L)\
    TinyBasicParser p(Buf(S));\
    auto refCode = C;\
    int refLine = L;\
    auto rv = p.file();\
    auto code = rv.code();\
    auto line = rv.lineNo();\
    auto source = p.buf().text()

using namespace Jak;
int main()
{
#if TEST == 1
    TESTCASE("10 PRINT 'Hello, World!'", Code::Okay, 1);
#elif TEST == 2
    TESTCASE("\
10 PRINT 'Hello, World!'\n\
20 GOTO 10",
    Code::Okay, 2);
#elif TEST == 3
    TESTCASE("\
10 PRINT 'Hello, World!'\n\
20 GOTO 10\n",
    Code::Okay, 3);
#elif TEST == 4
    TESTCASE("\
10 PIRNT 'Hello, World!'",
    Code::UnknownKeyword, 1);
#endif
#ifdef PRINT_INFO
    printf("source =\n%s$\n", source);
    printf("code = %d line = %d\n", static_cast<int>(code), line);
    printf("\n");
#endif
    auto pass = (code == refCode && line == refLine);
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    auto h = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(h, &csbi);
    if(pass) SetConsoleTextAttribute(h, FOREGROUND_GREEN|FOREGROUND_INTENSITY);
    else SetConsoleTextAttribute(h, FOREGROUND_RED|FOREGROUND_INTENSITY);
#endif
    printf("%s", pass ? "PASS" : "FAIL");
#ifdef _WIN32
    SetConsoleTextAttribute(h, csbi.wAttributes);
#endif
    printf("\n");
}
