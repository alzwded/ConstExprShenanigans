#include <cctype>
#include <cstdio>
#include <deque>
#include <functional>
#include <stdexcept>
#include <string>

// odd-even list processing lib
template<typename T>
void OddEvenList(
        std::deque<T> a,
        std::function<void(T)> fodd,
        std::function<void(T)> feven)
{
    if(a.empty()) return;
    fodd(a.front());
    a.pop_front();
    struct q {
        static void Continue(std::deque<T> a, decltype(fodd) ffodd, decltype(feven) ffeven)
        {
            if(a.empty()) return;
            ffeven(a.front());
            a.pop_front();
            if(a.empty()) throw std::out_of_range("number of elements");
            ffodd(a.front());
            a.pop_front();
            Continue(a, ffodd, ffeven);
        }
    };
    q::Continue(a, fodd, feven);
}

// our user defined odd and even functions
void odd(char c) { printf("%c", c); }
void even(char c) { printf(" "); }
void odd2(char c) { printf("%c", std::toupper(c)); }

int main()
{
    using namespace std::placeholders;
    // our data set
    auto l1 = std::deque<char> { 'a', ',', 'b', ',', 'c' };
    auto l2 = std::deque<char> { 'q', ',', 'w', ',', 'e' };
    // construct the function we need using higher order function
    // OddEvenList by binding the actions for odd, respectively even
    auto f = std::bind(OddEvenList<char>, _1, odd, even);
    // call our constructed function
    f(l1); printf("\n");
    f(l2); printf("\n");
    // ...or we can do something else for odd
    auto g = std::bind(OddEvenList<char>, _1, odd2, even);
    g(l1); printf("\n");
    g(l2); printf("\n");
}
