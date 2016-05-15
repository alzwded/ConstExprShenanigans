#include <functional>
#include <deque>
#include <iostream>
#include <iomanip>
#include <string>

enum class Errors
{
    None,
    NoKeyword,
    InvalidVar,
    LongVar,
    TrailingChars,
    NotEvaluated
};

struct Result
{
    std::deque<char> in;
    Errors err;
    explicit operator bool() const
    {
        return err == Errors::None;
    }
    bool operator==(Errors e) const
    {
        return err == e;
    }

    Result(Errors e)
        : in()
          , err(e)
    {}
    Result(std::deque<char> i)
        : err(Errors::None)
          , in(i)
    {}
    Result(std::deque<char> i, Errors e)
        : err(e)
          , in(i)
    {}

    friend std::ostream& operator<<(std::ostream&, Result const&);
};

std::ostream& operator<<(std::ostream& fout, Result const& r)
{
    if(r) fout << "Success";
    else {
        fout << "Fail[";
        switch(r.err) {
            case Errors::NoKeyword: fout << "NoKeyword]"; break;
            case Errors::InvalidVar: fout << "InvalidVar]"; break;
            case Errors::LongVar: fout << "LongVar]"; break;
            case Errors::TrailingChars: fout << "TrailingChars]"; break;
            case Errors::NotEvaluated: fout << "NotEvaluated]"; break;
            default:
                throw std::invalid_argument("what");
        }
    }
}

struct Parser
{
    std::function<Parser(std::deque<char>)> cont;
    Result result;
    inline Parser operator()(std::deque<char> in) const
    {
        if(cont) return Lazy(cont(in));
        return *this;
    }

    Parser(Result r, std::function<Parser(std::deque<char>)> f = nullptr)
        : result(r)
          , cont(f)
    {}

private:
    static Parser Lazy(Parser p)
    {
        if(p.cont) return p(p.result.in);
        return p;
    };
};

template<typename P>
Parser BindParser(P p)
{
    return {
        Result(Errors::NotEvaluated),
        [p](std::deque<char> in) -> Result {
            return p(in);
        }
    };
}

Result Return(Parser p)
{
    return p.result;
}

Parser Any(Parser a)
{
    return {
        Result(Errors::NotEvaluated),
        [a](std::deque<char> in) -> Parser {
            auto left = a(in);
            if(Return(left)) {
                return {
                    Return(left),
                    Any(Parser{
                        Return(left),
                        a
                    })
                };
            } else {
                auto r = Return(a);
                if(r.err == Errors::NotEvaluated) return left;
                return r;
            }
        }
    };
}

Parser And(Parser a, Parser b)
{
    return {
        Result(Errors::NotEvaluated),
        [a, b](std::deque<char> in) -> Parser {
            auto left = Return(a(in));
            if(left) {
                return {
                    left,
                    [b](std::deque<char> in) -> Parser {
                        return b(in);
                    }
                };
            } else {
                return { left };
            }
        }
    };
}

Parser Or(Parser a, Parser b)
{
    return {
        Result(Errors::NotEvaluated),
        [a, b](std::deque<char> in) -> Parser {
            auto left = Return(a(in));
            if(left) {
                return { left };
            } else {
                return {
                    Result(in, Errors::NotEvaluated),
                    [b](std::deque<char> in) -> Parser {
                        return b(in);
                    }
                };
            }
        }
    };
}

/*
BindParser = [](P) -> [P](deque<char>) -> Result || Continue || Fail
    Success(P) -> Continue || Result
    Fail(P) -> Fail
Any = [P](deque<char>) -> Result || Continue
    Success(P) -> [P](deque<char>) -> Result || Continue
    Fail(P) -> Result
And = [A, B](deque<char>) -> Result || Continue || Fail
    Success(A) => [B](deque<char>) -> Result || Fail
    Fail(A) => Fail
Or = [A, B](deque<char>) -> Result || Continue || Fail
    Success(A) => Result
    Fail(A) => [B](deque<char>) -> Result || Fail
*/


Result let(std::deque<char> in)
{
    while(in.size() && in.front() == ' ') in.pop_front();
    if(in.empty()) return Result(Errors::NoKeyword);
    auto a = in.front(); in.pop_front();
    if(in.empty()) return Result(Errors::NoKeyword);
    auto b = in.front(); in.pop_front();
    if(in.empty()) return Result(Errors::NoKeyword);
    auto c = in.front(); in.pop_front();
    if(a == 'l' && b == 'e' && c == 't') return Result(in);
    return Result(Errors::NoKeyword);
}

Result eof(std::deque<char> in)
{
    while(in.size() && in.front() == ' ') in.pop_front();
    if(in.empty()) return Result(in);
    else return Result(Errors::TrailingChars);
}

Result var(std::deque<char> in)
{
    while(in.size() && in.front() == ' ') in.pop_front();
    if(in.empty()) return Result(Errors::NoKeyword);
    auto a = in.front(); in.pop_front();
    if(a < 'a' || a > 'z') return Result(Errors::InvalidVar);
    if(in.empty()) return Result(in);
    if(in.front() != ' ') return Result(Errors::LongVar);
    return Result(in);
}

int main()
{
    auto p = Any(
            Or(
                And(
                    And(
                        BindParser(let)
                        ,
                        Any(
                            BindParser(var))
                       )
                    ,
                    BindParser(eof))
                ,
                And(
                    BindParser(var)
                    ,
                    BindParser(eof)
                   )
              )
            );

    std::deque<std::pair<std::string, Errors>> tests {
        { "let a b c", Errors::None },
        { "let", Errors::LongVar },
        { "a", Errors::None },
        { "ab", Errors::LongVar },
        { "3", Errors::InvalidVar },
        { "let 3", Errors::LongVar },
        { "let ab c", Errors::LongVar },
        { "a b", Errors::TrailingChars }
    };

    std::cout << "Passed?  " << std::setw(10) << std::left << "input" << "\tReturn value" << std::setw(0) << std::endl;
    std::cout << std::string(60, '-') << std::endl;

    for(auto&& i : tests) {
        auto in = std::deque<char>(i.first.begin(), i.first.end());
        auto r = Return(p(in));
        if(r == i.second) {
            std::cout << "Pass:    " << std::setw(10) << std::left << i.first << "\t:: Returned: " << r << std::setw(0) << std::endl;
        } else {
            std::cout << "FAIL:    " << std::setw(10) << std::left << i.first << "\t:: Returned: " << r << std::setw(0) << " Expected: " << i.second << std::endl;
        }
    }
}
