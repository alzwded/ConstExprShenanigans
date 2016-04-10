#include <cstdio>
#define REL_OPS 0

#define OR1(X) \
        .Or([this](){return (X);})
#define AND1(X) \
        .And([this](){return (X);})
#define OR0(X) \
        .Or([=]()mutable{return (X);})
#define AND0(X) \
        .And([=]()mutable{return (X);})

struct A
{
    bool fail;
    A(bool a = false) : fail(a) {}
    operator bool() { printf("good: %d\n", !fail); return !fail; }
#if REL_OPS
    A operator||(A o)
    {
        if(fail) return o;
        return *this;
    }
    A operator&&(A o)
    {
        if(fail) return *this;
        return o;
    }
#endif
    A f1()
    {
        printf("f1\n");
        if(fail) return *this;
        return {true};
    }
    A f2()
    {
        printf("f2\n");
        if(fail) return *this;
        return {false};
    }
    A f3()
    {
        printf("f3\n");
        if(fail) return *this;
        return {false};
    }
    A entry()
    {
        printf("entry\n");
        if(fail) return *this;
        auto& me = *this;
#if REL_OPS
        return me.f1().f2().f3()
            || me.f2().f3();
#else
        //return f1().f2().f3().Or([this](){return f2().f3();});
        return f1().f2().f3()
            OR0(me.f1().f2().f3())
            OR1(f2().f3())
            OR1(f1().f2().f3());
#if 0
        auto b1 = me.f1().f2().f3();
        if(b1) return b1;
        return me.f2().f3();
#endif
#endif
    }
    A entry2()
    {
        printf("entry2\n");
        if(fail) return *this;
        auto& me = *this;
#if REL_OPS
        return me.f2().f3()
            || me.f1().f2().f3();
#else
        //return f2().f3().Or([this](){return f1().f2().f3();});
        return me.f2().f3()
            OR0(me.f1().f2().f3());
#if 0
        auto b1 = me.f2().f3();
        if(b1) return b1;
        return me.f1().f2().f3();
#endif
#endif
    }
    A main()
    {
        printf("main\n");
        if(fail) return *this;
#if REL_OPS
        return this->entry()
            || this->entry2();
#else
        return entry().Or([this](){ return entry2(); });
#if 0
        auto b1 = entry();
        if(b1) return b1;
        auto b2 = entry2();
        if(b2) return b2;
        return {true};
#endif
#endif
    }
    template<typename F>
        A Or(F f)
        {
            if(fail) return f();
            return *this;
        }
    template<typename F>
        A And(F f)
        {
            if(fail) return *this;
            return f();
        }
    A failing()
    {
        printf("failing\n");
        if(fail) return *this;
        return this->f1().f2().f3().Or([this]() { return f1().f2().f3(); });
            /*
        return this->f1().f2().f3()
#if REL_OPS
            || this->f1().f2().f3()
#endif
            ;
            */
    }
    A passing()
    {
        printf("passing\n");
        if(fail) return *this;
        return this->f2().f3().Or([this]() { return f2().f3(); });
    }
};

int main()
{
    A gigel;
    auto r = gigel.entry();
    printf("%s\n\n", (bool)r?"pass":"fail");
    auto r2 = gigel.entry2();
    printf("%s\n\n", (bool)r2?"pass":"fail");
    auto m = gigel.main();
    printf("%s\n\n", (bool)m?"pass":"fail");
    auto f = gigel.failing();
    printf("%s\n\n", (bool)f?"pass":"fail");
    auto p = gigel.passing();
    printf("%s\n\n", (bool)p?"pass":"fail");
}
