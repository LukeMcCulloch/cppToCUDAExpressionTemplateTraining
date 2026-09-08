#include <iostream>

// ---------------------------------------------------------------
// First, the familiar way: runtime polymorphism via virtual functions.
// A Shape* can point at any derived shape; area() is resolved at
// RUNTIME via a vtable lookup -- the compiler cannot know at compile
// time which override will actually run.
// ---------------------------------------------------------------
struct VirtualShape {
    virtual double area() const = 0;
    virtual ~VirtualShape() = default;
};
struct VirtualCircle : VirtualShape {
    double r;
    explicit VirtualCircle(double radius) : r(radius) {}
    double area() const override { return 3.14159 * r * r; }
};

// ---------------------------------------------------------------
// CRTP: "Curiously Recurring Template Pattern". Derived inherits from
// Base<Derived> -- the base class template is instantiated WITH the
// derived type itself, so at compile time the base already knows
// exactly what concrete type it's dealing with. No virtual functions,
// no vtable, no runtime dispatch -- everything resolves and inlines at
// compile time. This is EXACTLY the Expr<Derived> pattern from your
// AD project's TinyADExpr.hpp.
// ---------------------------------------------------------------
template <typename Derived>
struct Shape {
    // self(): does the cast ONCE, so derived-facing code doesn't repeat
    // static_cast<const Derived*>(this) everywhere. Directly mirrors
    // Expr<Derived>::self() from your AD code.
    const Derived& self() const { return *static_cast<const Derived*>(this); }

    // NOT virtual. Calls area_impl() on the derived type, resolved at
    // COMPILE time via self() -- the compiler knows exactly which
    // area_impl() this is at the call site, and can inline it.
    double area() const { return self().area_impl(); }
};

struct Circle : Shape<Circle> { 
    // Shape<Circle> tells the compiler 
    // that the base class is a Shape instantiated with the derived type Circle. 
    // This is the "curiously recurring" part of CRTP.
    double r;
    explicit Circle(double radius) : r(radius) {}
    double area_impl() const { return 3.14159 * r * r; }

    // note
    /*
    Derived becomes Circle, not Shape<Circle>. 
    Shape<Circle> is the base class itself — 
    the result of plugging Circle in for Derived inside Shape's own definition. 
    Circle publicly inherits from that base class (struct Circle : Shape<Circle>), 
    and inside that base class, the template parameter Derived is just Circle
    */
};

struct Square : Shape<Square> {
    double side;
    explicit Square(double s) : side(s) {}
    double area_impl() const { return side * side; }
};

// Works with ANY CRTP shape generically -- this is the "polymorphism"
// part: same calling code, different concrete behavior, decided at
// COMPILE time instead of runtime.
template <typename ConcreteShape>
void print_area(const Shape<ConcreteShape>& s) {
    std::cout << "area = " << s.area() << "\n";
}

int main() {
    std::cout << "-- virtual dispatch (runtime polymorphism) --\n";
    VirtualCircle vc(2.0);
    VirtualShape* vs = &vc;
    std::cout << "vs->area() = " << vs->area() << "\n";
    std::cout << "sizeof(VirtualCircle) = " << sizeof(VirtualCircle) // sizeof(VirtualCircle) = 16 bytes (includes a hidden vtable pointer)
              << "  (includes a hidden vtable pointer)\n";

    std::cout << "\n-- CRTP (compile-time / static polymorphism) --\n";
    Circle c(2.0);
    Square sq(3.0);
    print_area(c);
    print_area(sq);
    std::cout << "sizeof(Circle) = " << sizeof(Circle) // sizeof(Circle) = 8 bytes (just the double -- no vtable pointer at all)
              << "  (just the double -- no vtable pointer at all)\n";

    // That size difference between vc (16 bytes) and c (8 bytes) 
    // is the concrete, measurable version of "no runtime overhead" that's usually just stated as a claim.
    return 0;
}