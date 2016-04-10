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

namespace Jak {

struct TinyBasicParser
{
    Code const code_;
    int const line_;
    Buf buf_;

    explicit constexpr TinyBasicParser(Buf const buf)
        : code_(Code::InternalError)
          , line_(-1)
          , buf_(buf)
    {}

    constexpr TinyBasicParser(Code const code, int const line, Buf const buf)
        : code_(Code::Okay)
          , line_(line)
          , buf_(buf)
    {}

    constexpr TinyBasicParser(TinyBasicParser&& p)
        : code_(p.code_)
          , line_(p.line_)
          , buf_(p.buf_)
    {}

    constexpr TinyBasicParser(TinyBasicParser const& p)
        : code_(p.code_)
          , line_(p.line_)
          , buf_(p.buf_)
    {}

    constexpr Code code() const { return code_; }
    constexpr int lineNo() const { return line_; }
    constexpr Buf buf() const { return buf_; }

    constexpr TinyBasicParser operator||(TinyBasicParser const p) const
    {
        if(failed()) return p;
        return *this;
    }

    constexpr TinyBasicParser file() const
    {
        if(buf_.empty()) return {Code::Okay, line_, buf_};
        if(failed()) return *this;
        return line().file();
    }

private:

    constexpr bool failed() const
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

    constexpr bool good() const
    {
        return !failed();
    }

    constexpr TinyBasicParser line() const
    {
        if(failed()) return *this;

        return number().statement().cr()
            || statement().cr()
            ;
    }

    constexpr TinyBasicParser number() const
    {
        if(failed()) return *this;

        Buf b {buf_.tail()};
        switch(buf_.head())
        {
        case ' ':
        case '\t':
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
            return {Code::ExpectingANumber, line_, buf_};
        }

        while(1) {
            switch(b.head()) {
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
                b = b.tail();
            default:
                return {code_, line_, b};
            }
        }
    }

    constexpr TinyBasicParser cr() const
    {
        if(failed()) return *this;
        if(buf_.empty) return *this;

        switch(buf_.head())
        {
        case ' ':
        case '\t':
            return {code_, line_, buf_.tail()}.cr();
        case '\n':
            return {code_, line_ + 1, buf_.tail()};
        default:
            return {Code::ExpectingEndOfLine, line_, buf_};
        }
    }

    constexpr TinyBasicParser statement() const
    {
        if(failed()) return *this;

        return literal("PRINT").expr_list()
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
    }

    constexpr TinyBasicParser literal(char const* s) const
    {
        if(failed()) return *this;

        switch(buf_.head()) {
        case ' ':
        case '\t':
            return {code_, line_, buf_.tail()}.literal(s);
        }
        if(*s == '\0') return *this;
        if(*s != buf_.head()) return {Code::UnknownKeyword, line_, buf_};
        return {code_, line_, buf_.tail()}.literal(s + 1);
    }

    constexpr TinyBasicParser relop() const
    {
        if(failed()) return *this;

        switch(buf_.head()) {
        case ' ':
        case '\t':
            return {code_, line_, buf_.tail()}.relop();
        case '<':
        case '>':
            switch(buf_.tail().head()) {
            case '<':
            case '>':
            case '=':
                return {code_, line_, buf_.tail()};
            default:
                return *this;

            }
        case '=':
            return {code_, line_, buf_.tail()};
        default:
            return {Code::ExpectingRelationalOperator, line_, buf_};
        }
    }

    constexpr TinyBasicParser var() const
    {
        if(failed()) return *this;

        switch(buf_.head())
        {
        case ' ':
        case '\t':
            return {code_, line_, buf_.tail()}.var();
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
        case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
        case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
        case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
        case 'Y': case 'Z':
            return {code_, line_, buf_.tail()};
        default:
            return {Code::ExpectingAVariable, line_, buf_};
        }
    }

    constexpr TinyBasicParser var_list() const
    {
        if(failed()) return *this;

        TinyBasicParser next = var();
        if(next.failed()) return next;
        while(next.literal(",").good()) {
            next = next.literal(",").var();
            if(next.failed()) return next;
        }
        return next;
    }

    constexpr TinyBasicParser expression() const
    {
        if(failed()) return *this;

        switch(buf_.head())
        {
        case ' ':
        case '\t':
            return {code_, line_, buf_.tail()}.expression();
        }

        TinyBasicParser next = literal("+").term()
            || literal("-").term()
            || term()
            ;
        if(next.failed()) return next;
        while((next.literal("+") || next.literal("-")).good()) {
            next = (next.literal("+") || next.literal("-")).term();
            if(next.failed()) return next;
        }
        return next;
    }

    constexpr TinyBasicParser term() const
    {
        if(failed()) return *this;

        switch(buf_.head())
        {
        case ' ':
        case '\t':
            return {code_, line_, buf_.tail()}.term();
        }

        TinyBasicParser next = factor();
        if(next.failed()) return next;
        while((next.literal("*") || next.literal("/")).good()) {
            next = (next.literal("*") || next.literal("/")).factor();
            if(next.failed()) return next;
        }
        return next;
    }

    constexpr TinyBasicParser factor() const
    {
        if(failed()) return *this;

        switch(buf_.head())
        {
        case ' ':
        case '\t':
            return {code_, line_, buf_.tail()}.factor();
        }

        return var()
            || number()
            || literal("(").expression().literal(")")
            ;
    }

    constexpr TinyBasicParser expr_list() const
    {
        if(failed()) return *this;
        
        switch(buf_.head())
        {
        case ' ':
        case '\t':
            return {code_, line_, buf_.tail()}.expr_list();
        }

        TinyBasicParser next = string()
            || expression()
            ;
        if(next.failed()) return next;
        while(next.literal(",").good()) {
            next = next.literal(",");
            next = next.string()
                || next.expression()
                ;
            if(next.failed()) return next,
        }
        return next();
    }

    constexpr TinyBasicParser string() const
    {
        if(failed()) return *this;
        
        switch(buf_.head())
        {
        case ' ':
        case '\t':
            return {code_, line_, buf_.tail()}.string();
        case '"':
        case '\'':
            break;
        default:
            return {Code::ExpectingQuotes, line_, buf_};
        }

        Buf b = buf_.tail();
        int line = line_;
        while(1) {
            if(b.empty()) return {Code::RunawayString, line, buf_};
            switch(b.head()) {
            case '"':
            case '\'':
                return {code_, line_, b.tail()};
            case '\n':
                line++;
                /*FALLTHROUGH*/
            default:
                b = b.tail();
                break;
            }
        }
    }
};

} // namespace Jak

#endif
