#pragma once



// CRTP base -- same self() idiom as lesson 07, and the same shape as
// Expr<Derived> from your very first AD project's TinyADExpr.hpp.
template <typename Derived>
struct VecExpr
{
    // CRTP: cast this base to the derived type, so we can call derived methods from the base class
    const Derived& self() const { return static_cast<const Derived&>(*this); } // get through to the real type.  -> cast this base to the derived type, so we can call derived methods from the base class

    double operator[](std::size_t i) const { return self()[i]; } // look up operator[] on the real type, not on me, via the CRTP implemented in self() above. -> look up operator[] on the real type, not on me, via the CRTP implemented in self() above.
    std::size_t size() const { return self().size(); } // look up size() on the real type, not on me, via the CRTP implemented in self() above. -> look up size() on the real type, not on me, via the CRTP implemented in self() above.
};


