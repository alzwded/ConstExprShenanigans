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
#ifndef BUFFER_HPP
#define BUFFER_HPP

namespace Jak {

struct Buf
{
    char const* const s_;
    unsigned len_;

    template<unsigned N>
        constexpr Buf(const char(&s)[N])
        : s_(s)
          , len_(N-1)
    {
        static_assert(N >= 1, "not a string literal");
    }

    constexpr Buf(Buf&& b)
        : s_(b.s_)
          , len_(b.len_)
    {}

    constexpr Buf(Buf const& b)
        : s_(b.s_)
          , len_(b.len_)
    {}

    constexpr Buf(char const* s, unsigned len)
        : s_(s)
          , len_(len)
    {}

    constexpr Buf tail() const
    {
        return {s_ + 1, len_ - 1};
    }

    constexpr char head() const
    {
        return *s_;
    }

    constexpr unsigned len() const
    {
        return len_;
    }

    constexpr char const* text() const
    {
        return s_;
    }

    constexpr bool empty() const
    {
        return *s_ == '\0';
    }
};

} // namespace Jak

#endif
