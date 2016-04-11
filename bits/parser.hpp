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
#ifndef PARSER_HPP
#define PARSER_HPP

#ifndef CONSTEXPR
# define CONSTEXPR constexpr
#endif

#ifndef TRACE
# define TRACE(F, ...) 
#endif

#ifndef DTRACE
# define DTRACE(F, ...)
#endif

namespace Jak {

struct TinyBasicParser
{
    Code const code_;
    int const line_;
    Buf buf_;

    explicit CONSTEXPR TinyBasicParser(Buf const buf)
        : code_(Code::InternalError)
          , line_(1)
          , buf_(buf)
    {}

    CONSTEXPR TinyBasicParser(Code const code, int const line, Buf const buf)
        : code_(code)
          , line_(line)
          , buf_(buf)
    {}

    CONSTEXPR TinyBasicParser(TinyBasicParser&& p)
        : code_(p.code_)
          , line_(p.line_)
          , buf_(p.buf_)
    {}

    CONSTEXPR TinyBasicParser(TinyBasicParser const& p)
        : code_(p.code_)
          , line_(p.line_)
          , buf_(p.buf_)
    {}

    CONSTEXPR Code code() const { return code_; }
    CONSTEXPR int lineNo() const { return line_; }
    CONSTEXPR Buf buf() const { return buf_; }

    CONSTEXPR TinyBasicParser operator||(TinyBasicParser const p) const
    {
        if(failed()) {
            TRACE("||: This " PFMT " failed, returning " PFMT "\n", P(*this), P(p));
            return p;
        }
        TRACE("||: This " PFMT " succeeded, ignoring " PFMT "\n", P(*this), P(p));
        return *this;
    }

    CONSTEXPR TinyBasicParser file() const
    {
        if(failed()) {
            DTRACE("file(): returning immediately\n");
            return *this;
        }
        if(buf_.empty()) {
            DTRACE("file(): empty buffer, returning OK\n");
            return {Code::Okay, line_, buf_};
        }
        DTRACE("file(): recursing\n");
        return line().file();
    }

private:

    CONSTEXPR bool failed() const
    {
        switch(code_)
        {
        case Code::Okay:
        case Code::InternalError:
            return false;
        default:
            return true;
        }
    }

    CONSTEXPR bool good() const
    {
        return !failed();
    }

    CONSTEXPR TinyBasicParser line() const
    {
        if(failed()) {
            DTRACE("line(): failed state, immediately returning\n");
            return *this;
        }

        DTRACE("line(): trying everything\n");
        auto ret = number().statement().cr()
            || statement().cr()
            ;
        DTRACE("line(): got " PFMT "\n", P(ret));
        return ret;
    }

    CONSTEXPR TinyBasicParser number_helper() const
    {
        if(buf_.empty()) {
            DTRACE("number_helper(): end of file, leaving it to cr()\n");
            return *this;
        }
        switch(buf_.head()) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                DTRACE("number_helper(): digit, recursing\n");
                return TinyBasicParser{code_, line_, buf_.tail()}.number_helper();
            default:
                DTRACE("number_helper(): ended\n");
                return *this;
        }
    }

    CONSTEXPR TinyBasicParser number() const
    {
        if(buf_.empty()) {
            DTRACE("number(): unexpected end of file\n");
            return {Code::UnexpectedEndOfFile, line_, buf_};
        }
        if(failed()) {
            DTRACE("number(): failed state, immediately return\n");
            return *this;
        }

        Buf b {buf_.tail()};
        switch(buf_.head())
        {
        case ' ':
        case '\t':
            DTRACE("number(): skipping whitespace\n");
            return TinyBasicParser{code_, line_, buf_.tail()}.number();
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            break;
        default:
            DTRACE("number(): expecting a number\n");
            return {Code::ExpectingANumber, line_, buf_};
        }

        DTRACE("number(): got a digit, entering helper\n");
        return TinyBasicParser{code_, line_, buf_.tail()}.number_helper();
    }

    CONSTEXPR TinyBasicParser cr() const
    {
        if(failed()) {
            DTRACE("cr(): failed state, immediately return\n");
            return *this;
        }
        if(buf_.empty()) {
            DTRACE("cr(): end of file reached, considering it as final CR\n");
            return *this;
        }

        switch(buf_.head())
        {
        case ' ':
        case '\t':
            DTRACE("cr(): skipping whitespace\n");
            return TinyBasicParser{code_, line_, buf_.tail()}.cr();
        case '\n':
            DTRACE("cr(): got CR\n");
            return {code_, line_ + 1, buf_.tail()};
        default:
            DTRACE("cr(): got something else, error out\n");
            return {Code::ExpectingEndOfLine, line_, buf_};
        }
    }

    CONSTEXPR TinyBasicParser statement() const
    {
        if(buf_.empty()) {
            DTRACE("statement(): unexpected end of file\n");
            return {Code::UnexpectedEndOfFile, line_, buf_};
        }
        if(failed()) {
            DTRACE("statement(): fail state, immediately returning\n");
            return *this;
        }
        
        DTRACE("statement(): trying everything\n");

        auto ret = literal("PRINT").expr_list()
            || literal("DATA").expr_list()
            || literal("IF").expression().relop().expression().literal("THEN").statement()
            || literal("GOTO").expression()
            || literal("INPUT").var_list()
            || literal("LET").var().literal("=").expression()
            || literal("GOSUB").expression()
            || literal("RETURN")
            || literal("CLEAR")
            || literal("LIST")
            || literal("RUN")
            || literal("END")
            ;

        DTRACE("statement(): got " PFMT "\n", P(ret));

        return ret;
    }

    CONSTEXPR TinyBasicParser literal(char const* s) const
    {
        if(buf_.empty()) {
            DTRACE("literal(%s): unexpected end of file, immediately returning\n", s);
            return {Code::UnexpectedEndOfFile, line_, buf_};
        }
        if(failed()) {
            DTRACE("literal(%s): fail state, immediately returning\n", s);
            return *this;
        }

        switch(buf_.head()) {
        case ' ':
        case '\t':
            DTRACE("literal(%s): skipping whitespace\n", s);
            return TinyBasicParser{code_, line_, buf_.tail()}.literal(s);
        }
        if(*s == '\0') {
            DTRACE("literal($): done\n");
            return *this;
        }
        if(*s != buf_.head()) {
            DTRACE("literal(%s): mismatch found\n", s);
            return {Code::UnknownKeyword, line_, buf_};
        }
        DTRACE("literal(%s): recursing\n", s);
        return TinyBasicParser{code_, line_, buf_.tail()}.literal(s + 1);
    }

    CONSTEXPR TinyBasicParser relop() const
    {
        if(buf_.empty()) {
            DTRACE("relop(): unexpected end of file, immediately, returning\n");
            return {Code::UnexpectedEndOfFile, line_, buf_};
        }
        if(failed()) {
            DTRACE("relop(): fail state, immediately returning\n");
            return *this;
        }

        switch(buf_.head()) {
        case ' ':
        case '\t':
            DTRACE("relop(): skipping whitespace\n");
            return TinyBasicParser{code_, line_, buf_.tail()}.relop();
        case '<':
        case '>':
            DTRACE("relop(): checking for composed\n");
            switch(buf_.tail().head()) {
            case '<':
            case '>':
            case '=':
                DTRACE("relop(): got composed\n");
                return {code_, line_, buf_.tail().tail()};
            default:
                DTRACE("relop(): got single\n");
                return {code_, line_, buf_.tail()};

            }
        case '=':
            DTRACE("relop(): got single\n");
            return {code_, line_, buf_.tail()};
        default:
            DTRACE("relop(): error\n");
            return {Code::ExpectingRelationalOperator, line_, buf_};
        }
    }

    CONSTEXPR TinyBasicParser var() const
    {
        if(buf_.empty()) {
            DTRACE("var(): unexpected end of file, immediately returning\n");
            return {Code::UnexpectedEndOfFile, line_, buf_};
        }
        if(failed()) {
            DTRACE("var(): fail state, immediately returning\n");
            return *this;
        }

        switch(buf_.head())
        {
        case ' ':
        case '\t':
            DTRACE("var(): skipping whitespace\n");
            return TinyBasicParser{code_, line_, buf_.tail()}.var();
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
        case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
        case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
        case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
        case 'Y': case 'Z':
            DTRACE("var(): got variable\n");
            return {code_, line_, buf_.tail()};
        default:
            DTRACE("var(): error\n");
            return {Code::ExpectingAVariable, line_, buf_};
        }
    }

    CONSTEXPR TinyBasicParser var_list_helper() const
    {
        if(buf_.empty()) {
            DTRACE("var_list_helper(): end of file, leaving it to cr()\n");
            return *this;
        }
        if(literal(",").good()) {
            DTRACE("var_list_helper(): got a comma, continuing\n");
            auto next = literal(",").var();
            DTRACE("var_list_helper(): next was " PFMT "\n", P(next));
            if(next.failed()) return next;
            DTRACE("var_list_helper(): continuing\n");
            return next.var_list_helper();
        }
        DTRACE("var_list_helper(): no comma, quitting\n");
        return *this;
    }

    CONSTEXPR TinyBasicParser var_list() const
    {
        if(buf_.empty()) {
            DTRACE("var_list(): unexpected end of file, immediately returning\n");
            return {Code::UnexpectedEndOfFile, line_, buf_};
        }
        if(failed()) {
            DTRACE("var_list(): fail state, immediately returning\n");
            return *this;
        }

        TinyBasicParser next = var();
        DTRACE("var_list(): next is " PFMT "\n", P(next));
        if(next.failed()) return next;
        DTRACE("var_list(): entering helper\n");
        return next.var_list_helper();
    }

    CONSTEXPR TinyBasicParser expression_helper() const
    {
        if(failed()) {
            DTRACE("expression_helper(): fail state, immediately returning\n");
            return *this;
        }
        if(buf_.empty()) {
            DTRACE("expression_helper(): end of file, leaving it to cr()\n");
            return *this;
        }
        DTRACE("expression_helper(): trying + or -\n");
        auto next = literal("+") || literal("-");
        DTRACE("got " PFMT "\n", P(next));
        if(next.failed()) return *this;
        DTRACE("expression_helper(): trying term\n");
        auto nextnext = next.term();
        DTRACE("got " PFMT "\n", P(nextnext));
        if(nextnext.failed()) return nextnext;
        DTRACE("recursing\n");
        return nextnext.expression_helper();
    }

    CONSTEXPR TinyBasicParser expression() const
    {
        if(buf_.empty()) {
            DTRACE("expression(): unexpected end of file, immediately returning\n");
            return {Code::UnexpectedEndOfFile, line_, buf_};
        }
        if(failed()) {
            DTRACE("expression(): fail state, immediately returning\n");
            return *this;
        }

        switch(buf_.head())
        {
        case ' ':
        case '\t':
            DTRACE("expression(): skipping whitespace\n");
            return TinyBasicParser{code_, line_, buf_.tail()}.expression();
        }

        DTRACE("expression(): trying many things\n");
        TinyBasicParser next = literal("+").term()
            || literal("-").term()
            || term()
            ;
        DTRACE("expression(): got " PFMT "\n", P(next));
        if(next.failed()) return next;
        DTRACE("expression(): entering helper\n");
        return next.expression_helper();
    }

    CONSTEXPR TinyBasicParser term_helper() const
    {
        if(buf_.empty()) {
            DTRACE("term_helper(): end of file, leaing it to cr()\n");
            return *this;
        }
        DTRACE("term_helper(): trying * or /\n");
        auto next = literal("*") || literal("/");
        DTRACE("term_helper(): got " PFMT "\n", P(next));
        if(next.failed()) return *this;
        DTRACE("term_helper(): trying factor\n");
        auto nextnext = next.factor();
        DTRACE("term_helper(): got " PFMT "\n", P(nextnext));
        if(nextnext.failed()) return nextnext;
        DTRACE("term_helper(): recursing\n");
        return nextnext.term_helper();
    }

    CONSTEXPR TinyBasicParser term() const
    {
        if(buf_.empty()) {
            DTRACE("term(): unexpected end of file, immediately returning\n");
            return {Code::UnexpectedEndOfFile, line_, buf_};
        }
        if(failed()) {
            DTRACE("term(): fail state, immediately returning\n");
            return *this;
        }

        switch(buf_.head())
        {
        case ' ':
        case '\t':
            DTRACE("term(): skipping whitespace\n");
            return TinyBasicParser{code_, line_, buf_.tail()}.term();
        }

        DTRACE("term(): trying factor\n");
        TinyBasicParser next = factor();
        DTRACE("term(): got " PFMT "\n", P(next));
        if(next.failed()) return next;
        DTRACE("term(): entering helper\n");
        return next.term_helper();
    }

    CONSTEXPR TinyBasicParser factor() const
    {
        if(buf_.empty()) {
            DTRACE("factor(): unexpected end of file, immediately returning\n");
            return {Code::UnexpectedEndOfFile, line_, buf_};
        }
        if(failed()) {
            DTRACE("factor(): fail state, immediately returning\n");
            return *this;
        }

        switch(buf_.head())
        {
        case ' ':
        case '\t':
            DTRACE("factor(): skipping whitespace\n");
            return TinyBasicParser{code_, line_, buf_.tail()}.factor();
        }

        DTRACE("factor(): trying many things\n");
        auto ret = var()
            || number()
            || literal("(").expression().literal(")")
            ;
        DTRACE("factor(): got " PFMT "\n", P(ret));
        return ret;
    }

    CONSTEXPR TinyBasicParser expr_list_helper() const
    {
        if(buf_.empty()) {
            DTRACE("expr_list_helper(): unexpected end of file, leaving it to cr()\n");
            return *this;
        }
        DTRACE("expr_list_helper(): trying comma\n");
        auto next = literal(",");
        DTRACE("expr_list_helper(): got " PFMT "\n", P(next));
        if(next.failed()) return *this;
        DTRACE("expr_list_helper(): trying string or expression\n");
        auto nextnext = next.string()
            || next.expression()
            ;
        DTRACE("expr_list_helper(): got " PFMT "\n", P(nextnext));
        if(nextnext.failed()) return nextnext;
        DTRACE("expr_list_helper(): recursing\n");
        return nextnext.expr_list_helper();
    }

    CONSTEXPR TinyBasicParser expr_list() const
    {
        if(buf_.empty()) {
            DTRACE("expr_list(): unexpected end of file, immediately returning\n");
            return {Code::UnexpectedEndOfFile, line_, buf_};
        }
        if(failed()) {
            DTRACE("expr_list(): fail state, immediately returning\n");
            return *this;
        }
        
        switch(buf_.head())
        {
        case ' ':
        case '\t':
            DTRACE("expr_list(): skipping whitespace\n");
            return TinyBasicParser{code_, line_, buf_.tail()}.expr_list();
        }

        DTRACE("expr_list(): trying string or expression\n");
        TinyBasicParser next = string()
            || expression()
            ;
        DTRACE("expr_list(): got " PFMT "\n", P(next));
        if(next.failed()) return next;
        DTRACE("expr_list(): entering helper\n");
        return next.expr_list_helper();
    }

    CONSTEXPR TinyBasicParser string_helper() const
    {
        if(failed()) {
            DTRACE("string_helper(): fail state, immediately returning\n");
            return *this;
        }

        if(buf_.empty()) {
            DTRACE("string_helper(): runaway string\n");
            return {Code::RunawayString, line_, buf_};
        }
        switch(buf_.head()) {
        case '"':
        case '\'':
            DTRACE("string_helper(): end found\n");
            return {code_, line_, buf_.tail()};
        case '\n':
            DTRACE("string_helper(): EOL found\n");
            return TinyBasicParser{code_, line_ + 1, buf_.tail()}.string_helper();
        default:
            DTRACE("string_helper(): recursing\n");
            return TinyBasicParser{code_, line_, buf_.tail()}.string_helper();
        }
    }

    CONSTEXPR TinyBasicParser string() const
    {
        if(buf_.empty()) {
            DTRACE("string(): unexpected end of file, immediately returning\n");
            return {Code::UnexpectedEndOfFile, line_, buf_};
        }
        if(failed()) {
            DTRACE("string(): fail state, immediately returning\n");
            return *this;
        }
        
        switch(buf_.head())
        {
        case ' ':
        case '\t':
            DTRACE("string(): skipping whitespace\n");
            return TinyBasicParser{code_, line_, buf_.tail()}.string();
        case '"':
        case '\'':
            DTRACE("string(): head found\n");
            break;
        default:
            DTRACE("string(): no quotes found\n");
            return {Code::ExpectingQuotes, line_, buf_};
        }

        DTRACE("string(): entering helper\n");
        return TinyBasicParser{code_, line_, buf_.tail()}.string_helper();
    }
};

} // namespace Jak

#endif
