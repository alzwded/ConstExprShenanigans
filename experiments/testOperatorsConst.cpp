struct A
{
    bool const fail;
    constexpr A()
        : fail(false)
    {}
    constexpr A(bool b)
        : fail(b)
    {}
    constexpr A(A&& o)
        : fail(o.fail)
    {}
    constexpr A(A const& o)
        : fail(o.fail)
    {}
    constexpr operator bool() const { return !fail; }
    constexpr bool bad() const { return fail; }
    constexpr A operator||(A const o)
    {
        if(fail) return A(o);
        return A(*this);
    }
    constexpr A operator&&(A const o)
    {
        if(fail) return A(*this);
        return A(o);
    }
    constexpr A f1() const
    {
        if(fail) return A(*this);
        return {true};
    }
    constexpr A f2() const
    {
        if(fail) return A(*this);
        return {false};
    }
    constexpr A f3() const
    {
        if(fail) return A(*this);
        return {false};
    }
    constexpr A entry() const
    {
        if(fail) return A(*this);
        auto& me = *this;
        return me.f1().f2().f3()
            || me.f1().f2().f3()
            || me.f2().f3()
            || me.f1().f2().f3();
    }
    constexpr A entry2() const
    {
        if(fail) return A(*this);
        auto& me = *this;
        return me.f2().f3()
            || me.f1().f2().f3();
    }
    constexpr A main() const
    {
        if(fail) return A(*this);
        return this->entry()
            || this->entry2();
    }
    constexpr A failing() const
    {
        if(fail) return A(*this);
        return f1().f2().f3()
            || f1().f2().f3();
    }
};

static_assert(A().main(), "bad to fail");

/* strategy: */

template<int I>
struct B
{
    constexpr B()
    {
        static_assert(valid(I), "not valid number");
    }

    static constexpr A valid(int i)
    {
        if(i > 0) return valid(i - 2);
        if(i < 0) return {false};
        return {true};
    }
};

int main()
{
    static_assert(A().entry(), "bad to fail");
    static_assert(A().failing(), "OK");
    static_assert(A().main(), "bad to fail");
    static_assert(A().f2() && A().f3(), "bad to fail");
    static_assert(A().f1() && A().f2(), "OK");

    B<4> b4;
    B<3> b3;
}
