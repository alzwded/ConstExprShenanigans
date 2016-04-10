/* *******************************************************
   Copyright (c) 2016, Vlad Meșco
   All rights reserved.
   
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
   
   * Redistributions of source code must retain the above copyright notice, this
     list of conditions and the following disclaimer.
   
   * Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.
   
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
   FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
   CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
   OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
   ******************************************************* */
#ifndef VALIDATE_HPP
#define VALIDATE_HPP

namespace Jak {

#define JAK_ERR(NAME, ISOK)\
    struct E_##NAME {\
        constexpr E_##NAME() {}\
        constexpr operator bool() const { return ISOK; }\
    };\
    template<>\
    struct decode<Code::NAME> { typedef E_##NAME type; }

enum class Code {
    Okay = 0,
    InternalError = 1,
    ExpectingANumber,
    ExpectingEndOfLine,
    UnknownKeyword,
    ExpectingRelationalOperator,
    ExpectingAVariable,
    ExpectingQuotes,
    RunawayString,
    UnexpectedEndOfFile,
    TODO_remove_me
};

template<Code C>
struct decode;

JAK_ERR(Okay, true);
JAK_ERR(InternalError, false);
JAK_ERR(ExpectingANumber, false);
JAK_ERR(ExpectingEndOfLine, false);
JAK_ERR(UnknownKeyword, false);
JAK_ERR(ExpectingRelationalOperator, false);
JAK_ERR(ExpectingAVariable, false);
JAK_ERR(ExpectingQuotes, false);
JAK_ERR(RunawayString, false);
JAK_ERR(UnexpectedEndOfFile, false);

#undef JAK_ERR

template<typename error, int line>
constexpr void SyntaxCheck()
{
    static_assert(error(), "Syntax Error");
}

template<Code C, int line>
constexpr void SyntaxCheckHelper()
{
    SyntaxCheck<typename decode<C>::type, line>();
}

} // namespace Jak

#endif
