#include <functional>
#include <deque>
#include <iostream>
#include <iomanip>
#include <string>

// Possible errors returned by parsers
enum class Errors
{
    None,
    NoKeyword,
    InvalidVar,
    LongVar,
    TrailingChars,
    NotEvaluated
};

// Result class
// also carries forward the state of the input buffer
struct Result
{
    std::deque<char> in; // this could be moved into Parser
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

// Generic parser type
struct Parser
{
    std::function<Parser(std::deque<char>)> cont; // continuation
    Result result; // return value

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
    // lazy evaluation expansion
    static Parser Lazy(Parser p)
    {
        // as long as it returns a continuation, continue evaluating
        if(p.cont) return p(p.result.in);
        return p;
    };
};

// Bind operation
// Takes a parser function as input and creates a Parser monad
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

// Return operation
// Takes a Parser monad and extracts its value
Result Return(Parser p)
{
    return p.result;
}

// Many operation
// Continues executing a Parser while it returns successfully
// Matches one or more instances of 'a'
Parser Many(Parser a)
{
    return {
        Result(Errors::NotEvaluated),
        [a](std::deque<char> in) -> Parser {
            auto left = a(in);
            if(Return(left)) {
                return {
                    Return(left),
                    Many(Parser{
                        Return(left),
                        a
                    })
                };
            } else {
                auto r = Return(a);
                // NotEvaluated represents the initial state
                // We want to match one or more, so if the first
                // analysis fails, return with *that* error
                if(r.err == Errors::NotEvaluated) return left;
                // else, return the previous state
                return r;
            }
        }
    };
}

// And operation
// Takes two parsers. If a fails, the result is Fail.
// Otherwise, both are executed and 'b' is returned.
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

// Or operation
// Takes two parsers. If 'a' succeeds, the result is 'a'
// Otherwise, the result is 'b'
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

// User-defined parser functions
// =============================

// match keyword 'let'
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

// match end-of-file
Result eof(std::deque<char> in)
{
    while(in.size() && in.front() == ' ') in.pop_front();
    if(in.empty()) return Result(in);
    else return Result(Errors::TrailingChars);
}

// match [a-z]
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
// =============================

int main()
{
    // build a parser
    auto p = Many(
            // Match either...
            Or(
                // 'let' followed by any number > 1 of single letters...
                And(
                    And(
                        BindParser(let)
                        ,
                        Many(
                            BindParser(var))
                       )
                    ,
                    BindParser(eof))
                , // ...or...
                // a single letter.
                And(
                    BindParser(var)
                    ,
                    BindParser(eof)
                   )
              )
            );

    // test cases
    std::deque<std::pair<std::string, Errors>> tests {
        // input        expected result
        { "let a b c",  Errors::None },
        { "let",        Errors::LongVar },
        { "a",          Errors::None },
        { "ab",         Errors::LongVar },
        { "3",          Errors::InvalidVar },
        { "let 3",      Errors::LongVar },
        { "let ab c",   Errors::LongVar },
        { "a b",        Errors::TrailingChars }
    };

    // output header
    std::cout << "Passed?  " << std::setw(10) << std::left << "input" << "\tReturn value" << std::setw(0) << std::endl;
    std::cout << std::string(60, '-') << std::endl;

    // execute tests
    for(auto&& i : tests) {
        // transform string into deque<char>
        auto in = std::deque<char>(i.first.begin(), i.first.end());
        // execute parser on input
        auto r = Return(p(in));
        // validate result
        if(r == i.second) {
            // passed...
            std::cout << "Pass:    " << std::setw(10) << std::left << i.first << "\t:: Returned: " << r << std::setw(0) << std::endl;
        } else {
            // failed...
            std::cout << "FAIL:    " << std::setw(10) << std::left << i.first << "\t:: Returned: " << r << std::setw(0) << " Expected: " << i.second << std::endl;
        }
    }
}

/*
BindParser = [](P) -> [P](deque<char>) -> Result || Continue || Fail
    Success(P) -> Continue || Result
    Fail(P) -> Fail
Return = [P]() -> Result
Many = [P](deque<char>) -> Result || Continue
    Success(P) -> [P](deque<char>) -> Result || Continue
    Fail(P) -> Result
And = [A, B](deque<char>) -> Result || Continue || Fail
    Success(A) => [B](deque<char>) -> Result || Fail
    Fail(A) => Fail
Or = [A, B](deque<char>) -> Result || Continue || Fail
    Success(A) => Result
    Fail(A) => [B](deque<char>) -> Result || Fail
*/
