#include <iostream>
#include <utility>      // std::forward, std::move
#include <type_traits>  // std::is_lvalue_reference_v
#include <vector>
// FORWARDING REFERENCES
/*
What "forwarding" actually means, conceptually — worth having this in plain words, 
separate from the mechanics (reference collapsing, T&&) we already covered:

Imagine a middleman function — something generic that takes an argument 
and needs to hand it along to some other function that does the real work, 
without being the final destination itself. 
The problem this creates: once an argument has a name inside your middleman function, 
it's an lvalue (you learned this rule already, back 
with IntBuffer's move constructor's other). 
So if your middleman just naively passes that named argument on, 
the receiving function always sees an lvalue 
— even if the original caller passed you something disposable, like a temporary, 
meant to be moved cheaply rather than copied expensively. 
You've accidentally destroyed information on the way through: 
    "was this safe to steal from?" got flattened to "no" 
    by the mere act of giving it a name in your function.

"Forwarding" is preserving that information across the handoff 
— relaying an argument to another function while keeping intact 
whether the original caller gave you something disposable (rvalue) 
or something they still need (lvalue), 
so the function at the far end makes the same copy-vs-move decision 
it would have made if the original caller had called it directly. 
std::forward<T>(x) is the tool that restores that original category 
right before the handoff, instead of letting it silently collapse to "always lvalue" 
just because x has a name.

This is exactly the situation your own AddExpr/MultExpr 
constructors will be in once you rebuild the expression-template AD code properly: 
    AddExpr(LHS l, RHS r) takes operands and needs to store them 
    — built from two temporaries, 
    it should be free to move them in cheaply; 
    built from two named Vars, it needs to copy. 
    A perfectly-forwarding version accepts LHS&&, RHS&& and uses std::forward 
    when initializing its members, 
    so it gets the right behavior automatically in both cases 
    instead of picking one policy for everyone.

What to actually do with the lesson 08 file, now that it builds:

Run it as-is. 
Check the three relay(...) calls' T deduced as ... lines match what you'd expect: 
    the named-lvalue call should show Point&, 
    the temporary and the std::move() call should both show plain Point.
Look closely at the pair of sink(...) calls inside relay for each of the three outer calls: 
sink(p) (no forward) should hit the const Point& overload every single time, 
regardless of which relay(...) call you're in 
— while sink(std::forward<T>(p)) correctly varies, matching the original caller's argument. 
That contrast, side by side, is the whole lesson made visible.
Uncomment the two not_forwarding lines at the bottom, rebuild, 
and read the actual compiler error. 
That failure is the proof that a T&& buried inside std::vector<T> 
is not a forwarding reference, even though it's spelled the same way as one.
*/
/*

The rule that makes this whole thing work — reference collapsing. 
When a template parameter T gets deduced as itself a reference type, and you then form T&&, 
the two references "collapse" into one, 
by a fixed rule: & & → &, & && → &, && & → &, and only && && → &&. 
// ---
In plain words: anything touching a single & collapses down to &; 
the only way to end up with a real && is if both sides were already &&. 
This is the entire mechanism behind forwarding references — nothing magic, just that one substitution rule.

*/


// Lvalue and Rvalue point of view:
/*
(a) sink(const Point&) runs when you hand it an lvalue; sink(Point&&) runs when you hand it an rvalue.
(b) Any parameter with a name is an lvalue, once you're inside the function body 
— you learned this exact fact from other in the move constructor. 
Doesn't matter what type it was declared as; having a name makes it an lvalue.
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
	Point named{ 1 };// named is an lvalue

    std::cout << "-- relay(named)  [lvalue] --\n";
	relay(named);// named is an lvalue

    std::cout << "\n-- relay(Point{2})  [temporary/rvalue] --\n";
	relay(Point{ 2 });// temporary, an rvalue

    std::cout << "\n-- relay(std::move(named))  [explicit rvalue cast] --\n";
	relay(std::move(named));// std::move(named) is an unconditional cast to an rvalue, so T deduces as Point, not Point&.

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


/*
A plain rvalue reference to a concrete type binds only to rvalues — no exceptions, no collapsing, nothing that could let an lvalue sneak through directly. Trying to pass a plain named IntBuffer to it is simply a compile error.

One precise caveat, since it's worth being exact: d = std::move(c); 
does successfully reach that && parameter even though c is a named lvalue 
— but that's not an lvalue "getting through" despite the rule. 
std::move(c) produces a new expression that is itself rvalue-typed 
(a cast, static_cast<IntBuffer&&>(c)), 
and it's that rvalue-typed expression that binds to the parameter 
— the caller explicitly manufactured an rvalue on purpose. 
c the variable never stopped being an lvalue; 
you just handed the function a cast result instead of c directly. 
So the rule holds without exception: 
    whatever actually binds to a plain, non-template T&& is an rvalue, full stop 
— it's just that std::move is how you get to supply one from something that started out named.
*/