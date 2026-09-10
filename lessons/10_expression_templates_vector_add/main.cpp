#include <iostream>
#include <memory>



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

// Vector: the leaf, data-owning type. Built on the exact judgment call
// from the smart-pointers lesson -- unique_ptr<double[]> gives
// destructor AND move for free (neither is written below at all; the
// compiler-generated versions are already correct, because unique_ptr's
// own move/destructor already are). Copy is the one thing unique_ptr
// can't guess for us, so it's the only special member written by hand.
class Vector : public VecExpr<Vector>
{
public:
    explicit Vector(std::size_t n) : size_(n), data_(std::make_unique<double[]>(n)) {} // memsets to zero under the hood (or loops, but anyway), so no need to loop and zero-initialize ourselves


    // WORKS:
    // explicit Vector(std::size_t n) : size_(n), data_(new double[n]) { // works because unique_ptr<double[]> has a constructor that takes a raw pointer, so this line compiles.  unique_ptr<double[]> implements destructor and move for us, so we don't write them here.
    //     for (std::size_t i = 0; i < size_; ++i) data_[i] = 0.0;// inefficient compared to the one we use
    // }

    // FAILS:
    // explicit Vector(std::size_t n) {
    //     size_ = n;
    //     data_ = new double[n];// doesn't work because unique_ptr<double[]> doesn't have a constructor that takes a raw pointer, so this line won't compile.  unique_ptr<double[]> implements destructor and move for us, so we don't write them here.  Copy is the one thing unique_ptr doesn't implement, so we do it by hand.
    //     for (std::size_t i = 0; i < size_; ++i) data_[i] = 0.0;
    // }

    // WORKS:
    // copy constructor: allocate our own buffer, fills it with zeros, then copy elements over.
    // this version double loops when we are going to copy the data anyway.
    // Vector(const Vector& other) : size_(other.size_), data_(std::make_unique<double[]>(other.size_)) {
    //     for (std::size_t i = 0; i < size_; ++i) data_[i] = other.data_[i];
    // }
    
    // copy constructor: allocate our own buffer, then copy elements over.
    Vector(const Vector& other) : size_(other.size_), data_(new double[other.size_]) {
        for (std::size_t i = 0; i < size_; ++i) data_[i] = other.data_[i];
    }

    // FAILS:
    // copy assignment: release what we currently own, then allocate our own buffer and copy elements over.
    // Vector &operator=(const Vector &other) { //: size_(other.size_).data_(mew double[other.size_])
    // {
    //     if (this == &other) return *this;
    //     size_ = other.size;
    //     delete[] data_;// fails because data_ is not a raw pointer, it's a unique_ptr<double[]>, so delete[] data_ won't compile.  unique_ptr<double[]> implements destructor and move for us, so we don't write them here.  Copy is the one thing unique_ptr doesn't implement, so we do it by hand.
    //     data_ = new double[size_];
    //     for (std::size_t i = 0 ; i < size_; ++i) data_[0] = other.data_[i];
    //     data_.reset(new double[size_]); // raw new[], not make_unique, because we want to copy the elements over ourselves, not default-construct them
    // }
    
    // copy assignment: release what we currently own, then allocate our own buffer and copy elements over.
    /* 
    this fails, here is why:
    
    delete[] data_; data_ = new double[size_]; 
    still won't compile against unique_ptr<double[]> data_ 
    — same reason as before: delete[] needs an actual pointer (not a unique_ptr object), 
    and unique_ptr refuses direct assignment from a raw pointer. 
    .reset(new double[size_]) is the only one of the two that actually compiles here.
    */
    // Vector &operator=(const Vector &other) {
    //     if (this == &other) return *this;
    //     size_ = other.size_;
    //     delete[] data_;// fails:  data_ is not a raw pointer, it's a unique_ptr<double[]>, so delete[] data_ won't compile.  unique_ptr<double[]> implements destructor and move for us, so we don't write them here.  Copy is the one thing unique_ptr doesn't implement, so we do it by hand.
    //     data_ = new double[size_];// can't do this either, because unique_ptr<double[]> doesn't have a constructor that takes a raw pointer, so this line won't compile.  unique_ptr<double[]> implements destructor and move for us, so we don't write them here.  
    //     for (std::size_t i = 0 ; i < size_; ++i) data_[i] = other.data_[i];
    //     return *this;
    // }


    // copy assignment: release what we currently own, then allocate our own buffer and copy elements over.
    Vector& operator=(const Vector& other) {
        if (this == &other) return *this;
        size_ = other.size_;
        data_.reset(new double[size_]);// raw new[], not make_unique, because we want to copy the elements over ourselves, not default-construct them
        for (std::size_t i = 0; i < size_; ++i) data_[i] = other.data_[i];
        return *this;
    }

    // destructor, move assignment, move constructor: 
    //      all implemented by unique_ptr<double[]> for us, so we don't write them here.

    double& operator[](std::size_t i) { return data_[i]; } 
    const double& operator[](std::size_t i) const { return data_[i]; }
    std::size_t size() const { return size_; }

private:
    std::size_t size_;
    std::unique_ptr<double[]> data_;// unique_ptr<double[]> implements destructor and move for us, 
    // so we don't write them here. 
    // Copy is the one thing unique_ptr doesn't implement, so we do it by hand.
};


// THE central decision of this lesson -- and the exact idea your old
// ExpressionTemplates repo was reaching for (its A_Traits/ExprRef
// pattern), just with the dangling-reference bug still live in it there.
//
// Default: store BY VALUE. Correct for expression NODES (AddExpr,
// etc.) -- cheap to copy (just a couple of members) and frequently
// temporaries themselves, e.g. the inner (a+b) in (a+b)+c. Storing them
// by value is what makes that safe: no reference to a temporary that's
// about to die.
template <typename T>
struct ExprTraits { using ExprRef = T; };
// defines ExprRef as just another name for T


// Specialized for Vector: store BY CONST REFERENCE. Vector is expensive
// to copy (a real heap buffer), and a named Vector passed into an
// expression outlives the expression using it in ordinary code -- so a
// reference is both safe and avoids an expensive copy.
template <>
struct ExprTraits<Vector> { using ExprRef = const Vector&; };
// redefines ExprRef as "const Vectort&" for Vectors

// AddExpr stores whatever ExprTraits<LHS>::ExprRef / ExprTraits<RHS>::ExprRef
// resolve to -- a reference for a Vector operand, a value for a nested
// expression-node operand. That's the whole fix, made structural instead
// of hoped-for.
template <typename LHS, typename RHS>
class AddExpr : public VecExpr<AddExpr<LHS, RHS>> {
public:
    AddExpr(const LHS& l, const RHS& r) : lhs_(l), rhs_(r) {}

    double operator[](std::size_t i) const { return lhs_[i] + rhs_[i]; }// iteration though all i is done by the evaluate function
    std::size_t size() const { return lhs_.size(); }

private:
    typename ExprTraits<LHS>::ExprRef lhs_;
    typename ExprTraits<RHS>::ExprRef rhs_;
};

//
// note!: AddExpr is not a member of any of the classes AddExpr, Vector, or struct VecExpr 
//
template <typename LHS, typename RHS>
AddExpr<LHS, RHS> operator+(const VecExpr<LHS>& lhs, const VecExpr<RHS>& rhs) {
    return AddExpr<LHS, RHS>(lhs.self(), rhs.self());
    // about self(): the lhs and rhs are VecExpr<LHS> and VecExpr<RHS> respectively, 
    // so self() returns a const LHS& and const RHS& respectively.  
    // The AddExpr constructor takes const LHS& and const RHS& respectively, so this works out.
    // CRTP:
    // const Derived& self() const { return static_cast<const Derived&>(*this); } 
}


template <typename LHS, typename RHS>
class SubExpr : public VecExpr<SubExpr<LHS, RHS>> {
public:
    SubExpr(const LHS& l, const RHS& r) : lhs_(l), rhs_(r) {}

    double operator[](std::size_t i) const { return lhs_[i] - rhs_[i]; }// iteration though all i is done by the evaluate function
    std::size_t size() const { return lhs_.size(); }

private:
    typename ExprTraits<LHS>::ExprRef lhs_;
    typename ExprTraits<RHS>::ExprRef rhs_;
};

template <typename LHS, typename RHS>
SubExpr<LHS, RHS> operator-(const VecExpr<LHS>& lhs, const VecExpr<RHS>& rhs) {
    return SubExpr<LHS, RHS>(lhs.self(), rhs.self());
    // about self(): the lhs and rhs are VecExpr<LHS> and VecExpr<RHS> respectively, 
    // so self() returns a const LHS& and const RHS& respectively.  
    // The AddExpr constructor takes const LHS& and const RHS& respectively, so this works out.
    // CRTP:
    // const Derived& self() const { return static_cast<const Derived&>(*this); } 
}





// The "DependentVar"-equivalent moment: forces a lazy expression tree
// into a real, concrete Vector.
template <typename Derived>
Vector evaluate(const VecExpr<Derived>& expr) {
    Vector result(expr.size());// allocate a real Vector called "result" to hold the result
    // double operator[](std::size_t i) const { return lhs_[i] + rhs_[i]; }
    for (std::size_t i = 0; i < expr.size(); ++i) result[i] = expr[i]; // very lazy evaluation: 
	// note the type of expr is VecExpr<Derived>, 
    // so expr[i] calls operator[](std::size_t i) const 
    // on the VecExpr<Derived> object, which calls self() to get the Derived object, 
    // and then calls operator[](std::size_t i) const on the Derived object, 
    // which is either a Vector or an AddExpr<LHS,RHS>.
    //
    // just loop over the expression and fill in the result Vector.  
    // This is where the actual computation happens, and where the expression tree is traversed.
    return result;
}



int main() {
    Vector a(3), b(3), c(3);
    a[0] = 1; a[1] = 2; a[2] = 3;
    b[0] = 10; b[1] = 20; b[2] = 30;
    c[0] = 100; c[1] = 200; c[2] = 300;

    // a + b + c parses as (a + b) + c. The inner (a+b) is a TEMPORARY
    // AddExpr<Vector,Vector>. It gets copied (cheaply -- ExprTraits
    // says "by value" for expression nodes) into the outer
    // AddExpr<AddExpr<Vector,Vector>, Vector> BEFORE that temporary is
    // destroyed. That's the fix from lesson 1, finally applied where it
    // actually matters.
    Vector result = evaluate(a + b + c);

    std::cout << "a + b + c = [";
    for (std::size_t i = 0; i < result.size(); ++i) {
        std::cout << result[i];
        if (i + 1 < result.size()) std::cout << ", ";
    }
    std::cout << "]\n(expected [111, 222, 333])\n";



    Vector result2 = evaluate(c - b - a);

    std::cout << "c - b - a = [";
    for (std::size_t i = 0; i < result2.size(); ++i) {
        std::cout << result2[i];
        if (i + 1 < result2.size()) std::cout << ", ";
    }
    std::cout << "]\n(expected [89, 178, 267])\n";

    return 0;
}




/*

this is the key:

for (std::size_t i = 0; i < expr.size(); ++i) result[i] = expr[i];

* note the type of expr is VecExpr<Derived>,
* so expr[i] calls operator[](std::size_t i) const on the VecExpr<Derived> object,
* which calls self() to get the Derived object,
* and then calls operator[](std::size_t i) const on the Derived object, 
* which is either a Vector or an AddExpr<LHS,RHS>.
* 
* how does it resolve to AddExpr<LHS,RHS>::operator[](std::size_t i) const?  
  * the Derived type is AddExpr<LHS,RHS>, so self() returns a const AddExpr<LHS,RHS>&, 
  * and then operator[](std::size_t i) const 
  * on that object calls AddExpr<LHS,RHS>::operator[](std::size_t i) const, 
  * which returns lhs_[i] + rhs_[i], which are either Vectors or other AddExprs, 
  * and so on recursively until we hit the leaf Vectors, which return their actual data.
  * 
  * 
* why is the Derived type is AddExpr<LHS,RHS>?  
    * Answer: because we have an overload of operator+ that returns an AddExpr<LHS,RHS> 
	*   we have no other operator+ overloads for VecExprs, so the compiler chooses that one
	*   but how did it know it was a VecExpr<LHS> and VecExpr<RHS>?
	*       - I thought it figured it out purely because there were no other operator+ overloads
* but how does it match the right operands?  The operands look like vectors to me.
    Answer: 
 
	*       - because the operator+ is a template that takes two VecExprs,
    * when we add two VecExprs together.
        * because the expression a + b + c parses as (a + b) + c, 
        * so the inner (a+b) is an AddExpr<Vector,Vector>, 
        * and the outer (a+b)+c is an AddExpr<AddExpr<Vector,Vector>, Vector>, 
        * so the Derived type is AddExpr<AddExpr<Vector,Vector>, Vector> for the outer expression.
*/