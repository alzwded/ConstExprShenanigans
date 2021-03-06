#include <cstdio>
#include <functional>
#include <map>

// available types
enum class TypeEnum : int
{
    BOOL,
    INT,
    DOUBLE,
    FAIL
};

// translate type enum to actual type
template<TypeEnum e> struct TypeOfTypeEnum;

template<> struct TypeOfTypeEnum<TypeEnum::BOOL>
{
    typedef bool type;
};

template<> struct TypeOfTypeEnum<TypeEnum::INT>
{
    typedef int type;
};

template<> struct TypeOfTypeEnum<TypeEnum::DOUBLE>
{
    typedef double type;
};

template<> struct TypeOfTypeEnum<TypeEnum::FAIL>
{
    typedef char const* type;
};

// Our variant type
struct Cont
{
    Cont(bool b)
        : type(TypeEnum::BOOL)
          , asBool(b)
    {}
    Cont(int i)
        : type(TypeEnum::INT)
          , asInt(i)
    {}
    Cont(double d)
        : type(TypeEnum::DOUBLE)
          , asDouble(d)
    {}

    TypeEnum type;
    union
    {
        bool asBool;
        int asInt;
        double asDouble;
    };
};

// read correct data based on type enum
template<TypeEnum e, typename T = typename TypeOfTypeEnum<e>::type>
 constexpr T const& GetMemberByType(Cont const& c);

template<>
constexpr bool const& GetMemberByType<TypeEnum::BOOL>(Cont const& c)
{
    return c.asBool;
}
template<>
constexpr int const& GetMemberByType<TypeEnum::INT>(Cont const& c)
{
    return c.asInt;
}
template<>
constexpr double const& GetMemberByType<TypeEnum::DOUBLE>(Cont const& c)
{
    return c.asDouble;
}

template<typename... Args>
struct UnsupportedCall
{
    void operator()(Cont const&, Args...) {}
};

// utility template to map the correct template instantiation to the correct type enum type type type
template<template<TypeEnum, typename... Args> class T, typename... MoreArgs>
std::map<TypeEnum, std::function<void(Cont const&, MoreArgs...)>> const& GenerateMap()
{
    static decltype(GenerateMap<T, MoreArgs...>()) rval {
        { TypeEnum::BOOL, &T<TypeEnum::BOOL>::template fn<MoreArgs...> },
        { TypeEnum::INT, &T<TypeEnum::INT>::template fn<MoreArgs...> },
        { TypeEnum::DOUBLE, &T<TypeEnum::DOUBLE>::template fn<MoreArgs...> },
        { TypeEnum::FAIL, UnsupportedCall<MoreArgs...>() }
    };
    return rval;
}

// utility to generate a dispatcher for a functor
template<template<typename T> class FN>
struct DispatchOf
{
    template<TypeEnum e>
    struct type
    {
        template<typename... Args>
        static void fn(Cont const& c, Args&&... args)
        {
            return FN<typename TypeOfTypeEnum<e>::type>()(GetMemberByType<e>(c), args...);
        };
    };
};

// utility to actually call the potato
template<template<typename F> class FN, typename... Args> void CallGenericFunctor(Cont const& c, Args&&... args)
{
    auto&& fn = GenerateMap<DispatchOf<FN>::template type, Args...>().at(c.type);
    return fn(c, args...);
}

// your processing functor
template<typename T>
struct Printer
{
    void operator()(T const& t, int&);
};

template<>
void Printer<bool>::operator()(bool const& t, int& myState)
{
    printf("%d: bool: %s\n", myState++, t ? "true" : "false");
}

template<>
void Printer<int>::operator()(int const& t, int& myState)
{
    printf("%d: int: %d\n", myState++, t);
}
template<>
void Printer<double>::operator()(double const& t, int& myState)
{
    printf("%d: double: %lg\n", myState++, t);
}

// another processor
#include <vector>
template<typename T>
struct Squarer
{
    void operator()(T const&, std::vector<Cont>&);
};

template<>
void Squarer<int>::operator()(int const& t, std::vector<Cont>& myStash)
{
    myStash.push_back(t * t);
}

template<>
void Squarer<double>::operator()(double const& t, std::vector<Cont>& myStash)
{
    myStash.push_back(t * t);
}

template<typename T>
void Squarer<T>::operator()(T const& t, std::vector<Cont>& myStash)
{
    myStash.push_back(t);
}

#include <algorithm>
int main(int argc, char* argv[])
{
    std::vector<Cont> conts {
        2,
        3.0,
        4,
        true,
        42,
        3.14
    };
    std::vector<Cont> squared;
    for(auto&& c : conts)
    {
        CallGenericFunctor<Squarer>(c, squared);
    }
    int lineNo = 1;
    auto p = [&lineNo](Cont const& c) {
        CallGenericFunctor<Printer>(c, lineNo);
    };
    std::for_each(squared.begin(), squared.end(), p);
}
