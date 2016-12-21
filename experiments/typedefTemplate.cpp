#include <cstdio>
#include <functional>
#include <map>

// available types
enum class TypeEnum : int
{
    BOOL,
    INT,
    DOUBLE
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

// utility template to map the correct template instantiation to the correct type enum type type type
template<template<TypeEnum> typename T>
std::map<TypeEnum, std::function<void(Cont const&)>> const& GenerateMap()
{
    static decltype(GenerateMap<T>()) rval {
        { TypeEnum::BOOL, &T<TypeEnum::BOOL>::fn },
        { TypeEnum::INT, &T<TypeEnum::INT>::fn },
        { TypeEnum::DOUBLE, &T<TypeEnum::DOUBLE>::fn }
    };
    return rval;
}

// utility to generate a dispatcher for a functor
template<template<typename T> typename FN>
struct DispatchOf
{
    template<TypeEnum e>
    struct type
    {
        static void fn(Cont const& c)
        {
            return FN<typename TypeOfTypeEnum<e>::type>()(GetMemberByType<e>(c));
        };
    };
};

// utility to actually call the potato
template<template<typename F> typename FN> void CallGenericFunctor(Cont const& c)
{
    return GenerateMap<DispatchOf<Printer>::type>().at(c.type)(c);
}

// your processing functor
template<typename T>
struct Printer
{
    void operator()(T const& t);
};

template<>
void Printer<bool>::operator()(bool const& t)
{
    printf("bool: %s\n", t ? "true" : "false");
}

template<>
void Printer<int>::operator()(int const& t)
{
    printf("int: %d\n", t);
}
template<>
void Printer<double>::operator()(double const& t)
{
    printf("double: %lg\n", t);
}

int main(int argc, char* argv[])
{
    Cont conts[] = {
        true,
        42,
        3.14
    };
    for(size_t i = 0; i < 3; ++i)
    {
        CallGenericFunctor<Printer>(conts[i]);
    }
}
