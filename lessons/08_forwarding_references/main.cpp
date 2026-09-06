#include <iostream>
#include <utility>      // std::forward, std::move
#include <type_traits>  // std::is_lvalue_reference_v
#include <vector>

/*

The rule that makes this whole thing work — reference collapsing. 
When a template parameter T gets deduced as itself a reference type, and you then form T&&, 
the two references "collapse" into one, by a fixed rule: & & → &, & && → &, && & → &, and only && && → &&. 
In plain words: anything touching a single & collapses down to &; 
the only way to end up with a real && is if both sides were already &&. 
This is the entire mechanism behind forwarding references — nothing magic, just that one substitution rule.

*/

struct Point { int x; };

// Two "sink" overloads, same trick as the &&-overload-resolution demo --
// lets us see which one actually got called.
void sink(const Point&) { std::cout << "    sink(const Point&) ran  -- treated as an lvalue\n"; }
void sink(Point&&)      { std::cout << "    sink(Point&&) ran       -- treated as an rvalue\n"; }

// A GENUINE forwarding reference: T is deduced RIGHT HERE, from a bare
// T&&, at this exact call. That's the actual requirement -- not the
// spelling "T&&", but T being deduced in this position.
template <typename T>
void relay(T&& p) {
    // Reference collapsing in action: called with an lvalue, T deduces
    // as Point& (a reference type!); called with an rvalue, T deduces
    // as plain Point. Printing T directly shows which happened.
    std::cout << "  relay<T>: T deduced as " << (std::is_lvalue_reference_v<T> ? "Point&  (lvalue arg)" : "Point   (rvalue arg)") << "\n";

    std::cout << "  relay: sink(p), no std::forward:\n";
    sink(p); // p is a NAMED variable -> always an lvalue here, regardless
             // of what T was deduced as (same rule as lesson 03's `other`)

    std::cout << "  relay: sink(std::forward<T>(p)):\n";
    sink(std::forward<T>(p)); // restores the ORIGINAL category: if T is
                              // Point&, stays an lvalue cast; if T is
                              // Point, becomes equivalent to std::move(p).
}

// The classic gotcha: this LOOKS like T&& too, but it is NOT a
// forwarding reference -- T isn't deduced from a bare T&& here, it's
// wrapped inside std::vector<T>. This is an ORDINARY rvalue reference:
// only binds to an rvalue std::vector<T>, exactly like lesson 03's move
// constructor.
template <typename T>
void not_forwarding(std::vector<T>&& v) {
    std::cout << "  not_forwarding: ran, size=" << v.size() << " (only rvalues can ever reach this)\n";
}

int main() {
    Point named{1};

    std::cout << "-- relay(named)  [lvalue] --\n";
    relay(named);

    std::cout << "\n-- relay(Point{2})  [temporary/rvalue] --\n";
    relay(Point{2});

    std::cout << "\n-- relay(std::move(named))  [explicit rvalue cast] --\n";
    relay(std::move(named));

    std::cout << "\n-- not_forwarding: only accepts rvalues --\n";
    not_forwarding(std::vector<int>{1, 2, 3}); // fine: a temporary, an rvalue

    // std::vector<int> v{4,5,6};
    // not_forwarding(v); // uncomment to watch this FAIL to compile --
    // proof that std::vector<T>&& is an ordinary rvalue reference, not a
    // forwarding reference, despite the T&& spelling being present.

    return 0;
}


/*

-- relay(named)  [lvalue] --
  relay<T>: T deduced as Point&  (lvalue arg)
  relay: sink(p), no std::forward:
    sink(const Point&) ran  -- treated as an lvalue
  relay: sink(std::forward<T>(p)):
    sink(const Point&) ran  -- treated as an lvalue

-- relay(Point{2})  [temporary/rvalue] --
  relay<T>: T deduced as Point   (rvalue arg)
  relay: sink(p), no std::forward:
    sink(const Point&) ran  -- treated as an lvalue
  relay: sink(std::forward<T>(p)):
    sink(Point&&) ran       -- treated as an rvalue

-- relay(std::move(named))  [explicit rvalue cast] --
  relay<T>: T deduced as Point   (rvalue arg)
  relay: sink(p), no std::forward:
    sink(const Point&) ran  -- treated as an lvalue
  relay: sink(std::forward<T>(p)):
    sink(Point&&) ran       -- treated as an rvalue

-- not_forwarding: only accepts rvalues --
  not_forwarding: ran, size=3 (only rvalues can ever reach this)

C:\tlm\projects\cppToCUDAExpressionTemplateTraining\out\build\x64-Debug\lessons\08_forwarding_references\08_forwarding_references.exe (process 86536) exited with code 0 (0x0).
Press any key to close this window . . .

*/