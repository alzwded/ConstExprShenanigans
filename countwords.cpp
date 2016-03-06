#if 0
goal: cound words
compile with g++ --std=gnu++14 countwords.cpp
#endif
#include <cstdio>

struct Buffer {
    char const* s_;
    unsigned len_;

    template<unsigned N>
    constexpr Buffer(const char(&a)[N])
        : s_(a)
        , len_(N-1)
    {
        static_assert(N >= 1, "not a string literal");
    }

    constexpr Buffer(const char* s, unsigned len)
        : s_(s)
        , len_(len)
    {}

    constexpr Buffer tail() const
    {
        return Buffer(s_+1, len_-1);
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
};


struct WordCounter
{
    Buffer b;
    int state;
    template<unsigned N>
        constexpr
        WordCounter(const char (&text)[N])
        : b(text)
          , state(0)
    {
        static_assert(N >= 1, "not a string");
    }

    constexpr
    WordCounter(Buffer buf, int sta)
    : b(buf)
      , state(sta)
    {}

    constexpr char const* text() const
    {
        return b.text();
    }

    constexpr int count() const
    {
        switch(state)
        {
            default:
            case -1: return 0;
            case 0: if(b.len() == 0) return 1;//return WordCounter(b, -1).count();
                        else if(b.head() == ' ') return 1 + WordCounter(b.tail(), 1).count();
                        else return WordCounter(b.tail(), 0).count();
            case 1: if(b.len() == 0) return 0; //return WordCounter(b, -1).count();
                        else if(b.head() != ' ') return WordCounter(b.tail(), 0).count();
                        else return WordCounter(b.tail(), 1).count();
        }

    }
};

int main()
{
    constexpr WordCounter wc("word  word ");
    static_assert(wc.count() == 2, "need two words");
    printf("%s\n", wc.text());

    constexpr WordCounter wc2("word  word word");
    static_assert(wc2.count() == 2, "need two words");
    printf("%s\n", wc2.text());
}
