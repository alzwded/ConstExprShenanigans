#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <cstdio>

inline void Execute(TinyBasicProgram prg)
{
    printf("%40svalid program:\n%s\n", TEST_NAME, prg.code);
}

#endif
