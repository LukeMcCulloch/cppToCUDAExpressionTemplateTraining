#pragma once



 ////CRTP base -- same self() idiom as lesson 07, and the same shape as
 ////     Expr<Derived> from your very first AD project's TinyADExpr.hpp.
 //template <typename Derived>
 //struct MatExpr
 //{
 //    // CRTP: cast this base to the derived type, so we can call derived methods from the base class
 //    const Derived& self() const { return static_cast<const Derived&>(*this); } // get through to the real type.  -> cast this base to the derived type, so we can call derived methods from the base class

 //    double operator()(std::size_t i, std::size_t j) const { return self()(i, j); } // look up operator() on the real type, not on me, via the CRTP implemented in self() above. 
 //    //std::size_t size() const { return self().size(); } // look up size() on the real type, not on me, via the CRTP implemented in self() above.
 //    std::size_t rows() const { return self().rows(); }
 //    std::size_t cols() const { return self().cols(); }
 //};


template <typename RHS>
class MultExpr : public VecExpr<MultExpr<RHS>> {
public:
    MultExpr(const Matrix& m, const RHS& v) : m_(m), v_(v) {}

    double operator[](std::size_t i) const {
        double sum = 0.0;
        for (std::size_t k = 0; k < m_.cols(); ++k) {
            sum += m_(i, k) * v_[k];
        }
        return sum;
    }
    std::size_t size() const { return m_.rows(); }

private:
    typename ExprTraits<Matrix>::ExprRef m_;
    typename ExprTraits<RHS>::ExprRef v_;
};

template <typename RHS>
MultExpr<RHS> operator*(const Matrix& m, const VecExpr<RHS>& v) {
    return MultExpr<RHS>(m, v.self());
}