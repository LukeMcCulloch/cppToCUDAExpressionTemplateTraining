#pragma once

#include "VecExpr.hpp"
#include "ExprTraits.hpp"



//--------------
// ADD
//--------------
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





//--------------
// SUBTRACT
//--------------

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




//--------------
// MULTIPLY Mat * Vec
//--------------

template <typename RHS>
class MatVecMultExpr : public VecExpr<MatVecMultExpr<RHS>> {
// this class is structuraly tied to producing a 1D VecExpr shaped result 
//      (it inherits from VecExpr<MultExpr<RHS>>)
public:
    MatVecMultExpr(const Matrix& m, const RHS& v) : m_(m), v_(v) {}

    double operator[](std::size_t i) const {
        // dot product of one row of the matrix with the vector
        double sum = 0.0;
        for (std::size_t k = 0; k < m_.cols(); ++k) 
        {
            sum += m_(i, k) * v_[k];
        }
        return sum;
    }

    std::size_t size() const { return m_.rows(); }// size of the output

private:
    typename ExprTraits<Matrix>::ExprRef m_;
    typename ExprTraits<RHS>::ExprRef v_;

};


// operator free function
template <typename RHS>
MatVecMultExpr<RHS> operator*(const Matrix& m, const VecExpr<RHS>& v) {
    return MatVecMultExpr<RHS>(m, v.self());
}



//--------------
// MULTIPLY Vec^T * Mat
//--------------

template <typename LHS>
class VecMatMultExpr : public VecExpr<VecMatMultExpr<LHS>> {
// this class is structuraly tied to producing a 1D VecExpr shaped result 
//      (it inherits from VecExpr<MultExpr<RHS>>)
public:
    VecMatMultExpr(const LHS& v, const Matrix& m) : v_(v), m_(m) {}

    double operator[](std::size_t i) const {
        // dot product of vector^T with one column of the matrix
        double sum = 0.0;
        for (std::size_t k = 0; k < m_.rows(); ++k) 
        {
            sum += v_[k] * m_(k, i) ;
        }
        return sum;
    }

    std::size_t size() const { return m_.cols(); } // size of the output

private:
    typename ExprTraits<LHS>::ExprRef v_;
    typename ExprTraits<Matrix>::ExprRef m_;

};


// operator free function
template <typename LHS>
VecMatMultExpr<LHS> operator*(const VecExpr<LHS>& v, const Matrix& m) {
    return VecMatMultExpr<LHS>(v.self(), m);
}



