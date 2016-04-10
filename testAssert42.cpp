enum class Code {
    OK = 0,
    MISSING,
    MALFORMED
};

struct Buf {
    Code const code;
    int line;

    constexpr Buf(Code c, int l)
        : code(c)
          , line(l)
    {}

    constexpr Code GetCode() const { return code; }
    constexpr int GetLine() const { return line; }
};

template<Code error, int line>
void check()
{
    static_assert(error == Code::OK, "Compilation failed");
}

int main()
{
    constexpr Buf a1(Code::OK, 10);
    constexpr Buf a2(Code::MISSING, 11);
    constexpr Buf a3(Code::MALFORMED, 12);

    check<a1.GetCode(), a1.GetLine()>();
    check<a2.GetCode(), a2.GetLine()>();
    check<a3.GetCode(), a3.GetLine()>();
}
