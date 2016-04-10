// THING ====================================================

#include <type_traits>
template<typename T>
struct ThingHelper
{
    constexpr T value() const;
};
template<>
constexpr void ThingHelper<void>::value() const {}
template<typename T>
constexpr T ThingHelper<T>::value() const { return {}; }


template<char C, typename TAIL = void>
struct Thing
{
    constexpr char head() const { return C; }
    template<typename Y = TAIL>
        constexpr Y tail() const
        {
            return ThingHelper<Y>().value();
        }
    typedef TAIL tail_t;
    constexpr bool more() const { return std::is_void<TAIL>::value; }
};

// PRINTY ===================================================

#include <cstdio>
template<typename T>
struct printyHelper2
{
    constexpr void continuee() const;
};
template<bool more> void printyHelper(char);
template<typename T, bool more = true>
void printy(T t)
{
    printyHelper<t.more()>(t.head());
    printyHelper2<typename T::tail_t>().continuee();
}

template<>
constexpr void printyHelper2<void>::continuee() const {}
template<typename T>
constexpr void printyHelper2<T>::continuee() const
{
    T t;
    printy<T, t.more()>(t);
}

template<>
void printyHelper<true>(char c)
{
    printf("%c", c);
}
template<>
void printyHelper<false>(char c)
{
    printf("%c\n", c);
}

int main()
{
    Thing<'a', Thing<'b', Thing<'c'>>> thing;
    printy(thing);
}
