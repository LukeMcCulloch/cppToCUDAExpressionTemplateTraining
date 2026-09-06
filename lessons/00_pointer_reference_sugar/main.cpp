#include <iostream>

struct Point {
    int x;
};

// A member function, so we have a real `this` to demonstrate with --
// mirrors exactly what's going on inside IntBuffer's methods.
struct Labeled {
    int value;
    void show() {
        std::cout << "  this      (pointer)      -> address: " << this << "\n";
        std::cout << "  *this     (dereferenced)  -> value:   " << (*this).value << "\n";
        std::cout << "  this->value (sugar for (*this).value): " << this->value << "\n";
    }
};


// ---------------------------------------------------------------
// && -- rvalue reference. A DIFFERENT reference kind from &, used
// purely to drive overload resolution: it binds ONLY to rvalues
// (temporaries, or things explicitly cast via std::move) -- never to
// a plain named variable. Two overloads, differing only in & vs &&,
// let the compiler pick automatically based on what's passed in.
// ---------------------------------------------------------------
void describe(const Point& p) {
    std::cout << "&& overload resolution: LVALUE overload ran  (const Point&), p.x=" << p.x << "\n";
}
void describe(Point&& p) {
    std::cout << "&& overload resolution: RVALUE overload ran  (Point&&),      p.x=" << p.x << "\n";

    // Gotcha: `p` has a NAME here, so inside this body p is itself an
    // lvalue -- even though its declared TYPE is Point&&. This call
    // goes to the LVALUE overload above, not a recursive call to this
    // one, despite being inside the && overload right now:
    std::cout << "  (from inside the && overload) describe(p) with no std::move: ";
    describe(p); // named rvalue references are themselves lvalues.  
}



int main() {

    // ---------------------------------------------------------------
    // & -- JOB 1: reference, in a TYPE position (declaring a parameter
    // or variable). No & or * ever needed at the use site.
    // ---------------------------------------------------------------
    auto print_via_reference = [](const Point& p) { // & here = "p is a reference"
        std::cout << "& job 1 (reference param): p.x = " << p.x << "\n";
    };
    Point pt{42};
    print_via_reference(pt); // called completely normally -- no & here at all.  prints p.x = 42

    // ---------------------------------------------------------------
    // & -- JOB 2: address-of OPERATOR, in an EXPRESSION, applied to an
    // existing object. Produces a pointer (a plain address value).
    // ---------------------------------------------------------------
    Point* addr = &pt; // & here = "give me pt's address" -- an operator, not a type
    std::cout << "& job 2 (address-of):    &pt   = " << addr << "\n"; // prints the address of pt, e.g. 0x7ffee3b8c9a0

    // ---------------------------------------------------------------
    // * -- JOB 1: pointer declarator, in a TYPE position (seen just
    // above: `Point*` declares addr as "pointer to Point").
    //
    // * -- JOB 2: dereference OPERATOR, in an EXPRESSION, applied to a
    // pointer VALUE. Gives you back the actual object at that address.
    // ---------------------------------------------------------------
    std::cout << "* job 2 (dereference):   *addr = " << (*addr).x << "\n"; // prints 42  aka (*addr).x = pt.x = 42

    // ---------------------------------------------------------------
    // * -- JOB 3 (bonus): plain old multiplication. Totally unrelated
    // to pointers -- same character, a third grammatical role.
    // ---------------------------------------------------------------
    int product = 6 * 7;
    std::cout << "* job 3 (multiplication): 6 * 7 = " << product << "\n"; // prints 42 for fun, not pointer-related at all

    // ---------------------------------------------------------------
    // -> -- JOB 1: pointer member access. Defined to mean EXACTLY
    // (*addr).x -- pure syntactic sugar, not a separate mechanism.
    // ---------------------------------------------------------------
    std::cout << "-> job 1 (member access): addr->x = " << addr->x << "\n"; // prints 42, same as (*addr).x above.  addr->x is just a shorthand for (*addr).x

    // ---------------------------------------------------------------
    // -> -- JOB 2: trailing return type, in a function/lambda
    // DECLARATION, right after the parameter list. Nothing to do with
    // pointers or dereferencing at all.
    // ---------------------------------------------------------------
    auto make_point = [](int val) -> Point { return Point{val}; }; // -> here = return type
    Point made = make_point(99);
    std::cout << "-> job 2 (trailing return type): make_point(99).x = " << made.x << "\n";// prints 99, nothing to do with pointers or dereferencing at all, except that the return type of the lambda is declared with a ->.  This is just a different way to write the return type, instead of writing `Point make_point(int val) { ... }` which is the more common way to declare a function's return type.

    // ---------------------------------------------------------------
    // this / *this / this->member, all three side by side, via a real
    // member function -- same rules as IntBuffer, just on a tiny type.
    // ---------------------------------------------------------------
    std::cout << "\nthis / *this / this->member:\n";
    Labeled lbl{7};
    lbl.show();

    

    // && : lvalue vs rvalue, decided by what's passed at the call site
    std::cout << "\n&& (rvalue reference), three call sites:\n";
    Point named_pt{5};
    describe(named_pt);            // named_pt is an lvalue          -> lvalue overload
    describe(Point{6});            // a temporary is an rvalue       -> rvalue overload
    describe(std::move(named_pt)); // explicitly cast to an rvalue   -> rvalue overload

    return 0;
}