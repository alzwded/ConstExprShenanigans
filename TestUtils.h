#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <cstdio>

#define QUOTE(X) #X
#define Q(X) QUOTE(X)

inline void Execute(TinyBasicProgram prg)
{
    printf("%s contains a valid program:\n%s\n", Q(TEST_NAME), prg.source);
}

#endif
