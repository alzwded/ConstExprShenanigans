#include <type_traits>
enum class Code {
    OK = 0,
    MISSING,
    MALFORMED
};

struct E_OK { constexpr E_OK(){} constexpr operator bool() const { return true; }};
struct E_MISSING { constexpr E_MISSING(){} constexpr operator bool() const { return false; }};
struct E_MALFORMED { constexpr E_MALFORMED(){} constexpr operator bool() const { return false; }};

template<Code C>
struct decode;

template<>
struct decode<Code::OK> { static constexpr E_OK validity(){return{};} };
template<>
struct decode<Code::MISSING> { static constexpr E_MISSING validity(){return{};} };
template<>
struct decode<Code::MALFORMED> { static constexpr E_MALFORMED validity(){return{};} };



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

template<typename error, int line>
void check2()
{
    static_assert(error(), "Compilation Error");
}

template<typename error, int line>
void check()
{
    auto constexpr error2 = error::validity();
    check2<decltype(error2), line>();
}

int main()
{
    constexpr Buf a1(Code::OK, 10);
    constexpr Buf a2(Code::MISSING, 11);
    constexpr Buf a3(Code::MALFORMED, 12);
    constexpr Buf a4(Code::MISSING, 23);

    check<decode<a1.GetCode()>, a1.GetLine()>();
    check<decode<a2.GetCode()>, a2.GetLine()>();
    check<decode<a3.GetCode()>, a3.GetLine()>();
    check<decode<a4.GetCode()>, a4.GetLine()>();
}
